// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                      avtSampleStatisticsQuery.C                           //
// ************************************************************************* //

#include <avtSampleStatisticsQuery.h>

#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkDataSetRemoveGhostCells.h>
#include <vtkIdList.h>
#include <vtkPointData.h>
#include <vtkUnsignedCharArray.h>

#include <avtCallback.h>
#include <avtParallel.h>

#include <DebugStream.h>

#include <float.h>
#include <math.h>

using     std::string;
using     std::vector;


// ****************************************************************************
//  Method: avtSampleStatisticsQuery constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   March 11, 2009
//
//  Modifications:
//
// ****************************************************************************

avtSampleStatisticsQuery::avtSampleStatisticsQuery(bool pop)
    : avtTwoPassDatasetQuery()
{
    populationStatistics = pop;
    strcpy(descriptionBuffer, "Calculating sample statistics");
}


// ****************************************************************************
//  Method: avtSampleStatisticsQuery destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Jeremy Meredith
//  Creation:   March 11, 2009
//
// ****************************************************************************

avtSampleStatisticsQuery::~avtSampleStatisticsQuery()
{
    ;
}


// ****************************************************************************
//  Method: avtSampleStatisticsQuery::PreExecute
//
//  Purpose:
//      This is called before all of the domains are executed.
//
//  Programmer: Jeremy Meredith
//  Creation:   March 11, 2009
//
//  Modifications:
//
// ****************************************************************************

void
avtSampleStatisticsQuery::PreExecute(void)
{
    avtTwoPassDatasetQuery::PreExecute();

    totalvalues = 0;
    sum = 0;
    mean = 0;
    csum2 = 0;
    csum3 = 0;
    csum4 = 0;
}


// ****************************************************************************
//  Method: avtSampleStatisticsQuery::TupleIsGhost
//
//  Purpose:
//      Determines whether the i'th tuple of the scalar array corresponds to a
//      ghost element so it can be excluded from the statistics.  Mirrors the
//      ghost-handling logic in avtSummationQuery::Execute.  Previously the
//      statistics queries counted ghost cells/nodes, which double-counted data
//      at domain boundaries and skewed the mean/variance (VisIt issue #19952).
//
//  Programmer: VisIt issue #19952
//  Creation:   2026
//
// ****************************************************************************

bool
avtSampleStatisticsQuery::TupleIsGhost(vtkDataSet *ds, bool cellData,
                                       vtkUnsignedCharArray *ghostZones,
                                       vtkUnsignedCharArray *ghostNodes,
                                       int i, vtkIdList *list)
{
    if (cellData)
    {
        if (ghostZones != NULL)
        {
            return (ghostZones->GetValue(i) != 0);
        }
        else if (ghostNodes != NULL)
        {
            // A cell is ghost only if all of its nodes are ghost.
            bool allGhost = true;
            ds->GetCellPoints(i, list);
            for (int j = 0 ; j < list->GetNumberOfIds() ; j++)
            {
                if (ghostNodes->GetValue(list->GetId(j)) == 0)
                    allGhost = false;
            }
            return allGhost;
        }
    }
    else
    {
        if (ghostNodes != NULL)
        {
            return (ghostNodes->GetValue(i) != 0);
        }
        else if (ghostZones != NULL)
        {
            // A node is ghost only if all of its incident cells are ghost.
            ds->GetPointCells(i, list);
            int nghost = 0;
            for (int j = 0 ; j < list->GetNumberOfIds(); j++)
                nghost += ghostZones->GetValue(list->GetId(j)) > 0 ? 1 : 0;
            return (nghost == list->GetNumberOfIds());
        }
    }
    return false;
}


// ****************************************************************************
//  Method: avtSampleStatisticsQuery::Execute
//
//  Purpose:
//      Processes a single domain.
//
//  Programmer: Jeremy Meredith
//  Creation:   March 11, 2009
//
//  Modifications:
//    VisIt issue #19952: skip ghost cells/nodes so they are not double-counted.
//
// ****************************************************************************

void
avtSampleStatisticsQuery::Execute1(vtkDataSet *ds, const int dom)
{
    bool cellData = true;
    vtkDataArray *data = ds->GetCellData()->GetScalars();
    if (!data)
    {
        data = ds->GetPointData()->GetScalars();
        cellData = false;
    }
    if (!data)
    {
        cerr << "ERROR\n";
        return;
    }

    vtkUnsignedCharArray *ghostZones = (vtkUnsignedCharArray *)
                                  ds->GetCellData()->GetArray("avtGhostZones");
    vtkUnsignedCharArray *ghostNodes = (vtkUnsignedCharArray *)
                                 ds->GetPointData()->GetArray("avtGhostNodes");

    int n = data->GetNumberOfTuples();
    vtkIdList *list = vtkIdList::New();

    for (int i=0; i<n; i++)
    {
        if (TupleIsGhost(ds, cellData, ghostZones, ghostNodes, i, list))
            continue;

        double v = data->GetTuple1(i);
        sum += v;
        totalvalues++;
    }

    list->Delete();
}


// ****************************************************************************
//  Method:  avtSampleStatisticsQuery::MidExecute
//
//  Purpose:
//    This is called after all of the domains are executed the first time.
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 11, 2009
//
//  Modifications:
//
//   Dave Pugmire, Mon Apr  4 12:29:32 EDT 2011
//   Sum total number of values, and compute "sum" as the sum of each proc's "sum".
//
// ****************************************************************************

void
avtSampleStatisticsQuery::MidExecute(void)
{
    SumDoubleAcrossAllProcessors(sum);
    SumIntAcrossAllProcessors(totalvalues);
    mean = sum / double(totalvalues);
}


// ****************************************************************************
//  Method: avtSampleStatisticsQuery::Execute2
//
//  Purpose:
//      Processes a single domain -- second pass.
//
//  Programmer: Jeremy Meredith
//  Creation:   March 11, 2009
//
//  Modifications:
//
// ****************************************************************************

void
avtSampleStatisticsQuery::Execute2(vtkDataSet *ds, const int dom)
{
    bool cellData = true;
    vtkDataArray *data = ds->GetCellData()->GetScalars();
    if (!data)
    {
        data = ds->GetPointData()->GetScalars();
        cellData = false;
    }
    if (!data)
    {
        cerr << "ERROR\n";
        return;
    }

    vtkUnsignedCharArray *ghostZones = (vtkUnsignedCharArray *)
                                  ds->GetCellData()->GetArray("avtGhostZones");
    vtkUnsignedCharArray *ghostNodes = (vtkUnsignedCharArray *)
                                 ds->GetPointData()->GetArray("avtGhostNodes");

    int n = data->GetNumberOfTuples();
    vtkIdList *list = vtkIdList::New();

    for (int i=0; i<n; i++)
    {
        if (TupleIsGhost(ds, cellData, ghostZones, ghostNodes, i, list))
            continue;

        double v = data->GetTuple1(i);
        double c = v-mean;
        csum2 += c*c;
        csum3 += c*c*c;
        csum4 += c*c*c*c;
    }

    list->Delete();
}


// ****************************************************************************
//  Method: avtSampleStatisticsQuery::PostExecute
//
//  Purpose:
//      This is called after all of the domains are executed.
//
//  Programmer: Jeremy Meredith
//  Creation:   March 11, 2009
//
//  Modifications:
//
// ****************************************************************************

void
avtSampleStatisticsQuery::PostExecute(void)
{
    doubleVector values;

    double N = totalvalues;
    SumDoubleAcrossAllProcessors(csum2);
    SumDoubleAcrossAllProcessors(csum3);
    SumDoubleAcrossAllProcessors(csum4);
    
    double cmoment2 = csum2 / N;
    double cmoment3 = csum3 / N;
    double cmoment4 = csum4 / N;

    double variance;
    double stddev;
    double skewness;
    double kurtosis;

    if (populationStatistics)
    {
        variance = cmoment2;
        skewness = cmoment3 / pow(cmoment2, 3./2.);
        kurtosis = cmoment4 / (cmoment2*cmoment2) - 3;
    }
    else
    {
        variance = csum2 / (N-1);
        skewness = (N * sqrt(N-1) / (N-2)) * csum3 / pow(csum2, 3./2.);
        kurtosis = ((N+1)*N / ((N-1)*(N-2)*(N-3))) * csum4/(variance*variance)
                  - 3*(N-1)*(N-1)/((N-2)*(N-3));
    }
    stddev = sqrt(variance);

    //
    //  Parent class uses this message to set the Results message
    //  in the Query Attributes that is sent back to the viewer.
    //  That is all that is required of this query.
    //
    char msg[4096];
    string floatFormat = queryAtts.GetFloatFormat();
    string format =
              "Mean      = "   + floatFormat + "\n"
              "Std Dev   = "   + floatFormat + "\n"
              "Variance  = "   + floatFormat + "\n"
              "Skewness  = "   + floatFormat + "\n"
              "Kurtosis  = "   + floatFormat + "\n";

    snprintf(msg, 4096,format.c_str(),
             mean, stddev, variance, skewness, kurtosis);

    values.push_back(mean);
    values.push_back(stddev);
    values.push_back(variance);
    values.push_back(skewness);
    values.push_back(kurtosis);

    SetResultMessage(msg);
    SetResultValues(values);
}
