// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtStatisticsQuery.h                           //
// ************************************************************************* //

#ifndef AVT_STATISTICS_QUERY_H
#define AVT_STATISTICS_QUERY_H
#include <query_exports.h>

#include <avtTwoPassDatasetQuery.h>

class vtkDataSet;
class vtkIdList;
class vtkUnsignedCharArray;

// ****************************************************************************
//  Class: avtStatisticsQuery
//
//  Purpose:
//      A single, ghost-aware query that reports many descriptive statistics
//      for a scalar variable in one pass over the data: count, sum, minimum,
//      maximum, range, (unweighted arithmetic) mean, variance, standard
//      deviation, skewness, and kurtosis.  All measures are returned in the
//      result MapNode so the user/GUI can pick whichever is needed.
//
//      Created in response to VisIt issue #19952, which noted that the
//      existing statistics queries (a) silently double-counted ghost cells
//      and (b) were spread across several single-measure queries with
//      inconsistent terminology.  Ghost cells/nodes are excluded here.
//
//      Note: the size-weighted mean is intentionally NOT computed here; it is
//      available via the "Average Value" query.  The mean reported here is the
//      unweighted arithmetic mean.
//
//  Programmer: VisIt issue #19952
//  Creation:   2026
//
// ****************************************************************************

class QUERY_API avtStatisticsQuery : public avtTwoPassDatasetQuery
{
  public:
                                    avtStatisticsQuery();
    virtual                        ~avtStatisticsQuery();

    virtual const char             *GetType(void)
                                             { return "avtStatisticsQuery"; };
    virtual const char             *GetDescription(void)
                                             { return descriptionBuffer; };

  protected:
    int                             totalvalues;
    double                          sum;
    double                          mean;
    double                          minval;
    double                          maxval;
    double                          csum2;
    double                          csum3;
    double                          csum4;

    char                            descriptionBuffer[1024];

    virtual void                    PreExecute(void);
    virtual void                    Execute1(vtkDataSet *, const int);
    virtual void                    MidExecute(void);
    virtual void                    Execute2(vtkDataSet *, const int);
    virtual void                    PostExecute(void);

    static bool                     TupleIsGhost(vtkDataSet *ds, bool cellData,
                                        vtkUnsignedCharArray *ghostZones,
                                        vtkUnsignedCharArray *ghostNodes,
                                        int i, vtkIdList *list);
};


#endif
