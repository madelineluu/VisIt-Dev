// ************************************************************************* //
//                       avt_MADELINE_LUUFileFormat.h                        //
// ************************************************************************* //

#ifndef AVT_MADELINE_LUU_FILE_FORMAT_H
#define AVT_MADELINE_LUU_FILE_FORMAT_H

#include <avtSTSDFileFormat.h>
#include <string>
#include <vector>

// ****************************************************************************
//  Class: avt_MADELINE_LUUFileFormat
//
//  Purpose:
//      Reads the MADELINE_LUU custom file format (.madeline).
//
//      File format specification:
//
//        # Comments start with '#'
//
//        POINTS <N>
//        x0 y0 z0
//        x1 y1 z1
//        ...                      (N lines)
//
//        CELLS <M>
//        cellType p0 p1 p2 ...    (M lines; cellType 5 = VTK_TRIANGLE)
//
//        DATA
//        scalar0
//        scalar1
//        ...                      (one value per point)
//
//  Programmer: Madeline Luu
//  Creation:   May 2026
//
// ****************************************************************************

class avt_MADELINE_LUUFileFormat : public avtSTSDFileFormat
{
public:
                       avt_MADELINE_LUUFileFormat(const char *filename);
    virtual           ~avt_MADELINE_LUUFileFormat() {}

    // -----------------------------------------------------------------------
    // Required overrides
    // -----------------------------------------------------------------------
    virtual const char        *GetType()  { return "MADELINE_LUU"; }
    virtual void               FreeUpResources();

    virtual vtkDataSet        *GetMesh(const char *meshname);
    virtual vtkDataArray      *GetVar(const char *varname);
    virtual vtkDataArray      *GetVectorVar(const char *varname);

    virtual void               PopulateDatabaseMetaData(avtDatabaseMetaData *);

protected:
    void                       ReadFile();

    std::string                filename_;
    bool                       fileRead_;

    // Geometry
    std::vector<float>         points_;       // flat: x0,y0,z0, x1,y1,z1, ...
    std::vector<int>           cellTypes_;    // one per cell
    std::vector<std::vector<int>> cellConn_; // one inner vector per cell

    // Per-point scalar
    std::vector<float>         scalarData_;
};

#endif
