//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// DebugWindow.cpp
//
// Copyright (C) Stephen Kew. All Rights Reserved.
//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "stdafx.h"       // main symbols
#include "Deskcon.h"
#include <iostream>

using namespace std;

// CDebugWindow
class CDebugWindow : public CComObjectRootEx<CComMultiThreadModel>,
					public IDebugWindow
{
private:
	// data variables
	FILE* _stream;

public:
	CDebugWindow()
	{
		OpenWindow( );
	}
	
	virtual ~CDebugWindow( )
	{
		CloseWindow( );
	}

//
// IUnknown
//
BEGIN_COM_MAP(CDebugWindow)
	COM_INTERFACE_ENTRY(IDebugWindow)
END_COM_MAP()

public:
	//
	// IDebugWindow
	//

	STDMETHODIMP_( void )WriteLine( LPCTSTR buffer )
	{
		_tprintf( _T( "%s\n" ), buffer );
	}

	STDMETHODIMP_( void )Write( LPCTSTR buffer )
	{
		_tprintf( _T( "%s" ), buffer );
	}

	STDMETHODIMP_( BOOL )CloseWindow( void )
	{
		// close the stream file
		fclose( _stream );

		// free the console
		FreeConsole( );

		return TRUE;
	}

	STDMETHODIMP_( BOOL )OpenWindow( void )
	{
		// when we create the debug window, allocate the console
		AllocConsole();

		// attach the std out to it
		_stream = freopen("CONOUT$", "w", stdout);
		
		// sync it to cout
		cout.sync_with_stdio(); 

		WriteLine( _T( "miniMIZE Debug Console" ) );

		return TRUE;
	}

	/*STDMETHODIMP_( void ) WriteFormat( char* buffer, ... )
	{
		printf( buffer );
	}*/

};

HRESULT CreateDebugWindow( IDebugWindow **ppDebugWindow )
{
	return CComCreator< CComObject< CDebugWindow > >::CreateInstance( NULL, __uuidof( IDebugWindow ), ( void** ) ppDebugWindow );
}
