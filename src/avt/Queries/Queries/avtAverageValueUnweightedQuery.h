// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                      avtAverageValueUnweightedQuery.h                     //
// ************************************************************************* //

#ifndef AVT_AVERAGE_VALUE_UNWEIGHTED_QUERY_H
#define AVT_AVERAGE_VALUE_UNWEIGHTED_QUERY_H

#include <query_exports.h>

#include <avtVariableSummationQuery.h>


// ****************************************************************************
//  Class: avtAverageValueUnweightedQuery
//
//  Purpose:
//      A query that computes the unweighted arithmetic mean of a variable,
//      i.e. Sum(var) / count over non-ghost elements.  This is the number most
//      users expect from an "average", as opposed to the size-weighted mean
//      computed by avtAverageValueQuery.  See VisIt issue #19952.
//
//  Programmer: VisIt issue #19952
//  Creation:   2026
//
// ****************************************************************************

class QUERY_API avtAverageValueUnweightedQuery
    : public avtVariableSummationQuery
{
  public:
                         avtAverageValueUnweightedQuery();
    virtual             ~avtAverageValueUnweightedQuery();

    virtual const char  *GetType(void)
                             { return "avtAverageValueUnweightedQuery"; };

  protected:
    virtual bool         CalculateAverage(void) { return true; };
    virtual bool         AverageByCount(void)   { return true; };
};


#endif
