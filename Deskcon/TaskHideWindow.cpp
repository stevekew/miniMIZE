//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// TaskHideWindow.cpp
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
#include "Deskcon.h"
#include "deskconinternal.h"
#include "settings.h"

//
// Externs
//
extern HINSTANCE _hInstance;
extern HWND _mainHwnd;
extern CComPtr< IDebugWindow > _debugWindow;

//
// CTaskHideWindow
//
class CTaskHideWindow : public CWindowImpl< CTaskHideWindow >,
						public CComObjectRootEx<CComSingleThreadModel>,
						public ITaskHideWindow
	
{
private:

public:
	CTaskHideWindow()
	{
	}

	~CTaskHideWindow( )
	{
	}

DECLARE_WND_CLASS( _T( "miniMIZE_TaskHide_Window" ) )

//
// IUnknown
//
BEGIN_COM_MAP( CTaskHideWindow )
	COM_INTERFACE_ENTRY( ITaskHideWindow )
END_COM_MAP( )

//
// WndProc
//
BEGIN_MSG_MAP(CTaskHideWindow)
	MESSAGE_HANDLER( WM_NCCREATE, OnNCCreate )
	MESSAGE_HANDLER( WM_DESTROY, OnDestroy )
	MESSAGE_HANDLER( WM_CLOSE, OnClose )
END_MSG_MAP()

LRESULT OnNCCreate( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled )
{
	AddRef( );
	return 1;
}

LRESULT OnDestroy( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled )
{
	SAFEDEBUGWRITELINE( _debugWindow, _T( "Taskhide window destroyed" ) )
	return 1;
}
LRESULT OnClose( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled )
{
	Shutdown( );
	return 1;
}
virtual void OnFinalMessage( HWND hWnd )
{
	Release( );
}


private:
void Shutdown( )
{
	SAFEDEBUGWRITELINE( _debugWindow, _T( "Shutting Down" ) )
	DestroyWindow( ); 
}

public:

//
// ITaskHideWindow
//
STDMETHODIMP_( HWND )Initialize( HWND parent )
{
	HWND wnd = Create( parent, CWindow::rcDefault, _T(""), WS_POPUP );

	SetWindowPos( NULL, 0, 10000, 0, 0, NULL );

	ShowWindow( SW_SHOW );

	return wnd;
}

STDMETHODIMP_( void )Close ( void )
{
	PostMessage( WM_CLOSE, 0, 0 );
}
};
HRESULT CreateTaskHideWindow( ITaskHideWindow **ppTaskHideWindow )
{
	return CComCreator< CComObject< CTaskHideWindow > >::CreateInstance( NULL, __uuidof( ITaskHideWindow ), ( void** ) ppTaskHideWindow );
}
