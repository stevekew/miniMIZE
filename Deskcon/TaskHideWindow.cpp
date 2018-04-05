//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// TaskHideWindow.cpp
//
// Copyright (C) Stephen Kew. All Rights Reserved.
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
