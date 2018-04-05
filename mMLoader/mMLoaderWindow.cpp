//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// mMLoaderWindow.cpp
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
#include "resource.h"       // main symbols
#include "mMLoader.h"

//
// CMMLoaderWindow - main miniMIZE window class
//
class CMMLoaderWindow : public CWindowImpl< CMMLoaderWindow >,
						public CComObjectRootEx<CComSingleThreadModel>,
						public IMMLoaderWindow
{
private:
	UINT UWM_SHUTDOWN_MMLOADER;

public:
	CMMLoaderWindow()
	{
		UWM_SHUTDOWN_MMLOADER = ::RegisterWindowMessage( UWM_SHUTDOWN_MMLOADER_MSG );
	}

	~CMMLoaderWindow( )
	{
	}

DECLARE_WND_CLASS( _T( "miniMIZE_Loader_Window" ) )

//
// IUnknown
//
BEGIN_COM_MAP( CMMLoaderWindow )
	COM_INTERFACE_ENTRY( IMMLoaderWindow )
END_COM_MAP( )

//
// WndProc
//
BEGIN_MSG_MAP(CMMLoaderWindow)
	MESSAGE_HANDLER( WM_NCCREATE, OnNCCreate )
	MESSAGE_HANDLER( WM_DESTROY, OnDestroy )
	MESSAGE_HANDLER( WM_CLOSE, OnClose )
	MESSAGE_HANDLER( UWM_SHUTDOWN_MMLOADER, OnShutdownLoader )
END_MSG_MAP()

LRESULT OnNCCreate( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled )
{
	AddRef( );
	return 1;
}
LRESULT OnDestroy( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled )
{
	return 1;
}
LRESULT OnClose( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled )
{
	Shutdown( );
	return 1;
}

LRESULT OnShutdownLoader( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled )
{
	SendMessage( WM_CLOSE );
	return 1;
}

virtual void OnFinalMessage( HWND hWnd )
{
	Release( );
}
public:
//
// IDeskconWindow
//

STDMETHODIMP_( HWND )Initialize( )
{
	HWND wnd = Create( NULL, CWindow::rcDefault, _T(""), WS_POPUP, WS_EX_TOOLWINDOW );

	return wnd;
}

STDMETHODIMP_( void )Close ( void )
{
	PostMessage( WM_CLOSE, 0, 0 );
}
STDMETHODIMP_( HWND )GetHWND( void )
{
	return m_hWnd;
}

private:
void Shutdown( )
{
	DestroyWindow( ); 
	PostQuitMessage(0);
}

};

HRESULT CreateLoaderWindow( IMMLoaderWindow **ppLoaderWindow )
{
	return CComCreator< CComObject< CMMLoaderWindow > >::CreateInstance( NULL, __uuidof( IMMLoaderWindow ), ( void** ) ppLoaderWindow );
}
