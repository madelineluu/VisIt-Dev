// ************************************************************************* //
//                     MADELINE_LUUEnginePluginInfo.C                        //
// ************************************************************************* //

#include <MADELINE_LUUPluginInfo.h>

// ****************************************************************************
//  Function:  GetEngineInfo
//
//  Purpose:   Entry point called by the compute engine to get plugin info.
//
//  Programmer: Madeline Luu
//  Creation:   May 2026
//
// ****************************************************************************

extern "C" EngineDatabasePluginInfo *
GetEngineInfo()
{
    return new MADELINE_LUUEnginePluginInfo;
}

// ****************************************************************************
//  Method: MADELINE_LUUEnginePluginInfo::Allocator
// ****************************************************************************

EngineDatabasePluginInfo *
MADELINE_LUUEnginePluginInfo::Allocator()
{
    return new MADELINE_LUUEnginePluginInfo;
}
