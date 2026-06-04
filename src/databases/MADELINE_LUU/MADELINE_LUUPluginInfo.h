// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         MADELINE_LUUPluginInfo.h                          //
// ************************************************************************* //

#ifndef MADELINE_LUU_PLUGIN_INFO_H
#define MADELINE_LUU_PLUGIN_INFO_H

#include <DatabasePluginInfo.h>
#include <string>
#include <vector>


class avtDatabase;
class avtDatabaseWriter;

// ****************************************************************************
//  Class: MADELINE_LUUDatabasePluginInfo
//
//  Purpose:
//    Classes that provide all the information about the MADELINE_LUU plugin.
//    Portions are separated into pieces that are used by the appropriate
//    components of VisIt.
//
//  Programmer: Madeline Luu
//  Creation:   May 2026
//
// ****************************************************************************

class MADELINE_LUUGeneralPluginInfo : public virtual GeneralDatabasePluginInfo
{
public:
    virtual const char *GetName() const;
    virtual const char *GetVersion() const;
    virtual const char *GetID() const;
    virtual std::vector<std::string> GetDefaultFilePatterns() const
    {
        std::vector<std::string> p;
        p.push_back("*.madeline");
        return p;
    }
    virtual bool NeedsByteSwapWhenReading() const { return false; }
    virtual bool HasWriter() const { return false; }
};

class MADELINE_LUUCommonPluginInfo : public virtual CommonDatabasePluginInfo,
                                     public virtual MADELINE_LUUGeneralPluginInfo
{
public:
    virtual DatabaseType GetDatabaseType();
    virtual avtDatabase *SetupDatabase(const char * const *list, int nList,
                                       int nBlock);
};

class MADELINE_LUUMDServerPluginInfo : public virtual MDServerDatabasePluginInfo,
                                       public virtual MADELINE_LUUCommonPluginInfo
{
public:
    static MDServerDatabasePluginInfo *Allocator();
    virtual ~MADELINE_LUUMDServerPluginInfo() {}
    virtual void dummy() {}
};

class MADELINE_LUUEnginePluginInfo : public virtual EngineDatabasePluginInfo,
                                     public virtual MADELINE_LUUCommonPluginInfo
{
public:
    static EngineDatabasePluginInfo *Allocator();
    virtual ~MADELINE_LUUEnginePluginInfo() {}
    virtual avtDatabaseWriter *GetWriter() { return NULL; }
};

#endif
