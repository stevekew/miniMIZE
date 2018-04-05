//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// ExclusionsSettingDialog.cpp
//
// Copyright (C) Stephen Kew. All Rights Reserved.
//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "stdafx.h"
#include "resource.h"       // main symbols
#include "deskcon.h"
#include "deskconinternal.h"
#include "settings.h"
#include <commdlg.h>
#include <vector>

using namespace std;

extern HWND _mainHwnd;
extern UINT UWM_SETTINGCHANGED;
extern HINSTANCE _hInstance;
extern vector< PEXCLUSIONITEM > _exclusionList;

#define BULLSEYE_CENTER_X_OFFSET 15//-1		//15
#define BULLSEYE_CENTER_Y_OFFSET 18//3		//18

class CExclusionsSettingsDlg : public CDialogImpl<CExclusionsSettingsDlg>,
							public CComObjectRootEx<CComSingleThreadModel>,
							public ITabPage
{

private:
	HCURSOR m_oldCursor;
	bool m_trackingWindows;
	HGDIOBJ m_pen;
	HWND m_currentWindow;
	HIMAGELIST m_imageList;
	int m_processImage;
	int m_windowImage;
	int m_classImage;

public:
	CExclusionsSettingsDlg( )
	{
		m_currentWindow = NULL;
		m_oldCursor = NULL;
		m_imageList = NULL;
		m_trackingWindows = false;
		m_processImage = -1;
		m_windowImage = -1;

		m_pen = CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
	}

	~CExclusionsSettingsDlg( )
	{
		if( m_pen )
			DeleteObject( m_pen );
	}

//
// IUnknown
//
BEGIN_COM_MAP(CExclusionsSettingsDlg)
	COM_INTERFACE_ENTRY(ITabPage)
END_COM_MAP()

enum { IDD = IDD_EXCLUSIONSSETTINGS };

BEGIN_MSG_MAP(CExclusionsSettingsDlg)
	MESSAGE_HANDLER( WM_INITDIALOG, OnInitDialog )
	MESSAGE_HANDLER( WM_LBUTTONDOWN, OnLButtonDown )
	MESSAGE_HANDLER( WM_LBUTTONUP, OnLButtonUp )
	MESSAGE_HANDLER( WM_MOUSEMOVE, OnMouseMove )
	COMMAND_HANDLER( IDC_ADDEXCLUSION, BN_CLICKED, OnAddExclusionClick )
	COMMAND_HANDLER( IDC_REMOVEEXCLUSION, BN_CLICKED, OnRemoveExclusionClick )
	COMMAND_HANDLER( IDC_UPDATEEXCLUSION, BN_CLICKED, OnUpdateExclusionClick )
	COMMAND_HANDLER( IDC_BROWSEPROC, BN_CLICKED, OnBrowseClick )
	NOTIFY_HANDLER( IDC_EXCLUSIONLIST, TVN_SELCHANGED, OnSelectionChanged )
	COMMAND_HANDLER( IDC_USEREGEX, BN_CLICKED, OnUseRegexClicked )
	MESSAGE_HANDLER( WM_CLOSE, OnClose )
END_MSG_MAP()

LRESULT OnInitDialog( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	AddRef( );

	LPTSTR strVal = NULL;
	HWND hControl = NULL;

	GetStringSetting( _T( "Advanced Excludes" ), &strVal, _T( "False" ) );

	if( strVal )
	{
		if( _tcsicmp( strVal, _T( "true" ) ) == 0 )
		{
			hControl = GetDlgItem( IDC_USEREGEX );
			
			::SendMessage( hControl, BM_SETCHECK, (WPARAM)BST_CHECKED, (LPARAM)NULL );
		}

		delete [] strVal;
		strVal = NULL;
	}

	hControl = GetDlgItem( IDC_FINDERICON );

	HICON icon = LoadIcon( _hInstance, MAKEINTRESOURCE( IDI_FINDER1 ) );

	::SendMessage( hControl, STM_SETICON, (WPARAM)icon, NULL );

	m_imageList = SetupImageList( _hInstance );

	hControl = GetDlgItem( IDC_EXCLUSIONLIST );

	TreeView_SetImageList( hControl, m_imageList, TVSIL_NORMAL );

	LoadExclusions( );

	return 1;  // Let the system set the focus
}

LRESULT OnAddExclusionClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	TCHAR process[100];
	TCHAR classname[100];
	TCHAR window[255];

	HWND hControl = GetDlgItem( IDC_PROCESSNAME );
	::GetWindowText( hControl, process, 100 );

	hControl = GetDlgItem( IDC_WINDOWCLASS );
	::GetWindowText( hControl, classname, 100 );

	hControl = GetDlgItem( IDC_WINDOWTITLE );
	::GetWindowText( hControl, window, 255 );
	
	AddExclusionToTree( process, classname, window );

	// then clear the boxes
	ClearTextControls( );

	return 0;
}

LRESULT OnUpdateExclusionClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
//	char buffer[255];
	HWND hControl = GetDlgItem( IDC_EXCLUSIONLIST );

//	TVITEMEX item;
//	int type;
	HTREEITEM selected = TreeView_GetSelection( hControl );

	if( selected )
	{
		UpdateNodesFromTextBoxes( selected );
	}

	return 0;
}
int GetNodeType( HTREEITEM hitem )
{
	HWND hControl = GetDlgItem( IDC_EXCLUSIONLIST );
	int retval = -1;

	TVITEMEX item;

	if( hitem )
	{
		item.hItem = hitem;
		item.mask = TVIF_IMAGE;

		if( TreeView_GetItem( hControl, &item ) )
		{
			retval = item.iImage;
		}
	}

	return retval;
}

void GetNodeText( HTREEITEM hitem, LPTSTR *text )
{
	TCHAR buffer[255];
	HWND hControl = GetDlgItem( IDC_EXCLUSIONLIST );

	TVITEMEX item;

	if( text == NULL )
		return;

	if( ( *text ) != NULL )
		return;


	item.hItem = hitem;
	item.mask = TVIF_TEXT;
	item.pszText = buffer;
	item.cchTextMax = 255;

	if( TreeView_GetItem( hControl, &item  ) )
	{
		(*text) = new TCHAR[ _tcslen( item.pszText ) + 1 ];

		_tcscpy( (*text), item.pszText );
	}
}

void SetNodeText( HTREEITEM hitem, LPCTSTR text )
{
	TCHAR buffer[255];
	HWND hControl = GetDlgItem( IDC_EXCLUSIONLIST );

	TVITEMEX item;

	_tcsncpy( buffer, text, 255 );

	if( hitem )
	{
		item.hItem = hitem;
		item.mask = TVIF_TEXT;
		item.pszText = buffer;
		item.cchTextMax = 255;

		TreeView_SetItem( hControl, &item );
	}

}
HTREEITEM FindProcessNode( LPCTSTR name )
{
	TCHAR buffer[100];
	HWND hControl = GetDlgItem( IDC_EXCLUSIONLIST );

	TVITEMEX item;
	HTREEITEM hItem = TreeView_GetFirstVisible( hControl );
	

	while( hItem )
	{
		item.hItem = hItem;
		item.mask = TVIF_TEXT;
		item.pszText = buffer;
		item.cchTextMax = 100;

		if( TreeView_GetItem( hControl, &item  ) )
		{
			if( _tcsicmp( name, buffer ) != 0 )
			{
				// no item, so get the next item
				hItem = TreeView_GetNextSibling( hControl, hItem );
			}
			else
			{
				break;
			}
		}
	}

	return hItem;
}

HTREEITEM FindWindowNode( HTREEITEM process, LPCTSTR name )
{
	TCHAR buffer[255];
	HWND hControl = GetDlgItem( IDC_EXCLUSIONLIST );

	TVITEMEX item;
	HTREEITEM hItem = TreeView_GetChild( hControl, process );

	while( hItem )
	{
		item.hItem = hItem;
		item.mask = TVIF_TEXT;
		item.pszText = buffer;
		item.cchTextMax = 255;

		if( TreeView_GetItem( hControl, &item  ) )
		{
			if( _tcsicmp( name, buffer ) != 0 )
			{
				// no item, so get the next item
				hItem = TreeView_GetNextSibling( hControl, hItem );
			}
			else
			{
				break;
			}
		}
	}

	return hItem;
}
void UpdateTextBoxesFromNode( HTREEITEM hitem )
{
	int type = -1;
	LPTSTR text = NULL;
	HWND hControl = NULL;
	HTREEITEM hActualItem = hitem;

	// loop from the item to all it's parents
	while( hActualItem != NULL )
	{
		// get the nodes type
		type = GetNodeType( hActualItem );

		GetNodeText( hActualItem, &text );

		if( text )
		{
			// find the text control based on type
			if( type == m_processImage )
			{
				// get the new process name
				hControl = GetDlgItem( IDC_PROCESSNAME );
			}
			else if( type == m_classImage ) 
			{
				// get the new class name
				hControl = GetDlgItem( IDC_WINDOWCLASS );
			}
			else if( type == m_windowImage )
			{
				// get the new class name
				hControl = GetDlgItem( IDC_WINDOWTITLE );
			} 

			// set the window text
			::SetWindowText( hControl, text );

			// clean up
			delete[] text;
			text = NULL;
		}

		hControl = GetDlgItem( IDC_EXCLUSIONLIST );
		hActualItem = TreeView_GetParent( hControl, hActualItem );
	}
}

void UpdateNodesFromTextBoxes( HTREEITEM hitem )
{
	int type = -1;
	HWND hControl = NULL;
	HTREEITEM hActualItem = hitem;
	TCHAR buffer[255];

	// loop from the item to all it's parents
	while( hActualItem != NULL )
	{
		// get the nodes type
		type = GetNodeType( hActualItem );

		// find the text control based on type
		if( type == m_processImage )
		{
			// get the new process name
			hControl = GetDlgItem( IDC_PROCESSNAME );
		}
		else if( type == m_classImage ) 
		{
			// get the new class name
			hControl = GetDlgItem( IDC_WINDOWCLASS );
		}
		else if( type == m_windowImage )
		{
			// get the new class name
			hControl = GetDlgItem( IDC_WINDOWTITLE );
		} 

		// set the window text
		::GetWindowText( hControl, buffer, 255 );

		if( _tcslen( buffer ) > 0 )
		{
			SetNodeText( hActualItem, buffer );
		}

		hControl = GetDlgItem( IDC_EXCLUSIONLIST );

		hActualItem = TreeView_GetParent( hControl, hActualItem );
	}
}

void ClearTextControls( )
{
	HWND hControl = GetDlgItem( IDC_PROCESSNAME );
	::SetWindowText( hControl, _T( "" ) );
	
	hControl = GetDlgItem( IDC_WINDOWCLASS );
	::SetWindowText( hControl, _T( "" ) );

	hControl = GetDlgItem( IDC_WINDOWTITLE );
	::SetWindowText( hControl, _T( "" ) );
}
LRESULT OnRemoveExclusionClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	HWND hControl = GetDlgItem( IDC_EXCLUSIONLIST );

	HTREEITEM item = TreeView_GetSelection( hControl );

	if( item )
	{
		TreeView_DeleteItem( hControl, item );
	}

	return 0;
}

LRESULT OnBrowseClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	OPENFILENAME ofn;       // common dialog box structure
	TCHAR szFile[260];       // buffer for file name
//	HANDLE hf;              // file handle
	HWND hControl = NULL;
	LPTSTR buffer = NULL;;

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = m_hWnd;
	ofn.lpstrFile = szFile;
	//
	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
	// use the contents of szFile to initialize itself.
	//
	ofn.lpstrFile[0] = _T( '\0' );
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = _T( "Applications\0*.exe\0" );
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

	// Display the Open dialog box. 

	if( GetOpenFileName(&ofn) == TRUE ) 
	{
		// get the process name i.e. get rid of the path
		hControl = GetDlgItem( IDC_PROCESSNAME );

		if( GetFilename( szFile, &buffer ) )
		{
			::SetWindowText( hControl, buffer );

			if( buffer )
			{
				delete [] buffer;
				buffer = NULL;
			}
		}
	}


	return 0;
}
LRESULT OnLButtonDown( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	HWND hControl = GetDlgItem( IDC_FINDERICON );

	RECT r = {0};

	::GetWindowRect( hControl, &r );

	RECT winRect = {0};

	GetWindowRect( &winRect );

	POINT pt;

	pt.x = GET_X_LPARAM(lParam) + winRect.left; 
	pt.y = GET_Y_LPARAM(lParam) + winRect.top;

	if( ( pt.x > r.left ) && ( pt.x < r.right ) )
	{
		if( ( pt.y > r.top ) && ( pt.y < r.bottom ) )
		{
			POINT screenpoint;

			//MessageBox("Left Button Down");
			// Change the icon
			HICON icon = LoadIcon( _hInstance, MAKEINTRESOURCE( IDI_FINDER2 ) );

			::SendMessage( hControl, STM_SETICON, (WPARAM)icon, NULL );

			// set the cursor
			HCURSOR curs = LoadCursor( _hInstance, MAKEINTRESOURCE( IDC_FINDER ) );

			screenpoint.x = r.left + BULLSEYE_CENTER_X_OFFSET;
			screenpoint.y = r.top + BULLSEYE_CENTER_Y_OFFSET;

			m_oldCursor = SetCursor( curs );

			SetCursorPos( screenpoint.x, screenpoint.y );

			SetCapture( );

			m_trackingWindows = true;

			m_currentWindow = NULL;
		}
	}

	return 0;
}

LRESULT OnLButtonUp( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	if( m_trackingWindows )
	{
		ReleaseCapture( );

		m_trackingWindows = false;

		RefreshWindow( m_currentWindow );

		HWND hControl = GetDlgItem( IDC_FINDERICON );

		HICON icon = LoadIcon( _hInstance, MAKEINTRESOURCE( IDI_FINDER1 ) );

		::SendMessage( hControl, STM_SETICON, (WPARAM)icon, NULL );

		m_oldCursor = SetCursor( m_oldCursor );

		DestroyCursor( m_oldCursor );

		m_oldCursor = NULL;

		if( m_currentWindow )
		{
			TCHAR buffer[100];
			TCHAR buffer2[255];

			// check we have the correct window
			if( !IsAppWindow( m_currentWindow ) )
			{
				HWND owner = ::GetWindow( m_currentWindow, GW_OWNER );

				if( IsAppWindow( owner ) )
				{
					m_currentWindow = owner;
				}
			}

			HWND hControl = GetDlgItem( IDC_WINDOWCLASS );

			GetClassName( m_currentWindow, buffer, 100 );

			::SetWindowText( hControl, buffer );

			hControl = GetDlgItem( IDC_WINDOWTITLE );

			::GetWindowText( m_currentWindow, buffer2, 255 );

			::SetWindowText( hControl, buffer2 );

			hControl = GetDlgItem( IDC_PROCESSNAME );
			
			GetProcessExecutableName( m_currentWindow, buffer, 100 );

			::SetWindowText( hControl, buffer );
		}
	}

	return 0;
}

LRESULT OnMouseMove( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	if( m_trackingWindows )
	{
		// do we want to track this window?
		POINT pt;

		//pt.x = GET_X_LPARAM(lParam);
		//pt.y = GET_Y_LPARAM(lParam);

		GetCursorPos( &pt );

		HWND window = WindowFromPoint( pt );

		// need to get the main window when we go over another window

		HWND owner = ::GetWindow( window, GW_OWNER );

		if( m_currentWindow != window )
		{
			if( IsAppWindow( window ) || ( owner != NULL && IsAppWindow( owner ) ) )
			{
				if( m_currentWindow )
					RefreshWindow( m_currentWindow );

				m_currentWindow = window;

				HighlightFoundWindow( m_currentWindow );
			}
		}

		return 0;
	}

	return 1;
}

long HighlightFoundWindow( HWND hwndFoundWindow )
{
    // The DC of the found window.
    HDC     hWindowDC = NULL;  

    // Handle of the existing pen in the DC of the found window.
    HGDIOBJ hPrevPen = NULL;   

    // Handle of the existing brush in the DC of the found window.
    HGDIOBJ hPrevBrush = NULL; 

    RECT        rect; // Rectangle area of the found window.
    long        lRet = 0;

    // Get the screen coordinates of the rectangle 
    // of the found window.
	::GetWindowRect (hwndFoundWindow, &rect);

    // Get the window DC of the found window.
	hWindowDC = ::GetWindowDC (hwndFoundWindow);

    if (hWindowDC)
    {
        // Select our created pen into the DC and 
        // backup the previous pen.
		hPrevPen = ::SelectObject (hWindowDC, m_pen);

        // Select a transparent brush into the DC and 
        // backup the previous brush.
		hPrevBrush = ::SelectObject (hWindowDC, 
            GetStockObject(HOLLOW_BRUSH));

        // Draw a rectangle in the DC covering 
        // the entire window area of the found window.
        Rectangle (hWindowDC, 3, 5, 
            (rect.right - rect.left) - 3, (rect.bottom - rect.top) - 5);

        // Reinsert the previous pen and brush 
        // into the found window's DC.
		::SelectObject (hWindowDC, hPrevPen);

		::SelectObject (hWindowDC, hPrevBrush);

        // Finally release the DC.
		::ReleaseDC (hwndFoundWindow, hWindowDC);
    }

    return lRet;
}


long RefreshWindow (HWND hwndWindowToBeRefreshed)
{
  long lRet = 0;

  ::InvalidateRect (hwndWindowToBeRefreshed, NULL, TRUE);
  ::UpdateWindow (hwndWindowToBeRefreshed);
  ::RedrawWindow (hwndWindowToBeRefreshed, NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);

  return lRet;
}
LRESULT OnClose( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	SaveExclusions( );

	DestroyWindow( );

	return 0;  // Let the system set the focus
}

LRESULT OnSelectionChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
//	TVITEMEX itemEx;
//	char buffer[255];
	HWND hControl = NULL;
	
	// clear the text boxes
	ClearTextControls( );

	if( pnmh )
	{
		LPNMTREEVIEW item = (LPNMTREEVIEW)pnmh;

		if( item->itemNew.hItem == NULL )
		{
			hControl = GetDlgItem( IDC_REMOVEEXCLUSION );

			::EnableWindow( hControl, FALSE );
		}
		else
		{
			UpdateTextBoxesFromNode( item->itemNew.hItem );

			hControl = GetDlgItem( IDC_REMOVEEXCLUSION );

			::EnableWindow( hControl, TRUE );
		}
	}
	else
	{
		::EnableWindow( hControl, FALSE );
	}

	return 0;
}

LRESULT OnUseRegexClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	TCHAR val[6];
	HWND hControl = GetDlgItem( IDC_USEREGEX );

	if( ::SendMessage( hControl, BM_GETCHECK, NULL, NULL ) == BST_CHECKED )
	{
		_stprintf( val, _T( "True\0" ) );
	}
	else
	{
		_stprintf( val, _T( "False\0" ) );
	}

	SetStringSetting( _T( "Advanced Excludes" ), val );
	UpdateSettingsString( SETTING_USEREGEX, (LPCTSTR)&val );
	return 0;
}


virtual void OnFinalMessage( HWND hWnd )
{
	Release( );
}

public:
STDMETHODIMP_( void )Show( HWND parent, RECT r  )
{
	Create( parent, r );

	ShowWindow( SW_SHOW );

	SetLocation( r );
}
STDMETHODIMP_( HWND )GetHWND( void )
{
	return m_hWnd;
}



private:
void AddExclusionToTree( LPTSTR process, LPTSTR classname, LPTSTR window )
{
	TVITEM tvi; 
    TVINSERTSTRUCT tvins; 
	HTREEITEM hPrev = (HTREEITEM)TVI_FIRST; 
	TCHAR buf[255];

	tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;

	// get the text
	HWND hControl = hControl = GetDlgItem( IDC_EXCLUSIONLIST );

	if( process && ( _tcslen( process ) > 0 ) )
	{
		HTREEITEM item = FindProcessNode( process );
		
		if( !item ) // if the item isn't in the list, add it
		{
			_tcsncpy( buf, process, 255 );

			// Set the text of the item. 
			tvi.pszText = buf; 
			tvi.cchTextMax = sizeof(tvi.pszText)/sizeof(tvi.pszText[0]); 
			tvi.iImage = tvi.iSelectedImage = m_processImage;

			tvins.item = tvi; 
			tvins.hInsertAfter = hPrev;
			tvins.hParent = TVI_ROOT;

			item = TreeView_InsertItem(hControl, &tvins); 

			TreeView_Expand( hControl, item, TVM_EXPAND );
		}

		// are we adding a window?
		if( classname && ( _tcslen( classname ) > 0 ) )
		{
			HTREEITEM childItem = FindWindowNode( item, classname );

			if( !childItem )
			{
				_tcsncpy( buf, classname, 255 );

				// Set the text of the item. 
				tvi.pszText = buf; 
				tvi.cchTextMax = sizeof(tvi.pszText)/sizeof(tvi.pszText[0]); 
				tvi.iImage = tvi.iSelectedImage = m_classImage;

				tvins.item = tvi; 
				tvins.hInsertAfter = TVI_FIRST;
				tvins.hParent = item;

				childItem = TreeView_InsertItem( hControl, &tvins ); 

				TreeView_Expand( hControl, childItem, TVM_EXPAND );
			}

			if( window && ( _tcslen( window ) > 0 ) )
			{
				// reuse item as we don't need it again
				item = FindWindowNode( childItem, window );

				if( !item )
				{
					_tcsncpy( buf, window, 255 );

					// Set the text of the item. 
					tvi.pszText = buf; 
					tvi.cchTextMax = sizeof(tvi.pszText)/sizeof(tvi.pszText[0]); 
					tvi.iImage = tvi.iSelectedImage = m_windowImage;

					tvins.item = tvi; 
					tvins.hInsertAfter = TVI_FIRST;
					tvins.hParent = childItem;

					TreeView_InsertItem(hControl, &tvins); 

					TreeView_Expand( hControl, item, TVM_EXPAND );
				}
			}
		}
	}
}
void SetLocation( RECT r )
{
	SetWindowPos( HWND_TOP, &r, SWP_NOSIZE );
}

void UpdateSettingsDWORD( int settingid, DWORD value )
{
	::SendMessage( _mainHwnd, UWM_SETTINGCHANGED, (LPARAM) settingid, (WPARAM) value );
}

void UpdateSettingsString( int settingid, LPCTSTR value )
{
	::SendMessage( _mainHwnd, UWM_SETTINGCHANGED, (LPARAM) settingid, (WPARAM) value );
}

bool IsAppWindow( HWND hwnd )
{
	// this function will work out if we have an app window or not
	if( ::IsWindowVisible( hwnd ) )
	{
		DWORD exStyle = ::GetWindowLong( hwnd, GWL_EXSTYLE );
		HWND parent = ( HWND )::GetWindowLong( hwnd, GWLP_HWNDPARENT );
		HWND owner = ::GetWindow( hwnd, GW_OWNER );

		// if WS_EX_APPWINDOW is set then we have 
		if( ( exStyle & WS_EX_APPWINDOW ) == WS_EX_APPWINDOW )
		{
			return true;
		}

		// otherwise, if it has no parent and isn't a child window
		if( ( parent == NULL ) && (  owner == NULL ) )
		{
			if( ( exStyle & WS_EX_TOOLWINDOW ) != WS_EX_TOOLWINDOW )
			{
				return true;
			}
		}
	}

	return false;
}


HIMAGELIST SetupImageList(HINSTANCE hinst) 
{ 
    HIMAGELIST himlIcons;  // handle to new image list 
    HICON hicon;           // handle to icon 
 
    // Create a masked image list large enough to hold the icons. 
    himlIcons = ImageList_Create(16, 16, ILC_MASK, 4, 0); 
 
    // Load the icon resources, and add the icons to the image list. 
    hicon = LoadIcon(hinst, MAKEINTRESOURCE(IDI_PROCESS)); 
    m_processImage = ImageList_AddIcon(himlIcons, hicon); 
 
	hicon = LoadIcon(hinst, MAKEINTRESOURCE(IDI_CLASS)); 
    m_classImage = ImageList_AddIcon(himlIcons, hicon); 

    hicon = LoadIcon(hinst, MAKEINTRESOURCE(IDI_WINDOW)); 
    m_windowImage = ImageList_AddIcon(himlIcons, hicon);

    return himlIcons; 
} 

void LoadExclusions( )
{
	HWND hControl = GetDlgItem( IDC_EXCLUSIONLIST );
	
	// loading the list from the exclusion list
	vector<PEXCLUSIONITEM>::iterator it;

	if( !_exclusionList.empty( ) )
	{
		for( it = _exclusionList.begin( ); it != _exclusionList.end( ); it++ )
		{
			if( (*it) != NULL )
			{
				// got an exclusion item, so add it
				AddExclusionToTree( (*it)->processName, (*it)->className, (*it)->windowTitle );
			}
		}
	}
}

void SaveExclusions( )
{
	// loop through the data in the exclusion list and write it to the actual list
	HWND hControl = GetDlgItem( IDC_EXCLUSIONLIST );
	HKEY hExclusionsKey = NULL;
	HKEY hProcessKey = NULL;
//	DWORD operation;
	TCHAR buffer[100];
	TCHAR buffer1[100];
	TCHAR buffer2[255];
//	TCHAR name[100];
	int count = 0;
	
	TVITEMEX item;
	HTREEITEM hItem = NULL;
	TVITEMEX classItem;
	HTREEITEM hClassItem = NULL;
	TVITEMEX processItem;
	HTREEITEM hProcessItem = NULL;

	// clear the list
	if( !_exclusionList.empty( ) )
	{
		vector<PEXCLUSIONITEM>::iterator it;

		for( it = _exclusionList.begin( ); it != _exclusionList.end( ); it++ )
		{
			if( (*it) != NULL )
			{
				delete (*it);
			}
		}

		_exclusionList.clear( );
	}

	// get the first node in the tree
	hProcessItem = TreeView_GetFirstVisible( hControl );

	// do we have something?
	while( hProcessItem )
	{
		// get its item
		processItem.hItem = hProcessItem;
		processItem.mask = TVIF_TEXT | TVIF_CHILDREN;
		processItem.pszText = buffer;
		processItem.cchTextMax = 100;

		if( TreeView_GetItem( hControl, &processItem  ) )
		{
			// does the process have classes?
			if( processItem.cChildren > 0 )
			{
				// loop through the classes
				hClassItem = TreeView_GetChild( hControl, hProcessItem );

				// do we have something?
				while( hClassItem )
				{
					// get its item
					classItem.hItem = hClassItem;
					classItem.mask = TVIF_TEXT | TVIF_CHILDREN;
					classItem.pszText = buffer1;
					classItem.cchTextMax = 100;

					if( TreeView_GetItem( hControl, &classItem  ) )
					{
						// does the class have windows?
						if( classItem.cChildren > 0 )
						{
							// loop through the windows
							hItem = TreeView_GetChild( hControl, hClassItem );

							// do we have something?
							while( hItem )
							{
								// get its item
								item.hItem = hItem;
								item.mask = TVIF_TEXT;
								item.pszText = buffer2;
								item.cchTextMax = 255;

								if( TreeView_GetItem( hControl, &item  ) )
								{
									// have a process with class and window
									PEXCLUSIONITEM ei = MakeExclusionItem( processItem.pszText, classItem.pszText, item.pszText );
									_exclusionList.push_back( ei );
								}

								// get the next window
								hItem = TreeView_GetNextSibling( hControl, hItem );
							}
						}
						else
						{
							// have a process with class
							PEXCLUSIONITEM ei = MakeExclusionItem( processItem.pszText, classItem.pszText, NULL );
							_exclusionList.push_back( ei );
						}
					}

					// get the next class
					hClassItem = TreeView_GetNextSibling( hControl, hClassItem );
				}
			}
			else
			{
				// only have a process
				PEXCLUSIONITEM ei = MakeExclusionItem( processItem.pszText, NULL, NULL );
				_exclusionList.push_back( ei );
			}
		}
		
		// get the next process
		hProcessItem = TreeView_GetNextSibling( hControl, hProcessItem );
	}

	// clear the registry before saving
	int val = SaveExclusionList( _exclusionList, true );
	int i = 0;
}

bool GetFilename( LPTSTR pathfile, LPTSTR *file )
{
	TCHAR *pdest = NULL;

	if( *file )
		return false;

	// look for the last \ in the path file
	pdest = _tcsrchr( pathfile, _T( '\\' ) );

	if( pdest )
	{
		pdest++;

		(*file) = new TCHAR[ _tcslen( pdest ) + 1 ];

		_tcscpy( (*file), pdest );

		return true;
	}

	return false;
}

};

HRESULT CreateExclusionsTabPage( ITabPage **ppTabPage )
{
	return CComCreator< CComObject< CExclusionsSettingsDlg > >::CreateInstance( NULL, __uuidof( ITabPage ), ( void** ) ppTabPage );
}