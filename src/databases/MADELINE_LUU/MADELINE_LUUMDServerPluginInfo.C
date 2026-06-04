// ************************************************************************* //
//                    MADELINE_LUUMDServerPluginInfo.C                       //
// ************************************************************************* //

#include <MADELINE_LUUPluginInfo.h>

// ****************************************************************************
//  Function:  GetMDServerInfo
//
//  Purpose:   Entry point called by the MDServer to get plugin info.
//
//  Programmer: Madeline Luu
//  Creation:   May 2026
//
// ****************************************************************************

extern "C" MDServerDatabasePluginInfo *
GetMDServerInfo()
{
    return new MADELINE_LUUMDServerPluginInfo;
}

// ****************************************************************************
//  Method: MADELINE_LUUMDServerPluginInfo::Allocator
// ****************************************************************************

MDServerDatabasePluginInfo *
MADELINE_LUUMDServerPluginInfo::Allocator()
{
    return new MADELINE_LUUMDServerPluginInfo;
}
