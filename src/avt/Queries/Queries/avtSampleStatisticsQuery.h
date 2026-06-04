// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                        avtSampleStatisticsQuery.h                         //
// ************************************************************************* //

#ifndef AVT_SAMPLE_STATISTICS_QUERY_H
#define AVT_SAMPLE_STATISTICS_QUERY_H
#include <query_exports.h>

#include <avtTwoPassDatasetQuery.h>

class vtkDataSet;
class vtkCell;
class vtkIdList;
class vtkUnsignedCharArray;

// ****************************************************************************
//  Class: avtSampleStatisticsQuery
//
//  Purpose:
//      This query calculates sample statistics on a data array.
//
//  Programmer: Jeremy Meredith
//  Creation:   March 11, 2009
//
//  Modifications:
//
// ****************************************************************************

class QUERY_API avtSampleStatisticsQuery : public avtTwoPassDatasetQuery
{
  public:
                                    avtSampleStatisticsQuery(bool population);
    virtual                        ~avtSampleStatisticsQuery();

    virtual const char             *GetType(void)
                                             { return "avtSampleStatisticsQuery"; };
    virtual const char             *GetDescription(void)
                                             { return descriptionBuffer; };

  protected:
    bool                            populationStatistics;
    int                             totalvalues;
    double                          sum;
    double                          mean;
    double                          csum2;
    double                          csum3;
    double                          csum4;

    char                            descriptionBuffer[1024];

    virtual void                    PreExecute(void);
    virtual void                    Execute1(vtkDataSet *, const int);
    virtual void                    MidExecute();
    virtual void                    Execute2(vtkDataSet *, const int);
    virtual void                    PostExecute(void);

    // Returns true if the i'th tuple of the scalar array belongs to a ghost
    // element (cell or node) and should be excluded from the statistics.
    // See VisIt issue #19952.
    static bool                     TupleIsGhost(vtkDataSet *ds, bool cellData,
                                        vtkUnsignedCharArray *ghostZones,
                                        vtkUnsignedCharArray *ghostNodes,
                                        int i, vtkIdList *list);
};


#endif


