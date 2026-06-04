// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtStatisticsQuery.C                           //
// ************************************************************************* //

#include <avtStatisticsQuery.h>

#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkIdList.h>
#include <vtkPointData.h>
#include <vtkUnsignedCharArray.h>

#include <avtParallel.h>

#include <MapNode.h>

#include <float.h>
#include <math.h>

#include <string>

using     std::string;
using     std::vector;


// ****************************************************************************
//  Method: avtStatisticsQuery constructor
//
//  Programmer: VisIt issue #19952
//  Creation:   2026
//
// ****************************************************************************

avtStatisticsQuery::avtStatisticsQuery()
    : avtTwoPassDatasetQuery()
{
    totalvalues = 0;
    sum = 0.;
    mean = 0.;
    minval = +DBL_MAX;
    maxval = -DBL_MAX;
    csum2 = 0.;
    csum3 = 0.;
    csum4 = 0.;
    strcpy(descriptionBuffer, "Calculating statistics");
}


// ****************************************************************************
//  Method: avtStatisticsQuery destructor
//
//  Programmer: VisIt issue #19952
//  Creation:   2026
//
// ****************************************************************************

avtStatisticsQuery::~avtStatisticsQuery()
{
    ;
}


// ****************************************************************************
//  Method: avtStatisticsQuery::TupleIsGhost
//
//  Purpose:
//      Determines whether the i'th tuple of the scalar array corresponds to a
//      ghost element so it can be excluded from the statistics.  Mirrors the
//      ghost-handling logic in avtSummationQuery::Execute (VisIt issue #19952).
//
//  Programmer: VisIt issue #19952
//  Creation:   2026
//
// ****************************************************************************

bool
avtStatisticsQuery::TupleIsGhost(vtkDataSet *ds, bool cellData,
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
//  Method: avtStatisticsQuery::PreExecute
//
//  Programmer: VisIt issue #19952
//  Creation:   2026
//
// ****************************************************************************

void
avtStatisticsQuery::PreExecute(void)
{
    avtTwoPassDatasetQuery::PreExecute();

    totalvalues = 0;
    sum = 0.;
    mean = 0.;
    minval = +DBL_MAX;
    maxval = -DBL_MAX;
    csum2 = 0.;
    csum3 = 0.;
    csum4 = 0.;
}


// ****************************************************************************
//  Method: avtStatisticsQuery::Execute1
//
//  Purpose:
//      First pass: count, sum, min and max over non-ghost elements.
//
//  Programmer: VisIt issue #19952
//  Creation:   2026
//
// ****************************************************************************

void
avtStatisticsQuery::Execute1(vtkDataSet *ds, const int dom)
{
    bool cellData = true;
    vtkDataArray *data = ds->GetCellData()->GetScalars();
    if (!data)
    {
        data = ds->GetPointData()->GetScalars();
        cellData = false;
    }
    if (!data)
        return;

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
        if (v < minval)
            minval = v;
        if (v > maxval)
            maxval = v;
    }

    list->Delete();
}


// ****************************************************************************
//  Method: avtStatisticsQuery::MidExecute
//
//  Purpose:
//      Reduce the first-pass accumulators across processors and compute the
//      mean, which the second pass needs.
//
//  Programmer: VisIt issue #19952
//  Creation:   2026
//
// ****************************************************************************

void
avtStatisticsQuery::MidExecute(void)
{
    SumDoubleAcrossAllProcessors(sum);
    SumIntAcrossAllProcessors(totalvalues);
    minval = UnifyMinimumValue(minval);
    maxval = UnifyMaximumValue(maxval);

    if (totalvalues > 0)
        mean = sum / double(totalvalues);
    else
        mean = 0.;
}


// ****************************************************************************
//  Method: avtStatisticsQuery::Execute2
//
//  Purpose:
//      Second pass: central moments about the mean over non-ghost elements.
//
//  Programmer: VisIt issue #19952
//  Creation:   2026
//
// ****************************************************************************

void
avtStatisticsQuery::Execute2(vtkDataSet *ds, const int dom)
{
    bool cellData = true;
    vtkDataArray *data = ds->GetCellData()->GetScalars();
    if (!data)
    {
        data = ds->GetPointData()->GetScalars();
        cellData = false;
    }
    if (!data)
        return;

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
        double c = v - mean;
        csum2 += c*c;
        csum3 += c*c*c;
        csum4 += c*c*c*c;
    }

    list->Delete();
}


// ****************************************************************************
//  Method: avtStatisticsQuery::PostExecute
//
//  Purpose:
//      Combine the second-pass accumulators and report all measures.
//
//  Programmer: VisIt issue #19952
//  Creation:   2026
//
// ****************************************************************************

void
avtStatisticsQuery::PostExecute(void)
{
    SumDoubleAcrossAllProcessors(csum2);
    SumDoubleAcrossAllProcessors(csum3);
    SumDoubleAcrossAllProcessors(csum4);

    double N = (double) totalvalues;

    double variance = 0.;
    double stddev   = 0.;
    double skewness = 0.;
    double kurtosis = 0.;
    double range    = 0.;

    if (totalvalues > 0)
        range = maxval - minval;

    // Sample statistics (divide by N-1), matching the default behavior of the
    // "Sample Statistics" query.  Guard against tiny samples.
    if (N > 1.)
    {
        variance = csum2 / (N - 1.);
        stddev   = sqrt(variance);
    }
    if (N > 2. && csum2 > 0.)
    {
        skewness = (N * sqrt(N - 1.) / (N - 2.)) * csum3 / pow(csum2, 3./2.);
    }
    if (N > 3. && variance > 0.)
    {
        kurtosis = ((N+1.)*N / ((N-1.)*(N-2.)*(N-3.)))
                       * csum4 / (variance*variance)
                 - 3.*(N-1.)*(N-1.) / ((N-2.)*(N-3.));
    }

    // Report every measure in the result MapNode so the GUI/CLI can pick.
    MapNode result_node;
    result_node["count"]              = (double) totalvalues;
    result_node["sum"]                = sum;
    result_node["minimum"]            = minval;
    result_node["maximum"]            = maxval;
    result_node["range"]              = range;
    result_node["mean"]               = mean;
    result_node["variance"]           = variance;
    result_node["standard_deviation"] = stddev;
    result_node["skewness"]           = skewness;
    result_node["kurtosis"]           = kurtosis;

    string floatFormat = queryAtts.GetFloatFormat();
    string format =
              "Count     = %g\n"
              "Minimum   = " + floatFormat + "\n"
              "Maximum   = " + floatFormat + "\n"
              "Range     = " + floatFormat + "\n"
              "Sum       = " + floatFormat + "\n"
              "Mean      = " + floatFormat + "  (unweighted; use \"Average "
                  "Value\" for the size-weighted mean)\n"
              "Std Dev   = " + floatFormat + "\n"
              "Variance  = " + floatFormat + "\n"
              "Skewness  = " + floatFormat + "\n"
              "Kurtosis  = " + floatFormat + "\n";

    char msg[4096];
    snprintf(msg, 4096, format.c_str(),
             (double) totalvalues, minval, maxval, range, sum,
             mean, stddev, variance, skewness, kurtosis);

    doubleVector values;
    values.push_back((double) totalvalues);
    values.push_back(minval);
    values.push_back(maxval);
    values.push_back(range);
    values.push_back(sum);
    values.push_back(mean);
    values.push_back(stddev);
    values.push_back(variance);
    values.push_back(skewness);
    values.push_back(kurtosis);

    SetResultMessage(msg);
    SetResultValues(values);
    SetXmlResult(result_node.ToXML());
}
