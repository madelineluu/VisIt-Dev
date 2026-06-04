// ************************************************************************* //
//                      avt_MADELINE_LUUFileFormat.C                         //
// ************************************************************************* //

#include <avt_MADELINE_LUUFileFormat.h>

#include <avtDatabaseMetaData.h>
#include <avtMeshMetaData.h>
#include <avtScalarMetaData.h>
#include <DBOptionsAttributes.h>
#include <InvalidVariableException.h>
#include <InvalidFilesException.h>

#include <vtkFloatArray.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cstring>

// -------------------------------------------------------------------------
// VTK cell-type constants (subset used here)
// -------------------------------------------------------------------------
#ifndef VTK_TRIANGLE
#define VTK_TRIANGLE   5
#define VTK_QUAD       9
#define VTK_TETRA      10
#define VTK_HEXAHEDRON 12
#endif

// ****************************************************************************
//  Method: avt_MADELINE_LUUFileFormat constructor
//
//  Programmer: Madeline Luu
//  Creation:   May 2026
// ****************************************************************************

avt_MADELINE_LUUFileFormat::avt_MADELINE_LUUFileFormat(const char *filename)
    : avtSTSDFileFormat(filename), filename_(filename), fileRead_(false)
{
}

// ****************************************************************************
//  Method: FreeUpResources
//
//  Purpose: Release cached data when VisIt is done with this timestep.
//
//  Programmer: Madeline Luu
//  Creation:   May 2026
// ****************************************************************************

void
avt_MADELINE_LUUFileFormat::FreeUpResources()
{
    points_.clear();
    cellTypes_.clear();
    cellConn_.clear();
    scalarData_.clear();
    fileRead_ = false;
}

// ****************************************************************************
//  Method: ReadFile  (private helper)
//
//  Purpose:
//      Parse a .madeline text file.
//
//      Grammar (whitespace / blank lines / #-comments ignored):
//
//        POINTS <N>
//          x y z          (N rows)
//
//        CELLS <M>
//          cellType p0 p1 ...   (M rows)
//
//        DATA
//          scalar          (one per point, N rows)
//
//  Programmer: Madeline Luu
//  Creation:   May 2026
// ****************************************************************************

void
avt_MADELINE_LUUFileFormat::ReadFile()
{
    if (fileRead_)
        return;

    std::ifstream in(filename_.c_str());
    if (!in.is_open())
    {
        EXCEPTION1(InvalidFilesException, filename_.c_str());
    }

    enum Section { NONE, READING_POINTS, READING_CELLS, READING_DATA };
    Section section = NONE;

    int nPoints = 0, nCells = 0;
    int pointsRead = 0, cellsRead = 0, dataRead = 0;

    std::string line;
    while (std::getline(in, line))
    {
        // Strip comments
        std::size_t hashPos = line.find('#');
        if (hashPos != std::string::npos)
            line = line.substr(0, hashPos);

        // Trim leading/trailing whitespace
        std::size_t start = line.find_first_not_of(" \t\r\n");
        if (start == std::string::npos)
            continue;   // blank line
        line = line.substr(start);

        std::istringstream iss(line);
        std::string token;
        iss >> token;

        // ----- Section headers -----
        if (token == "POINTS")
        {
            iss >> nPoints;
            section = READING_POINTS;
            points_.reserve(nPoints * 3);
            continue;
        }
        if (token == "CELLS")
        {
            iss >> nCells;
            section = READING_CELLS;
            cellConn_.reserve(nCells);
            cellTypes_.reserve(nCells);
            continue;
        }
        if (token == "DATA")
        {
            section = READING_DATA;
            scalarData_.reserve(nPoints);
            continue;
        }

        // ----- Data rows -----
        if (section == READING_POINTS && pointsRead < nPoints)
        {
            float x, y, z;
            std::istringstream row(line);
            if (!(row >> x >> y >> z))
                continue;
            points_.push_back(x);
            points_.push_back(y);
            points_.push_back(z);
            ++pointsRead;
        }
        else if (section == READING_CELLS && cellsRead < nCells)
        {
            std::istringstream row(line);
            int ct;
            if (!(row >> ct))
                continue;
            cellTypes_.push_back(ct);

            std::vector<int> conn;
            int idx;
            while (row >> idx)
                conn.push_back(idx);
            cellConn_.push_back(conn);
            ++cellsRead;
        }
        else if (section == READING_DATA && dataRead < nPoints)
        {
            std::istringstream row(line);
            float val;
            if (!(row >> val))
                continue;
            scalarData_.push_back(val);
            ++dataRead;
        }
    }

    in.close();
    fileRead_ = true;
}

// ****************************************************************************
//  Method: PopulateDatabaseMetaData
//
//  Purpose:
//      Tell VisIt what meshes and variables this file exposes.
//
//  Programmer: Madeline Luu
//  Creation:   May 2026
// ****************************************************************************

void
avt_MADELINE_LUUFileFormat::PopulateDatabaseMetaData(
    avtDatabaseMetaData *md)
{
    ReadFile();

    // ----- Mesh metadata -----
    avtMeshMetaData *mmd = new avtMeshMetaData;
    mmd->name            = "mesh";
    mmd->meshType        = AVT_UNSTRUCTURED_MESH;
    mmd->topologicalDimension = 2;   // triangles → 2D surface cells
    mmd->spatialDimension     = 3;
    mmd->numBlocks       = 1;
    mmd->blockOrigin     = 0;
    md->Add(mmd);

    // ----- Scalar variable metadata -----
    if (!scalarData_.empty())
    {
        avtScalarMetaData *smd = new avtScalarMetaData;
        smd->name        = "scalar";
        smd->meshName    = "mesh";
        smd->centering   = AVT_NODECENT;   // one value per point
        md->Add(smd);
    }
}

// ****************************************************************************
//  Method: GetMesh
//
//  Purpose:
//      Build and return a vtkUnstructuredGrid from parsed data.
//
//  Programmer: Madeline Luu
//  Creation:   May 2026
// ****************************************************************************

vtkDataSet *
avt_MADELINE_LUUFileFormat::GetMesh(const char *meshname)
{
    ReadFile();

    if (strcmp(meshname, "mesh") != 0)
    {
        EXCEPTION1(InvalidVariableException, meshname);
    }

    // ---- Points ----
    vtkPoints *pts = vtkPoints::New();
    int nPts = (int)(points_.size() / 3);
    pts->SetNumberOfPoints(nPts);
    for (int i = 0; i < nPts; ++i)
    {
        pts->SetPoint(i,
                      points_[i * 3 + 0],
                      points_[i * 3 + 1],
                      points_[i * 3 + 2]);
    }

    // ---- Cells ----
    vtkUnstructuredGrid *ug = vtkUnstructuredGrid::New();
    ug->SetPoints(pts);
    pts->Delete();

    int nCells = (int)cellTypes_.size();
    ug->Allocate(nCells);

    for (int c = 0; c < nCells; ++c)
    {
        const std::vector<int> &conn = cellConn_[c];
        // Build a vtkIdType array for the connectivity
        std::vector<vtkIdType> ids(conn.begin(), conn.end());
        ug->InsertNextCell(cellTypes_[c],
                           (vtkIdType)ids.size(),
                           &ids[0]);
    }

    return ug;
}

// ****************************************************************************
//  Method: GetVar
//
//  Purpose:
//      Return the per-point scalar array.
//
//  Programmer: Madeline Luu
//  Creation:   May 2026
// ****************************************************************************

vtkDataArray *
avt_MADELINE_LUUFileFormat::GetVar(const char *varname)
{
    ReadFile();

    if (strcmp(varname, "scalar") != 0)
    {
        EXCEPTION1(InvalidVariableException, varname);
    }

    vtkFloatArray *arr = vtkFloatArray::New();
    arr->SetName("scalar");
    int n = (int)scalarData_.size();
    arr->SetNumberOfTuples(n);
    for (int i = 0; i < n; ++i)
        arr->SetValue(i, scalarData_[i]);

    return arr;
}

// ****************************************************************************
//  Method: GetVectorVar  (not used, but must be implemented)
// ****************************************************************************

vtkDataArray *
avt_MADELINE_LUUFileFormat::GetVectorVar(const char *varname)
{
    EXCEPTION1(InvalidVariableException, varname);
}
