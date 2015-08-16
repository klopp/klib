/*
 * mkpath.c, part of "klib" project.
 *
 *  Created on: 05.06.2015, 16:27
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

#include "mkpath.h"
#include <errno.h>
#include <sys/stat.h>
#include "../stringlib/stringlib.h"
#if defined(__WINDOWS__)
# include <direct.h>
# define S_ISDIR(s) ((s) & S_IFDIR)
#endif

static int _mkpath( const char *path, mode_t mode )
{
    struct stat st;
    int status = 0;

    if( stat( path, &st ) != 0 )
    {
#if defined(__WINDOWS__)
        if( _mkdir( path ) != 0 ) status = -1;
#else
        if( mkdir( path, mode ) != 0 && errno != EEXIST ) status = -1;
#endif
    }
    else if( !S_ISDIR( st.st_mode ) )
    {
        errno = ENOTDIR;
        status = -1;
    }

    return status;
}

int mkpath( const char *path, mode_t mode )
{
    //char * pp;
    char * sp;
    int status = 0;
    char copypath[PATH_MAX + 1];

    strncpy( copypath, path, sizeof(copypath) - 1 );
    status = 0;
    sp = strtok( copypath, "\\/" );
    while( status == 0 && sp )
    {
#if defined(__WINDOWS__)
        if( copypath[1] == ':' && !copypath[2] )
        {}
        else
        {
#endif
        status = _mkpath( copypath, mode );
#if defined(__WINDOWS__)
    }
#endif
        *(sp + strlen( sp )) = '/';
        sp = strtok( NULL, "\\/" );
    }
    /*
     pp = copypath;
     while( status == 0 && (sp = strchr( pp, '/' )) != 0 )
     {
     if( sp != pp )
     {
     *sp = 0;
     status = _mkdir( copypath, mode );
     *sp = '/';
     }
     pp = sp + 1;
     }

     */
    if( status == 0 ) status = _mkpath( path, mode );
    return status == 0;
}

FILE * openpath( const char * file, const char * mode, mode_t dirmode )
{
    char copypath[PATH_MAX + 1];
    char * p;

    strncpy( copypath, file, sizeof(copypath) - 1 );
    p = strprbrk( copypath, "/\\" );
    if( !p )
    {
        p = copypath;
    }
    else
    {
        *p = 0;
        if( !mkpath( copypath, dirmode ) )
        {
            return NULL;
        }
    }
    return fopen( file, mode );
}

char * expand_home( const char * path )
{
    char * hpath;
    char * hdrive = NULL;
    char * fullpath;

    if( path[0] != '~' ) return NULL;

    hpath = getenv( "HOME" );
    if( !hpath )
    {
        hdrive = getenv( "HOMEDRIVE" );
        hpath = getenv( "HOMEPATH" );
    }
    if( !hpath ) return NULL;

    fullpath = Malloc(
            strlen( path ) + strlen( hpath ) + (hdrive ? strlen( hdrive ) : 0)
                    + 1 );
    if( !fullpath ) return NULL;
    strcpy( fullpath, hdrive ? hdrive : "" );
    strcat( fullpath, hpath );
    strcat( fullpath, path + 1 );

    return fullpath;
}
