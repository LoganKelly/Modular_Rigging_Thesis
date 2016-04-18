/*************************************************************
* Summary: Defines some macros useful for making error       *
*          checking in maya plugins less labor-intensive     *
*  Author: Logan Kelly                                       *
*    Date: 8/31/12                                           *
*************************************************************/

#include <maya/MString.h>
#include <maya/MGlobal.h>

inline MString MyFormatError( const MString &msg, const MString
                              &sourceFile, const int &sourceLine )
{
    MString txt( "[MetaDataRigging] " );
    txt += msg;
    txt += ", File: ";
    txt += sourceFile;
    txt += " Line: ";
    txt += sourceLine;
    return txt;
}

#define MyError( msg ) \
    { \
    MString __txt = MyFormatError( msg, __FILE__, __LINE__ ); \
    MGlobal::displayError( __txt ); \
    cerr << endl << "Error: " << __txt; \
    } \

#define MyCheckBool( result ) \
    if( !(result) ) \
        { \
        MyError( #result ); \
        }

#define MyCheckStatus( stat, msg ) \
    if( !stat ) \
        { \
        MyError( msg ); \
        }

#define MyCheckObject( obj, msg ) \
    if( obj.isNull() ) \
        { \
        MyError( msg ); \
        }

#define MyCheckStatusReturn( stat, msg ) \
    if( !stat ) \
        { \
        MyError( msg ); \
        return stat; \
        }
