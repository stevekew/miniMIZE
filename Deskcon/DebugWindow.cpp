//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// DebugWindow.cpp
//
//	Copyright (c) 2018 Stephen Kew
//
//	Permission is hereby granted, free of charge, to any person obtaining a copy
//	of this software and associated documentation files (the "Software"), to deal
//	in the Software without restriction, including without limitation the rights
//	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//	copies of the Software, and to permit persons to whom the Software is
//	furnished to do so, subject to the following conditions:
//
//	The above copyright notice and this permission notice shall be included in all
//	copies or substantial portions of the Software.
//
//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//	SOFTWARE.
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
