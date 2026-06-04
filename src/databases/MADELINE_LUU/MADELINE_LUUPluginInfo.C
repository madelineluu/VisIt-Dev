// ************************************************************************* //
//                       MADELINE_LUUPluginInfo.C                            //
// ************************************************************************* //

#include <MADELINE_LUUPluginInfo.h>
#include <visit-config.h>
#include <database_plugin_exports.h>

extern "C" { __attribute__((visibility("default"))) const char *IMDELINE_LUUVisItPluginVersion = VISIT_VERSION; }
extern "C" { __attribute__((visibility("default"))) const char *MDELINE_LUUVisItPluginVersion = VISIT_VERSION; }
extern "C" { __attribute__((visibility("default"))) const char *MMDELINE_LUUVisItPluginVersion = VISIT_VERSION; }
extern "C" { __attribute__((visibility("default"))) const char *EMDELINE_LUUVisItPluginVersion = VISIT_VERSION; }

// ****************************************************************************
//  Function:  GetGeneralInfo
//
//  Purpose:   Entry point — returns a new GeneralPluginInfo.
//
//  Programmer: Madeline Luu
//  Creation:   May 2026
//
// ****************************************************************************

extern "C" GeneralDatabasePluginInfo *
GetGeneralInfo()
{
    return new MADELINE_LUUGeneralPluginInfo;
}

// ****************************************************************************
//  Method: MADELINE_LUUGeneralPluginInfo::GetName
// ****************************************************************************

const char *
MADELINE_LUUGeneralPluginInfo::GetName() const
{
    return "MADELINE_LUU";
}

// ****************************************************************************
//  Method: MADELINE_LUUGeneralPluginInfo::GetVersion
// ****************************************************************************

const char *
MADELINE_LUUGeneralPluginInfo::GetVersion() const
{
    return "1.0";
}

// ****************************************************************************
//  Method: MADELINE_LUUGeneralPluginInfo::GetID
// ****************************************************************************

const char *
MADELINE_LUUGeneralPluginInfo::GetID() const
{
    return "MADELINE_LUU_1.0";
}
