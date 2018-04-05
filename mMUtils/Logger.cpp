//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// Logger.cpp
//
// Copyright (C) Stephen Kew. All Rights Reserved.
//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "stdafx.h"       // main symbols
#include "Logger.h"
#include "psapi.h"
#include <time.h>

CLogger::CLogger(LPCSTR filename)
{
	InitialiseLoggerInfo();

	if( StartLogger( filename ) )
	{
		WriteLine( _T( "miniMIZE logger started" ) );
	}
}

CLogger::CLogger(LPCSTR filename, LPTSTR startupString)
{
	InitialiseLoggerInfo();

	if( StartLogger( filename ) )
	{
		WriteLine( startupString );
	}
}
	
void CLogger::InitialiseLoggerInfo()
{
	_newLine = true;
	_stream = NULL;
	_processId = GetCurrentProcessId();

	HANDLE hProcess = GetCurrentProcess();

	if( hProcess != NULL )
	{
		HMODULE hMod = NULL;
        DWORD cbNeeded = -1;

		GetModuleBaseName( hProcess, hMod, _processName, 
                               sizeof(_processName)/sizeof(TCHAR) );
	}
}

CLogger::~CLogger( )
{
	StopLogger( );
}

//void CLogger::MapLogString(LPCTSTR
void CLogger::WriteLine( LPCTSTR buffer )
{
	if( _stream != NULL )
	{
		time_t currentTime;
		TCHAR timeStr[26];
		int err;

		time( &currentTime );
		
		err = _tctime_s( timeStr, 26, &currentTime );

		if (err == 0)
		{
			timeStr[24] = '\0';
		  //printf("Invalid Arguments for _wctime_s. Error Code: %d\n", err);
		}
		
		_ftprintf_s( _stream, _T( "%s - [%d - %s] - %s\n" ),timeStr,_processId, _processName, buffer );
	}
}

void CLogger::Write( LPCTSTR buffer )
{
	Write(buffer, false);
}

void CLogger::Write( LPCTSTR buffer, bool EOL )
{
	if( _stream != NULL )
	{
		if( _newLine )
		{
			time_t currentTime;
			TCHAR timeStr[26];
			int err;

			time( &currentTime );
			
			err = _tctime_s( timeStr, 26, &currentTime );

			if (err == 0)
			{
				timeStr[24] = '\0';
			  //printf("Invalid Arguments for _wctime_s. Error Code: %d\n", err);
			}
			
			_ftprintf_s( _stream, _T( "%s - [%d - %s] - %s" ),timeStr,_processId, _processName, buffer );
		}
		else
		{
			if( EOL )
			{
				_newLine = true;
				_ftprintf_s( _stream, _T( " %s\n" ), buffer );
			}
			else
			{
				_ftprintf_s( _stream, _T( " %s" ), buffer );
			}
		}
	}
}

BOOL CLogger::StopLogger( void )
{
	// close the stream file
	if( _stream != NULL )
	{
		fclose( _stream );
	}

	return TRUE;
}

BOOL CLogger::StartLogger( LPCSTR filename )
{
	if( _stream == NULL )
	{
		if( (_stream  = _fsopen( filename, "a+", _SH_DENYNO)) == NULL )
		{
			// an error happened. Do something?
			return FALSE;
		}
	}

	return TRUE;
}