//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// PassportControl.cpp
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

#include "stdafx.h"
#include "deskcon.h"
#include "deskconinternal.h"
#include <vector>

using namespace std;

extern CComPtr< IDebugWindow > _debugWindow;
extern HINSTANCE _hInstance;
extern HWND _mainHwnd;
extern vector< CComPtr< IThumbWindow > > _deskconWindows;

#define TIMEREVENT 0

static BOOL CALLBACK EnumWindowsProcProxy( HWND hwnd, LPARAM lParam );

class CPassportCtl : public CWindowImpl<CPassportCtl>,
						public CComObjectRootEx<CComSingleThreadModel>,
						public IPassportCtl
{
private:
	UINT m_pollRate;
	bool m_timerCreated;

public:
	CPassportCtl()
	{
		// start with a 500ms pollrate
		m_pollRate = 500;
		m_timerCreated = false;
	}

	~CPassportCtl()
	{
		Stop( );
	}

//
// IUnknown
//
BEGIN_COM_MAP(CPassportCtl)
	COM_INTERFACE_ENTRY(IPassportCtl)
END_COM_MAP()

BEGIN_MSG_MAP(CPassportCtl)
		MESSAGE_HANDLER( WM_NCCREATE, OnNCCreate )
		MESSAGE_HANDLER( WM_TIMER, OnTimer )
END_MSG_MAP()

public:
LRESULT OnNCCreate( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled )
{
    AddRef( );
	return 1;
}

LRESULT OnTimer( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled )
{
	if( ( ( UINT )wParam ) == TIMEREVENT )
	{
		bHandled = true;
		
		// timer has elapsed, so check our windows here
		if( !::EnumWindows( EnumWindowsProcProxy, ( LPARAM )this ) )
		{
			printf( "EnumWindows failed" );
		}

		return 0;
	}

	return 1;
}

BOOL EnumWindowsProc( HWND hwnd, LPARAM lParam )
{
	vector< CComPtr< IThumbWindow > >::iterator it;
	BOOL minimized = FALSE;
	BOOL windowFound = FALSE;
	TCHAR buffer[100];
	GetClassName( hwnd, buffer, 100 );

	// ok, so we have a window, is it an app window?
	if( IsAppWindow( hwnd ) )
	{
		minimized = ::IsIconic( hwnd );


		// ok, so we have an app window, what about it?
		// lets see if it is in our list
		if( !_deskconWindows.empty( ) )
		{
			for( it = _deskconWindows.begin( ); it != _deskconWindows.end( ); it++ )
			{
				if( (*it)->GetMonitoredWindow( ) == hwnd )
				{
					// ok, so here are we showing or are we hiding?
					if( minimized && !(*it)->IsVisible( ) )
					{
						(*it)->Show( );
					}
					else if( !minimized && (*it)->IsVisible( ) )
					{
						(*it)->Hide( );
					}
					else if( !minimized )
					{
						// should we update the ss?
						(*it)->WindowMinimized();//Initialize( hwnd, 0, 0, (*it)->IgnoreBlack( ),  false );
					}

					windowFound = true;
					break;
				}
			}
		}
	
		if( !windowFound )
		{
			if( !minimized )
			{
				bool ignoreBlack = false;
				TCHAR buffer[100];
				GetClassName( hwnd, buffer, 100 );

				if( _tcsicmp( buffer, _T( "consolewindowclass" ) ) == 0 )
					ignoreBlack = true;

				// it wasn't in our list, so lets add it, but only if we can ss it
				CComPtr< IThumbWindow > win;

				CreateThumbWindow( &win );

				win->Initialize( hwnd, 0, 0, ignoreBlack);

				int x = 0;
				int y = 0;
				
				//CalculateAvailablePosition( win->GetHWND( ), &x, &y );

				win->SetLocation( x, y );

				_deskconWindows.push_back( win );
			}
		}
	}

	return TRUE;
}

virtual void OnFinalMessage( HWND hWnd )
{
	Release( );
}

public:

STDMETHODIMP_( BOOL )Initialize( void )
{
	return TRUE;
}

STDMETHODIMP_( BOOL )Initialize( UINT pollrate )
{
	m_pollRate = pollrate;

	return TRUE;
}

STDMETHODIMP_( BOOL )Start( void )
{
	if( !m_hWnd )
		Create( NULL , CWindow::rcDefault, _T(""), WS_POPUP, WS_EX_TOOLWINDOW );

	BOOL retval = FALSE;

	// set the timer
	if( SetTimer( TIMEREVENT, m_pollRate ) )
	{
		m_timerCreated = true;

		retval = TRUE;
	}

	return retval;
}

STDMETHODIMP_( BOOL )Stop( void )
{
	BOOL retval = FALSE;

	if( m_timerCreated )
	{
		KillTimer( TIMEREVENT );

		return TRUE;
	}

	return retval;
}

STDMETHODIMP_( BOOL )SetPollRate( UINT pollrate )
{
	m_pollRate = pollrate;

	return TRUE;
}

private:
BOOL IsAppWindow( HWND hwnd )
{
	// only applies to visible windows
	if( ::IsWindowVisible( hwnd ) )
	{
		DWORD exStyle = ::GetWindowLong( hwnd, GWL_EXSTYLE );
		DWORD style = ::GetWindowLong( hwnd, GWL_STYLE );
		HWND parent = ( HWND )::GetWindowLong( hwnd, GWLP_HWNDPARENT );
		HWND owner = ::GetWindow(hwnd, GW_OWNER);

		// if WS_EX_APPWINDOW is set then we have 
		if( ( exStyle & WS_EX_APPWINDOW ) == WS_EX_APPWINDOW )
		{
			return TRUE;
		}

		// otherwise, if it has no parent and isn't a child window
		if( ( parent == NULL ) && (  owner == NULL ) )
		{
			if( ( exStyle & WS_EX_TOOLWINDOW ) != WS_EX_TOOLWINDOW )
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}
};

static BOOL CALLBACK EnumWindowsProcProxy( HWND hwnd, LPARAM lParam )
{
	return  ( (CPassportCtl*)( lParam ) )->EnumWindowsProc( hwnd, lParam );
}

HRESULT CreatePassportControl( IPassportCtl **ppPassportCtl )
{
	return CComCreator< CComObject< CPassportCtl > >::CreateInstance( NULL, __uuidof( IPassportCtl ), ( void** ) ppPassportCtl );
}