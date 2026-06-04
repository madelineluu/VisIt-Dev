// ************************************************************************* //
//                     MADELINE_LUUCommonPluginInfo.C                        //
// ************************************************************************* //

#include <MADELINE_LUUPluginInfo.h>
#include <avt_MADELINE_LUUFileFormat.h>
#include <avtSTSDFileFormatInterface.h>
#include <avtGenericDatabase.h>

// ****************************************************************************
//  Method: MADELINE_LUUCommonPluginInfo::GetDatabaseType
//
//  Purpose:
//      Returns the type of database — Single Time Single Domain (STSD).
//
//  Programmer: Madeline Luu
//  Creation:   May 2026
//
// ****************************************************************************

DatabaseType
MADELINE_LUUCommonPluginInfo::GetDatabaseType()
{
    return DB_TYPE_STSD;
}

// ****************************************************************************
//  Method: MADELINE_LUUCommonPluginInfo::SetupDatabase
//
//  Purpose:
//      Sets up a MADELINE_LUU database.
//
//  Programmer: Madeline Luu
//  Creation:   May 2026
//
// ****************************************************************************

avtDatabase *
MADELINE_LUUCommonPluginInfo::SetupDatabase(const char * const *list,
                                             int nList, int nBlock)
{
    avtSTSDFileFormat **ffl = new avtSTSDFileFormat *[nList];
    for (int i = 0; i < nList; i++)
    {
        ffl[i] = new avt_MADELINE_LUUFileFormat(list[i]);
    }
    avtSTSDFileFormatInterface *inter
        = new avtSTSDFileFormatInterface(&ffl, nList, nBlock);
    return new avtGenericDatabase(inter);
}
