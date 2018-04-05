//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// ThumbnailBase.cpp
//
// Copyright (C) Stephen Kew. All Rights Reserved.
//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "stdafx.h"
#include "deskcon.h"
#include "deskconinternal.h"
#include "image_funcs.h"
#include "Predicates.h"

#include <gdiplus.h>
#include <shlobj.h>

//
// Externs
//
extern CComPtr< IDebugWindow > _debugWindow;
extern CComPtr< ITaskbarList > _taskbar;
extern vector< CComPtr< IThumbWindow > > _deskconWindows;
extern bool _useSnapto;
extern bool _customShadows;

extern HINSTANCE _hInstance;
extern HWND _mainHwnd;

#define TAKESNAPSHOT_COUNT 10
#define FLASHTIMER 100

// shadows
#define SHADOW_SIZE 5
#define SHADOW_ALPHA 0.6

const float blurFactor = 4.0f;
const int xOffset = 10;
const int yOffset = 10;

#define CAPTION_SIZE 20

template<class T>
class CThumbnailBase : public CWindowImpl<T>,
						public CComObjectRootEx<CComSingleThreadModel>,
						public IThumbWindow
{
protected:
	Gdiplus::Bitmap *m_windowBitmap;
	Gdiplus::Bitmap *shadowBmp;
	Gdiplus::Bitmap *iconBmp;

	int m_xPos;
	int m_yPos;
	int m_width;
	int m_height;
	int m_defSize;
	int m_opacity;

	bool m_snapshotTaken;
	bool m_ignoreBlack;
	bool m_drawFlashed;
	bool m_customDraw;

	int m_windowLevel;

	bool m_showIcons;
	int m_sizeMode;

	HWND m_hiddenWindow;
	HWND m_thumbnailWindow;
	HWND m_tooltipWindow;

	LPTSTR m_windowText;

	COLORREF m_flashColor;

	int m_clickStyle;

	bool m_delphiApp;

	bool m_useCtrlDrag;

	HWND m_owner;

public:

static ATL::CWndClassInfo& GetWndClassInfo()
{ 
	DWORD style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;

	LPCTSTR WndClassName = _T( "miniMIZEThumbWin" );

	HBRUSH bkgnd = NULL;

	if( !_customShadows )
		style |= CS_DROPSHADOW;

	static ATL::CWndClassInfo wc = 
	{ 
		{ 
			sizeof(WNDCLASSEX), 
			style, 
			StartWindowProc,
			0, 0, 
			NULL, NULL, NULL, 
			(HBRUSH)(bkgnd + 1),
			NULL,
			WndClassName,
			NULL 
		}, 
		NULL, NULL,
		IDC_ARROW,
		TRUE,
		0,
		_T("") 
	}; 
	return wc; 
}

	CThumbnailBase()
	{
		m_windowBitmap = NULL;
		shadowBmp = NULL;
		m_hiddenWindow = NULL;
		m_windowText = NULL;

		m_defSize = 100;
		m_width = m_defSize;
		m_height = m_defSize;
		m_showIcons = true;
		m_sizeMode = SM_USEWIDTH;

		m_snapshotTaken = false;
		m_ignoreBlack = false;
		m_drawFlashed = false;
		m_windowLevel = WL_NORMAL;

		m_opacity = 100;

		m_flashColor = RGB( 255, 0, 0 );

		m_clickStyle = CS_DOUBLECLICK;

		m_delphiApp = false;

		m_useCtrlDrag = false;

		m_owner = NULL;

		m_xPos = 0;
		m_yPos = 0;

//		m_customShadows = false;
	}

	~CThumbnailBase()
	{
		DeleteTooltip( );

		m_hiddenWindow = NULL;

		if( m_windowBitmap != NULL )
			delete m_windowBitmap;

		if( m_windowText )
			delete [] m_windowText;
	}



//
// IUnknown
//
BEGIN_COM_MAP(T)
	COM_INTERFACE_ENTRY(IThumbWindow)
END_COM_MAP()

BEGIN_MSG_MAP(T)
		MESSAGE_HANDLER( WM_NCCREATE, OnNCCreate )
		MESSAGE_HANDLER( WM_NCHITTEST, OnNCHitTest )
		MESSAGE_HANDLER( WM_NCLBUTTONDBLCLK, OnNCLButtonDblClick )
		MESSAGE_HANDLER( WM_LBUTTONUP, OnLButtonUp )
		MESSAGE_HANDLER( WM_LBUTTONDBLCLK, OnNCLButtonDblClick )
		MESSAGE_HANDLER( WM_NCRBUTTONUP, OnNCRButtonUp )
		MESSAGE_HANDLER( WM_PAINT, OnPaint )
		MESSAGE_HANDLER( WM_TIMER, OnTimer )
		MESSAGE_HANDLER( WM_MOVE, OnWindowMove )
END_MSG_MAP()

public:
LRESULT OnNCCreate( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled )
{
    AddRef( );
	return 1;
}

LRESULT OnNCHitTest( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled )
{
	if( ( m_clickStyle == CS_DOUBLECLICK ) && ( m_useCtrlDrag == false ) )
	{
	    return HTCAPTION;
	}
	else
	{
		// single click
		// check if ctrl is down
		SHORT nVirtKey = GetKeyState( VK_CONTROL ); 
				
		if (nVirtKey & KEY_DOWN) 
        { 
			// yes, so return that we want to be moving
			return HTCAPTION;
		}
		else
		{
			bHandled = false;
		}
	}

	return HTCLIENT;
}

LRESULT OnNCLButtonDblClick( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled )
{
	LRESULT retval = 1;
   
	if( m_clickStyle == CS_DOUBLECLICK )
	{
		if( m_hiddenWindow )
		{
			::PostMessage( m_hiddenWindow, WM_SYSCOMMAND, (WPARAM) SC_RESTORE, 0 );

			SetForegroundWindow( m_hiddenWindow );
		}
	}

    return retval;
}
LRESULT OnLButtonUp( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled )
{
	LRESULT retval = 1;
	
	if( m_clickStyle == CS_SINGLECLICK )
	{
		if( m_hiddenWindow )
		{
			::PostMessage( m_hiddenWindow, WM_SYSCOMMAND, (WPARAM) SC_RESTORE, 0 );

			SetForegroundWindow( m_hiddenWindow );
		}

		retval = 0;
	}

	return retval;
}
LRESULT OnNCRButtonUp( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled )
{
	LRESULT retval = 1;
	POINT pt = {0};

	::GetCursorPos(&pt);
	
	SHORT nVirtKey = GetKeyState( VK_CONTROL ); 
				
	if (nVirtKey & KEY_DOWN) 
    { 
		ShowContextMenu( pt.x, pt.y );
	}
	else
	{
		ShowSystemMenu( pt.x, pt.y );
	}

	return retval;
}
LRESULT OnPaint( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled )
{
	if( !_customShadows )
	{
		PAINTSTRUCT ps; 
		HDC hdc;

		hdc = BeginPaint( &ps ); 

		SetBkMode( hdc, TRANSPARENT );
		
		// draw our bitmap here
		Gdiplus::Graphics g( hdc );

		g.Clear( Gdiplus::Color(0, 0, 0, 255) );

		DrawShadow( &g, 5, 5, m_width, m_height, 33, 0.5, 255 );

		g.DrawImage( m_windowBitmap, 0, 0, m_width, m_height );
		
		if( m_drawFlashed )
		{
			Gdiplus::Color col;
			
			col.SetFromCOLORREF( m_flashColor );
			// Create a Pen object.
			Gdiplus::Pen blackPen( col, 1 );
			// Create a Rect object.
			Gdiplus::Rect rect(0, 0, m_width-1, m_height-1);
			// Draw rect.
			g.DrawRectangle(&blackPen, rect);
		}

		EndPaint( &ps );
	}
	
	bHandled = (_customShadows) ? FALSE : TRUE;

	return 0;
}

LRESULT OnTimer( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled )
{
	SAFEDEBUGWRITELINE( _debugWindow, _T( "Flash Timer" ) )

	if( wParam == FLASHTIMER )
	{
		m_drawFlashed = false;
		
		// turn off the flashing
		if( _customShadows )
		{
			UpdateThumbWindow( );
		}
		else
		{
			::KillTimer( m_hWnd, FLASHTIMER );

			InvalidateRect( NULL );
		}
	}

    return 0;
}


LRESULT OnWindowMove( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled )
{
	// this is for general positioning
	// we have been moved, so store our new co-ordinates
	int xPos = 0;
	int yPos = 0;

	xPos = (int)(short) LOWORD(lParam);   // horizontal position 
	yPos = (int)(short) HIWORD(lParam);   // vertical position 

	// work out our offset

	m_xPos = xPos + ( m_width / 2 );
	m_yPos = yPos + ( m_height / 2 );

	if( _useSnapto )
	{
		// only snapto if we're not holding down alt
		SHORT nVirtKey = GetKeyState( VK_MENU ); 

        if( !(nVirtKey & KEY_DOWN) )
        { 
			// lets do some snapto
			// is it in our exclusion list?
			FindThumbnailSnaptoPredicate findPredicate( GetThumbnailRect( ) );

			vector< CComPtr< IThumbWindow > >::iterator it;

			it = find_if( _deskconWindows.begin(), _deskconWindows.end(), findPredicate );

			if( it != _deskconWindows.end( ) )
			{
				// yup0r, so snap to!!
				SnapTo( (*it)->GetThumbnailRect( ) );
			}
		}
		
	}

    return 1;
}


virtual void OnFinalMessage( HWND hWnd )
{
	Release( );
}


public:
//
// IThumbWindow
//
//STDMETHODIMP_( BOOL )Initialize( HWND hWndView, int x, int y, bool ignoreBlack )
//{
//	return Initialize( hWndView, x, y, ignoreBlack, true );
//}
STDMETHODIMP_( BOOL )Initialize( HWND hWndView,
								 int x,
								 int y,
								 bool ignoreBlack)
{
	m_ignoreBlack = ignoreBlack;

	// work out our rect
	RECT r;

	m_hiddenWindow = hWndView;

	m_thumbnailWindow = ValidateWindowHandle(m_hiddenWindow);

	// store the window text
	if( m_windowText == NULL )
	{
		m_windowText = new TCHAR[100];

		::GetWindowText( m_hiddenWindow, m_windowText, 100 );
	}

	// are we showing icons?
	if( m_showIcons )
	{
		iconBmp = GetWindowIcon(hWndView, m_thumbnailWindow, m_delphiApp);
	}

	::GetWindowRect(m_thumbnailWindow, &r);

	CalculateThumbnailSize(&r, m_sizeMode, &m_width, &m_height);

	//TakeWindowSnapshot( m_hiddenWindow );

	// setup the rect
	r.top = y; 
	r.left = x;
	r.right = r.left + m_width;
	r.bottom = r.top + m_height;

	if( _customShadows )
	{
		r.right += SHADOW_SIZE;
		r.bottom += SHADOW_SIZE;
	}

	// create the actual window
	Create( NULL , r, _T("ThumbWindow"), WS_POPUP, WS_EX_TOOLWINDOW | WS_EX_LAYERED );

	// set the window level
	SetWindowLevel( m_windowLevel );

	// and finally, create the tooltip
	CreateMyTooltip( );

	// derived classes should prepare their thumbnails in here
	OnInitialized( );

	// if we're drawing custom shadows, then we need to do an update
	if( _customShadows )
	{
		UpdateThumbWindow( );
	}
	else
	{
		SetOpacity( m_opacity );
	}
	
	return TRUE;
}

STDMETHODIMP_( BOOL )WindowMaximized()
{
	OnWindowMaximized();

	return TRUE;
}

STDMETHODIMP_( BOOL )WindowMinimized()
{
	// the window we monitor was minimized, so we should do something with our thumbnail
	// depending on the type of thumbnail we are, some might do nothing
	RECT rect;

	m_thumbnailWindow = ValidateWindowHandle(m_hiddenWindow);

	::GetWindowRect(m_thumbnailWindow, &rect);

	// might have changed the size of the thumbnail
	CalculateThumbnailSize(&rect, m_sizeMode, &m_width, &m_height);

	//TakeWindowSnapshot( m_hiddenWindow );

	// setup the rect
	RECT r = GetThumbnailRect();

	/*r.right = r.left + m_width;
	r.bottom = r.top + m_height;*/

	if( _customShadows )
	{ 
		m_width += SHADOW_SIZE;
		m_height += SHADOW_SIZE;
	}
	
	SetWindowPos( HWND_BOTTOM, 0, 0, m_width, m_height, SWP_NOMOVE | SWP_NOACTIVATE );
	
	OnWindowMinimized( );

	if( _customShadows )
	{
		UpdateThumbWindow( );
	}

	return TRUE;
}

STDMETHODIMP_( BOOL )Show( void )
{
	BOOL retval = FALSE;

	if( m_snapshotTaken )
	{
		retval = ShowWindow( SW_SHOW );

		// lets just set our window level again for good measure
		SetWindowLevel( m_windowLevel );

		retval = TRUE;
	}

	return retval;
}

STDMETHODIMP_( BOOL )IsVisible( void )
{
	if( m_hWnd )
	{
		return IsWindowVisible( );
	}
	
	return FALSE;
}

STDMETHODIMP_( void )Hide( void )
{
	ShowWindow( SW_HIDE );
}
STDMETHODIMP_( HWND )GetMonitoredWindow ( void )
{
	return m_hiddenWindow;
}

STDMETHODIMP_( void )Close ( void )
{
	PostMessage( WM_CLOSE, 0, 0 );
}

STDMETHODIMP_( void )GetLocation( int* x, int* y )
{
	if( ( x == NULL ) || ( y == NULL ) )
		return;
    
	(*x) = m_xPos;
	(*y) = m_yPos;
}

STDMETHODIMP_( void )SetLocation( int x, int y )
{
	int newx = x - ( m_width / 2 );
	int newy = y - ( m_height / 2 );

	SetWindowPos( NULL, newx, newy, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
}

STDMETHODIMP_( HWND )GetHWND( )
{
	return m_hWnd;
}


STDMETHODIMP_( void )Flash( void )
{
	// this function means we must flash on. I guess we need a timer to flash off?
	m_drawFlashed = true;

	if( _customShadows )
	{
		UpdateThumbWindow( );
	}
	else
	{
		::SetTimer( m_hWnd,  FLASHTIMER, 300, NULL );

		InvalidateRect( NULL );
	}
}

STDMETHODIMP_( void )ShowTaskbarButton( bool show )
{
	if( show )
	{
		_taskbar->AddTab( m_hiddenWindow );
	}
	else
	{
		/*HWND window = m_hiddenWindow;

		HWND wnd = ::GetWindow( window, GW_ENABLEDPOPUP );

		if( ( wnd ) && ( m_delphiApp ) )
		{
			window = wnd;
		}*/
		_taskbar->DeleteTab( m_hiddenWindow );
	}
}

STDMETHODIMP_( RECT )GetThumbnailRect( )
{
	RECT r = {0};

	GetWindowRect( &r );

	return r;
}

//
// IThumbnailSettings
//
STDMETHODIMP_( bool )IgnoreBlack( void )
{
	return m_ignoreBlack;
}
STDMETHODIMP_( void )SetSize( int size )
{
	m_defSize = size;
}
STDMETHODIMP_( void )SetOpacity( int percent )
{
	m_opacity = FindNearestTen( percent );

	if( IsWindow( ) )
	{
		if( _customShadows )
		{
			//UpdateThumbWindow( );
		}
		else
		{
			//SetLayeredWindowAttributes ( m_hWnd, NULL, m_opacity * 2.55, LWA_ALPHA );
		}
	}
}
STDMETHODIMP_( void )SetShowIcons( bool show )
{
	m_showIcons = show;
}
STDMETHODIMP_( void )SetSizeMode( int sizemode )
{
	m_sizeMode = sizemode;
}

STDMETHODIMP_( void )SetWindowLevel( int level )
{
	if( IsWindow( ) )
	{
		switch( level )
		{
			case WL_ALWAYSONTOP:
				{
					if( m_windowLevel == WL_ALWAYSONBOTTOM )
					{
						SetAlwaysOnBottom( false );
					}

					SetAlwaysOnTop( true );
				}
				break;
			case WL_NORMAL:
				{
					if( m_windowLevel == WL_ALWAYSONTOP )
					{
						SetAlwaysOnTop( false );
					}
					else if( m_windowLevel == WL_ALWAYSONBOTTOM )
					{
						SetAlwaysOnBottom( false );
					}
				}
				break;
			case WL_ALWAYSONBOTTOM:
				{
					if( m_windowLevel == WL_ALWAYSONTOP )
					{
						SetAlwaysOnTop( false );
					}
					else if( m_windowLevel == WL_ALWAYSONBOTTOM )
					{
						SetAlwaysOnBottom( false );
					}

					SetAlwaysOnBottom( true );
				}
				break;
			default:
				return;
		}
	}

	m_windowLevel = level;
}

STDMETHODIMP_( void )SetFlashColor( COLORREF color )
{
	m_flashColor = color;
}
STDMETHODIMP_( void )SetDelphiApp( bool delphiApp )
{
	m_delphiApp = delphiApp;
}
STDMETHODIMP_( void )SetClickStyle( int clickstyle )
{
	m_clickStyle = clickstyle;
}
STDMETHODIMP_( void )SetUseCtrlDrag( bool ctrldrag )
{
	m_useCtrlDrag = ctrldrag;
}

STDMETHODIMP_( void )SetDrawCustomShadows( bool custom )
{
	_customShadows = custom;
}

protected:
	// overrides
virtual bool OnWindowMinimized()
{
	return false;
}

virtual bool OnWindowMaximized()
{
	return false;
}

virtual bool OnInitialized()
{
	return false;
}

private:
void SnapTo( RECT r )
{
	RECT myRect = GetThumbnailRect( );

	// ok, we want to snap to the given rect, so figure out which way
	int x = 0;
	//int left = 0;
	int y = 0;
	//int bottom = 0;
	
	if( abs( myRect.left - r.left ) <= 10 )
	{
		x = r.left;
	}

	/*if( abs( myRect.right - r.right ) <= 10 )
	{
		right = r.right;
	}*/

	if( abs( myRect.top - r.top ) <= 10 )
	{
		y = r.top;
	}

	/*if( abs( myRect.bottom - r.bottom ) <= 10 )
	{
		bottom = r.bottom;
	}*/

	if( abs( myRect.right - r.left ) <= 10 )
	{
		x = r.left - ( myRect.right - myRect.left );
		y = ( y == 0 ) ? myRect.top : y; 
		//SetWindowPos( NULL, r.left - ( myRect.right - myRect.left ), myRect.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
	}
	//else if( ( myRect.left - 10 ) >= r.right )
	if( abs( myRect.left - r.right ) <= 10 )
	{
		x = r.right;
		y = ( y == 0 ) ? myRect.top : y;
		//SetWindowPos( NULL, r.right, myRect.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER );	
	}
	//else if( ( myRect.top - 10 ) >= r.bottom )
	if( abs( myRect.top - r.bottom ) <= 10 )
	{
		x = x = ( x == 0 ) ? myRect.left : x;
		y = r.bottom;
		//SetWindowPos( NULL, myRect.left, r.bottom, 0, 0, SWP_NOSIZE | SWP_NOZORDER );	
	}
	//else if( ( myRect.bottom + 10 ) >= r.top )
	if( abs( myRect.bottom - r.top ) <= 10 )
	{
		x = ( x == 0 ) ? myRect.left : x;
		y =  - ( myRect.bottom - myRect.top );
		//SetWindowPos( NULL, myRect.left, r.top - ( myRect.bottom - myRect.top ), 0, 0, SWP_NOSIZE | SWP_NOZORDER );	
	}
}
void SetAlwaysOnBottom( bool set )
{
	if( set )
	{
		HWND desktopWnd = FindWindow( _T("Progman"), NULL );

		SetParent( desktopWnd );

		SetWindowLong( GWL_STYLE, ( GetWindowLong( GWL_STYLE ) &~ WS_CHILD ) );
	}
	else
	{
		SetParent( NULL );
	}
}
void SetAlwaysOnTop( bool set )
{
	HWND pos = HWND_NOTOPMOST;


	if( set )
	{
		pos = HWND_TOPMOST;
	}
	
	SetWindowPos( pos, &CWindow::rcDefault, SWP_NOSIZE | SWP_NOMOVE );
}

void ShowContextMenu( int x, int y )
{
	UINT flags;
	int val;
	int opacity = 0;
	TCHAR buf[5];
	HMENU hContextMenu = ::CreatePopupMenu( );

	if( hContextMenu )
	{
		HMENU hOpacityMenu = ::CreatePopupMenu( );

		flags = MF_BYPOSITION;

		if( hOpacityMenu )
		{
			for( int i = 10; i > 0; i-- )
			{
				opacity = i * 10;

				if( m_opacity == opacity )
				{
					flags = flags | MF_CHECKED;
				}
				else
				{
					flags = flags &~ MF_CHECKED;
				}

				_stprintf( buf, _T( "%d%%\0" ), opacity );

				::InsertMenu( hOpacityMenu, -1, flags, opacity, buf );
			}

			::InsertMenu( hContextMenu, -1, MF_BYPOSITION | MF_POPUP ,(UINT_PTR) hOpacityMenu, _T( "Opacity" ) );

		}

		HMENU hLevelMenu = ::CreatePopupMenu( );

		if( hLevelMenu )
		{
			::InsertMenu( hLevelMenu, -1, (m_windowLevel == WL_ALWAYSONBOTTOM) ? ( MF_BYPOSITION | MF_CHECKED ) : ( MF_BYPOSITION ), WL_ALWAYSONBOTTOM, _T( "Pin to Desktop" ) );
			::InsertMenu( hLevelMenu, -1, (m_windowLevel == WL_NORMAL) ? ( MF_BYPOSITION | MF_CHECKED ) : ( MF_BYPOSITION ), WL_NORMAL, _T( "Normal" ) );
			::InsertMenu( hLevelMenu, -1, (m_windowLevel == WL_ALWAYSONTOP) ? ( MF_BYPOSITION | MF_CHECKED ) : ( MF_BYPOSITION ), WL_ALWAYSONTOP, _T( "Pin on Top" ) );

			::InsertMenu( hContextMenu, -1, MF_BYPOSITION | MF_POPUP ,(UINT_PTR) hLevelMenu, _T( "Thumbnail Level" ) );

		}

		::SetForegroundWindow( m_hWnd );

		val = ::TrackPopupMenu( hContextMenu, TPM_RETURNCMD | TPM_LEFTBUTTON, x, y, 0, m_hWnd, NULL );

		if( val > 0 )
		{
			if( val > 3 )
			{
				SetOpacity( val );
			}
			else
			{
				// set the window level
				SetWindowLevel( val );
			}
		}

		if( hOpacityMenu )
			::DestroyMenu( hOpacityMenu );

		if( hLevelMenu )
			::DestroyMenu( hLevelMenu );

		::DestroyMenu( hContextMenu );
	}
}




void ShowSystemMenu( int x, int y )
{
	HMENU sysmenu = ::GetSystemMenu( m_hiddenWindow, FALSE );

	if( sysmenu )
	{
		if( ::IsIconic( m_hiddenWindow ) )
		{
			::EnableMenuItem( sysmenu, SC_RESTORE, MF_BYCOMMAND | MF_ENABLED );
			::EnableMenuItem( sysmenu, SC_MOVE, MF_BYCOMMAND | MF_GRAYED );
			::EnableMenuItem( sysmenu, SC_SIZE, MF_BYCOMMAND | MF_GRAYED );
			::EnableMenuItem( sysmenu, SC_MINIMIZE, MF_BYCOMMAND | MF_GRAYED );
		}
		else if( ::IsZoomed( m_hiddenWindow ) )
		{
			::EnableMenuItem( sysmenu, SC_RESTORE, MF_BYCOMMAND | MF_ENABLED );
			::EnableMenuItem( sysmenu, SC_MOVE, MF_BYCOMMAND | MF_GRAYED );
			::EnableMenuItem( sysmenu, SC_SIZE, MF_BYCOMMAND | MF_GRAYED );
			::EnableMenuItem( sysmenu, SC_MAXIMIZE, MF_BYCOMMAND | MF_GRAYED );
		}
		else
		{
			::EnableMenuItem( sysmenu, SC_RESTORE, MF_BYCOMMAND | MF_GRAYED );
			::EnableMenuItem( sysmenu, SC_MOVE, MF_BYCOMMAND | MF_ENABLED );
			::EnableMenuItem( sysmenu, SC_SIZE, MF_BYCOMMAND | MF_ENABLED );
			::EnableMenuItem( sysmenu, SC_MINIMIZE, MF_BYCOMMAND | MF_ENABLED );
		}
		
		DWORD selection = ::TrackPopupMenuEx( sysmenu, TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_RETURNCMD, x, y, m_hWnd, NULL );

		if( selection > 0 )
		{
			::SendMessage( m_hiddenWindow, WM_SYSCOMMAND, (WPARAM) selection, NULL );
		}
	}
}

Gdiplus::Bitmap* GetWindowIcon(HWND mainHwnd, HWND activeHwnd, bool isDelphi)
{
	HICON icon = NULL;
	Gdiplus::Bitmap* icoBmp = NULL;
	HWND hwnd = mainHwnd;

	if( activeHwnd && isDelphi )
	{
		hwnd = activeHwnd;
	}

	icon = (HICON)::SendMessage( hwnd, WM_GETICON, ICON_BIG, NULL );

	if( icon == NULL )
	{
		icon = (HICON)::GetClassLongPtr( hwnd, GCLP_HICON );
	}

	if( icon == NULL )
	{
		// get process id from window handle
	//
	//DWORD dwProcess = 0;

	//::GetWindowThreadProcessId( hwnd, &dwProcess );

	//if ( dwProcess != NULL )
	//{
	//	// get process handle from id
	//	HANDLE hProcess = ::OpenProcess(  PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwProcess );
	//	
	//	if ( hProcess != NULL )
	//	{
	//		// get module handle from process handle
	//		//
	//		DWORD count = 0;
	//		HMODULE hm[1] = { 0 };
	//		EnumProcessModules( (HANDLE)hProcess, hm, sizeof(hm), &count );

	//		// get file name from module handle
	//		GetModuleBaseName( hProcess, hm[0], name, length );

	//		// close process handle
	//		::CloseHandle( hProcess );

	//		bRet = true;
	//	}
	//}		
	}

	if( icon != NULL )
	{
		icoBmp = AlphaBitmapFromIcon( icon );
	}
	else
	{
		SAFEDEBUGWRITELINE( _debugWindow, _T( "No Icon" ) )
	}
		
	return icoBmp;
}

Gdiplus::Bitmap* AlphaBitmapFromIcon(HICON hIcon)
{
	ICONINFO ii;

	if( !GetIconInfo(hIcon, &ii) )
	return NULL;

	BITMAP bm;
	GetObject(ii.hbmColor, sizeof(bm), &bm);

	if( bm.bmBitsPixel != 32 )
	{
	DeleteObject(ii.hbmColor);
	DeleteObject(ii.hbmMask);
	return Gdiplus::Bitmap::FromHICON(hIcon);
	}

	BYTE* c;
	BYTE* m;
	BITMAPINFO bmi = { sizeof(bmi.bmiHeader), bm.bmWidth, -bm.bmHeight, bm.bmPlanes, bm.bmBitsPixel, BI_RGB, 0, 0, 0, 0, 0 };
	HBITMAP clr = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (LPVOID*)&c, NULL, 0);
	HBITMAP msk = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (LPVOID*)&m, NULL, 0);

	HDC hCD = CreateCompatibleDC(NULL);
	HDC hMD = CreateCompatibleDC(NULL);
	HDC hCC = CreateCompatibleDC(NULL);
	HDC hMC = CreateCompatibleDC(NULL);
	HGDIOBJ hCDBmp = SelectObject(hCD, clr);
	HGDIOBJ hMDBmp = SelectObject(hMD, msk);
	HGDIOBJ hCCBmp = SelectObject(hCC, ii.hbmColor);
	HGDIOBJ hMCBmp = SelectObject(hMC, ii.hbmMask);

	BitBlt(hCD, 0, 0, bm.bmWidth, bm.bmHeight, hCC, 0, 0, SRCCOPY);
	BitBlt(hMD, 0, 0, bm.bmWidth, bm.bmHeight, hMC, 0, 0, SRCCOPY);

	SelectObject(hCD, hCDBmp);
	SelectObject(hMD, hMDBmp);
	SelectObject(hCC, hCCBmp);
	SelectObject(hMC, hMCBmp);
	DeleteDC(hCD);
	DeleteDC(hMD);
	DeleteDC(hCC);
	DeleteDC(hMC);

	int p;
	int n = bm.bmWidth*bm.bmHeight;
	BYTE* t = (BYTE*)c;
	bool a = false;

	// determine if there's any alpha in the color bitmap; if not, then this is
	// a 32bpp icon with a mask, so build the alpha channel from it instead

	for( p = 0; p < n; p++ )
	{
		if( t[3] ) 
		{ 
			a = true;
			break; 
		}

		t += 4;
	}

	Gdiplus::BitmapData bmd;
	Gdiplus::Rect rect(0, 0, bm.bmWidth, bm.bmHeight);
	Gdiplus::Bitmap* bitmap = new Gdiplus::Bitmap(bm.bmWidth, bm.bmHeight, PixelFormat32bppARGB);

	bitmap->LockBits(&rect, Gdiplus::ImageLockModeWrite, bitmap->GetPixelFormat(), &bmd);

	BYTE* b = (BYTE*)bmd.Scan0;

	for( p = 0; p < n; p++ )
	{
		b[0] = c[0]; // blue
		b[1] = c[1]; // green
		b[2] = c[2]; // red
		b[3] = a ? c[3] : (m[0] == 0 ? 255 : 0); // use alpha or apply mask
		c += 4;
		m += 4;
		b += 4;
	}

	bitmap->UnlockBits(&bmd);

	DeleteObject(ii.hbmColor);
	DeleteObject(ii.hbmMask);
	DeleteObject(clr);
	DeleteObject(msk);

	return bitmap;
} 
void CreateMyTooltip( void )
{
	// struct specifying info about tool in ToolTip control
	TOOLINFO ti;
	unsigned int uid = 0;       // for ti initialization
	RECT rect;                  // for client area coordinates

	/* CREATE A TOOLTIP WINDOW */
	m_tooltipWindow = CreateWindowEx(WS_EX_TOPMOST,
		TOOLTIPS_CLASS,
		NULL,
		WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,		
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		m_hWnd,
		NULL,
		_hInstance,
		NULL
		);

	::SetWindowPos( m_tooltipWindow, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE );

	/* GET COORDINATES OF THE MAIN CLIENT AREA */
	GetClientRect( &rect );

	/* INITIALIZE MEMBERS OF THE TOOLINFO STRUCTURE */
	ti.cbSize = sizeof(TOOLINFO);
	ti.uFlags = TTF_SUBCLASS;
	ti.hwnd = m_hWnd;
	ti.hinst = _hInstance;
	ti.uId = uid;
	ti.lpszText = m_windowText;
		// ToolTip control will cover the whole window
	ti.rect.left = rect.left;    
	ti.rect.top = rect.top;
	ti.rect.right = rect.right;
	ti.rect.bottom = rect.bottom;

	/* SEND AN ADDTOOL MESSAGE TO THE TOOLTIP CONTROL WINDOW */
	::SendMessage(m_tooltipWindow, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);	
}

void DeleteTooltip( void )
{
	// struct specifying info about tool in ToolTip control
	TOOLINFO ti;
	unsigned int uid = 0;       // for ti initialization

	/* INITIALIZE MEMBERS OF THE TOOLINFO STRUCTURE */
	ti.cbSize = sizeof(TOOLINFO);
	ti.hwnd = m_hWnd;
	ti.hinst = _hInstance;
	ti.uId = uid;

	/* SEND AN ADDTOOL MESSAGE TO THE TOOLTIP CONTROL WINDOW */
	::SendMessage(m_tooltipWindow, TTM_DELTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);	
}



void UpdateThumbWindow( )
{
	
	HDC memDC = ::CreateCompatibleDC( NULL );

	POINT screenPos = {0};

	RECT r = {0};

	GetWindowRect( &r );

	screenPos.x = r.left;
	screenPos.y = r.top;

	SIZE windowSize = {0};

	windowSize.cx = (r.right - r.left);
	windowSize.cy = (r.bottom - r.top);

	BLENDFUNCTION blendPixelFunction = {0};

	blendPixelFunction.BlendOp = AC_SRC_OVER;
	blendPixelFunction.BlendFlags = 0;
	blendPixelFunction.SourceConstantAlpha = m_opacity * 2.55;
	blendPixelFunction.AlphaFormat = AC_SRC_ALPHA;

	POINT srcPos = {0};

	srcPos.x = 0;
	srcPos.y = 0;

	Gdiplus::Bitmap memBmp( m_width + SHADOW_SIZE, m_height + SHADOW_SIZE );

	Gdiplus::Graphics g( &memBmp );

	DrawShadow( &g, 2, 2, m_width, m_height, 0.1, 0.8, RGB( 0, 0, 0 ) );//RGB( 0, 24, 48 ) );

	g.DrawImage( m_windowBitmap, 0, 0, m_width, m_height );

	if( m_drawFlashed )
	{
		Gdiplus::Color col;
		
		col.SetFromCOLORREF( m_flashColor );
		// Create a Pen object.
		Gdiplus::Pen flashPen( col, 1 );
		// Create a Rect object.
		Gdiplus::Rect rect(0, 0, m_width-1, m_height-1);
		// Draw rect.
		g.DrawRectangle(&flashPen, rect);

		::SetTimer( m_hWnd,  FLASHTIMER, 300, (TIMERPROC) NULL );
	}
	else
	{
		::KillTimer( m_hWnd,  FLASHTIMER );
	}

	//WriteCaption( &g, 0, m_height + 5 );

	HBITMAP bmp;

	memBmp.GetHBITMAP( Color( 0 ), &bmp );

	HBITMAP oldBmp = (HBITMAP)::SelectObject( memDC, bmp );

	UpdateLayeredWindow( m_hWnd, NULL, &screenPos, &windowSize, memDC, &srcPos, NULL, &blendPixelFunction, ULW_ALPHA );

	::SelectObject( memDC, oldBmp );
	DeleteObject( bmp );
	DeleteObject( oldBmp );
	::ReleaseDC( NULL, memDC );
	::DeleteDC( memDC );
}



//
// Util Functions
//

void CalculateThumbnailSize(LPRECT rect, int sizeMode, int* width, int* height)
{
	if( sizeMode == SM_SMARTSIZE )
	{
		if( ( rect->bottom - rect->top ) > ( rect->right - rect->left ) )
		{
			sizeMode = SM_USEHEIGHT;
		}
		else
		{
			sizeMode = SM_USEWIDTH;
		}
	}

	if( sizeMode == SM_USEWIDTH )
	{
		(*width) = m_defSize;
		(*height) = (int) ( (*width) * ( rect->bottom - rect->top ) ) / ( ( rect->right - rect->left ) + 1 );
	}
	else
	{
		(*height) = m_defSize;
		(*width) = (int) ( (*height) * ( rect->right - rect->left ) ) / ( ( rect->bottom - rect->top ) + 1 );
	}
}


HWND ValidateWindowHandle(HWND hwndOriginal)
{
	HWND wnd = ::GetWindow( hwndOriginal, GW_ENABLEDPOPUP );

	if( ( wnd ) && ( m_delphiApp ) )
	{
		return wnd;
	}

	return hwndOriginal;
}

// 
// The caption under the thumbnail
//
VOID WriteCaption( Gdiplus::Graphics* g, int left, int top )
{
   // Create a string.
//   WCHAR string[] = L"Sample Text";
//   
//   // Initialize arguments.
//   Font myFont(L"Arial", 10);
//   RectF layoutRect(left, top, m_width, m_height + SHADOW_SIZE);
//   StringFormat format;
//   format.SetAlignment(StringAlignmentCenter);
//  
//   SolidBrush blackBrush(Color(255, 255, 255, 255));
//
//   Gdiplus::Bitmap shadowBmp( m_width / 4, ( m_height + SHADOW_SIZE )/ 4 );
//
//   Gdiplus::Graphics shadowG( &shadowBmp );
//
//   shadowG.SetTextRenderingHint( Gdiplus::TextRenderingHintAntiAlias );
//
//   Gdiplus::Matrix mx( 0.25f, 0, 0, 0.25f, 3, 3 );
//
//   shadowG.SetTransform( &mx );
//
////   Gdiplus::StringFormat shadowFormat( (const Gdiplus::StringFormat*) Gdiplus::StringFormat::GenericTypographic );
//	RectF shadowRect = layoutRect;
//	shadowRect.X += 2;
//	shadowRect.Y += 2;
//
//	SolidBrush shadowBrush( Color( 128, 0, 0, 0 ) );
//
//	shadowG.DrawString( string, 11, &myFont, shadowRect, &format, &shadowBrush);
//
//
//	g->SetTextRenderingHint( Gdiplus::TextRenderingHintAntiAlias );
//
//	g->DrawImage( &shadowBmp, shadowRect, 0, 0, shadowBmp.GetWidth( ), shadowBmp.GetHeight( ), UnitPixel,0,0,0);
//
//   // Draw string.
//   g->DrawString( string, 11, &myFont, layoutRect, &format, &blackBrush);
}

//
// the drop shadow
//
void DrawShadow( Gdiplus::Graphics* g, int left, int top,int width, int height, REAL grayshade, REAL alpha, LONG color )
{
	if(shadowBmp)
	{
		delete shadowBmp;
		shadowBmp = NULL;
	}

	if(!shadowBmp)
	{
		Gdiplus::RectF imgRect(0, 0, m_windowBitmap->GetWidth( ), m_windowBitmap->GetHeight( ) );

		Gdiplus::Bitmap tmp(width+20,height+20, PixelFormat32bppARGB);
		Gdiplus::Graphics tmpG(&tmp);
		tmpG.DrawImage(m_windowBitmap,Gdiplus::Rect(10,10,width,height),imgRect.X, imgRect.Y, imgRect.Width, imgRect.Height,UnitPixel,0,0,0);
		shadowBmp = GetBlur(&tmp,2.0f);
	}

	Gdiplus::ImageAttributes ia;

	Gdiplus::ColorMatrix cm = GetIdentityMatrix();

	cm.m[4][0] = static_cast<float>GetRValue(color)/ 255;
	cm.m[4][1] = static_cast<float>GetGValue(color)/ 255;
	cm.m[4][2] = static_cast<float>GetBValue(color)/ 255;
	cm.m[0][0] = cm.m[1][1] = cm.m[2][2] = grayshade;
	cm.m[3][3] = alpha * ((float)this->m_opacity/100.0f);
	
	ia.SetColorMatrix(&cm);

	g->DrawImage(shadowBmp, Rect( left - 4,top-4,width+13,height+13 ),0,0,shadowBmp->GetWidth(),shadowBmp->GetHeight() ,UnitPixel,&ia,0,0);
}

Gdiplus::ColorMatrix GetIdentityMatrix( )
{
	Gdiplus::ColorMatrix cm = {	1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
								0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
								0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
								0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
								0.0f, 0.0f, 0.0f, 0.0f, 1.0f};

	return cm;
}

};

