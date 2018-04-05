//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// HotkeysTabPage.cpp
//
// Copyright (C) Stephen Kew. All Rights Reserved.
//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "stdafx.h"
#include "resource.h"       // main symbols
#include "deskcon.h"
#include "deskconinternal.h"
#include "settings.h"

extern HWND _mainHwnd;
extern UINT UWM_SETTINGCHANGED;

extern CComPtr< IDebugWindow > _debugWindow;

//typedef struct tagHOTKEYSTRUCT
//{
//    WORD        wModifierKeys;
//    unsigned    uKey;
//    char        szText [STR_LEN];
//} HOTKEYSTRUCT;
	
//class CHotkeysControlImpl : public CWindowImpl<CHotkeysControlImpl>,
//							public CComObjectRootEx<CComSingleThreadModel>
//{
//private:
//	HWND hHotKeyEdit;
//	HOTKEYSTRUCT    ghksHotKey;
//
//public:
//	CHotkeysControlImpl( )
//	{
//		hHotKeyEdit = NULL;
//	}
//
//	~CHotkeysControlImpl( )
//	{
//	}
//
//BEGIN_MSG_MAP( CHotkeysControlImpl )
//	MESSAGE_HANDLER( WM_KEYDOWN, OnKeyDown )
//	MESSAGE_HANDLER( WM_SYSKEYDOWN, OnKeyDown )
//	MESSAGE_HANDLER( WM_KEYUP, OnKeyUp )
//	MESSAGE_HANDLER( WM_SYSKEYUP, OnKeyUp )
//	MESSAGE_HANDLER( WM_SYSCHAR, OnKeyUp )
//	MESSAGE_HANDLER( WM_CHAR, OnKeyUp )
//END_MSG_MAP()
//
//LRESULT OnKeyUp( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
//{
//	switch (wParam)
//    {
//        // If it's a restricted key, then pass it to windows
//        case VK_RETURN:
//        case VK_ESCAPE:
//        case VK_TAB:
//			{
//				bHandled = false;
//				return 1;
//            /*return (CallWindowProc (glpOldHotKeyEditProc, hHotKeyEdit, uMsg,
//                                    wParam, lParam));*/
//			}
//
//            default:
//                // If only the modifier keys were pressed, 
//                // OR
//                // If SHIFT is the only modifier key with a valid key,
//                // then erase and initialize the edit control
//                if (ghksHotKey.uKey == 0 ||
//                (wModifierKeys == SHIFT_BIT && uMessage == WM_KEYUP))
//                    DoErasingStuff (hHotKeyEdit);
//
//                return (0);
//    }
//}
//
//LRESULT OnKeyDown( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
//{
//	*pwModifierKeys = 0;
//
//    // Set wModifierKeys according to the modifier keys down
//
//    if (GetKeyState (VK_CONTROL) & 0x1000)
//        *pwModifierKeys |= CONTROL_BIT;
//
//    if (GetKeyState (VK_SHIFT) & 0x1000)
//        *pwModifierKeys |= SHIFT_BIT;
//
//    if (lParam & 0x20000000)
//        *pwModifierKeys |= MENU_BIT;
//
//    // If the keys are for the dialog box or Windows, pass them
//
//    if (wParam == VK_TAB || wParam == VK_ESCAPE
//    || wParam == VK_RETURN || wParam == VK_SPACE)
//    {
//        DoErasingStuff (hHotKeyEdit);
//		bHandled = false;
//		return 1;
//            /*return (CallWindowProc (glpOldHotKeyEditProc, hHotKeyEdit, uMessage,
//                                wParam, lParam));*/
//    }
//    else if (wParam == VK_BACK && *pwModifierKeys == 0)
//    {
//        // If BACKSPACE, then erase the edit control and the shortcut key structure;
//        // and make the INSTALL button disabled.
//
//        DoErasingStuff (hHotKeyEdit);
//        return (0);
//    }
//    else if (*pwModifierKeys)
//    {
//        // Display the keys but only if the modifier(s) present
//
//        DisplayHotKey (GetParent (hHotKeyEdit), *pwModifierKeys, wParam, lParam);
//        return (0);
//
//    }
//}
//
//BOOL Subclass( HWND window )
//{
//	hHotKeyEdit = window;
//
//	return SubclassWindow( window );
//}
//
//BOOL StopSubclass( )
//{
//	UnsubclassWindow( );
//}
//
//void DisplayHotKey (HWND hDlg,    WORD wModifierKeys,
//                    WORD wKeyNum, LONG lKeyInfo)
//{
//    char    szHotKey [STR_LEN];
//    char    szKeyName [STR_LEN];
//    int     iHotKeyStrLen;
//    HWND    hHotKeyEdit = GetDlgItem (hDlg, IDD_HOTKEYEDIT);
//
//    szHotKey [0] = 0;
//
//    // Disable the button if it was enabled.
//    EnableWindow (GetDlgItem (hDlg, IDD_INSTALL), FALSE);
//
//    // Create the string to display the modifier keys.
//
//    if (wModifierKeys & MENU_BIT)
//        wsprintf ((LPSTR) szHotKey, "Alt");
//
//    if (wModifierKeys & CONTROL_BIT)
//        if (wModifierKeys & MENU_BIT)
//            lstrcat ((LPSTR) szHotKey, (LPSTR) "+Ctrl");
//        else
//            lstrcat ((LPSTR) szHotKey, (LPSTR) "Ctrl");
//
//    if (wModifierKeys & SHIFT_BIT)
//        if (wModifierKeys & MENU_BIT || wModifierKeys & CONTROL_BIT)
//            lstrcat ((LPSTR) szHotKey, (LPSTR) "+Shift");
//        else
//            lstrcat ((LPSTR) szHotKey, (LPSTR) "Shift");
//
//
//    // Display modifier keys
//
//    SetWindowText (hHotKeyEdit, (LPSTR) szHotKey);
//    
//    // Save the keys
//
//    ghksHotKey.wModifierKeys = wModifierKeys;
//    ghksHotKey.uKey = 0;
//
//    // Set the cursor at the end of the text
//    iHotKeyStrLen = lstrlen ((LPSTR) szHotKey);
//
//    SendMessage (hHotKeyEdit, EM_SETSEL, 0, 
//                MAKELONG (iHotKeyStrLen, iHotKeyStrLen));
//
//
//    // Add the character key and then display it.
//
//    if (GetKeyNameText (lKeyInfo, (LPSTR) szKeyName, STR_LEN)
//    && (wKeyNum != VK_MENU && wKeyNum != VK_CONTROL && wKeyNum != VK_SHIFT))
//    {
//        lstrcat ((LPSTR) szHotKey, (LPSTR) "+");
//        lstrcat ((LPSTR) szHotKey, (LPSTR) szKeyName);
//
//        // Save the character key
//
//        ghksHotKey.uKey = wKeyNum;
//
//        // Display rest of the shortcut key
//
//        SetWindowText (hHotKeyEdit, (LPSTR) szHotKey);
//
//        // Let the user Install the shortcut key
//        // if it's a valid shortcut key
//        if (wModifierKeys != SHIFT_BIT)
//            EnableWindow (GetDlgItem (hDlg, IDD_INSTALL), TRUE);
//
//        // Set the cursor at the end of the text
//        iHotKeyStrLen = lstrlen ((LPSTR) szHotKey);
//
//        SendMessage (hHotKeyEdit, EM_SETSEL, 0, 
//                    MAKELONG (iHotKeyStrLen, iHotKeyStrLen));
//    }
//
//} // DisplayHotKey()
//
//
//void DoErasingStuff (HWND hHotKeyEdit)
//{
//    SetWindowText (hHotKeyEdit, (LPSTR) '\0');
//    ghksHotKey.wModifierKeys = 0;
//    ghksHotKey.uKey = 0;
//} 
//
//};

class CHotkeysTabPage : public CDialogImpl<CHotkeysTabPage>,
							public CComObjectRootEx<CComSingleThreadModel>,
							public ITabPage
{
private:

//	CHotkeysControlImpl m_impl;
public:
	CHotkeysTabPage( )
	{
	}

	~CHotkeysTabPage( )
	{
	}

//
// IUnknown
//
BEGIN_COM_MAP(CHotkeysTabPage)
	COM_INTERFACE_ENTRY(ITabPage)
END_COM_MAP()

enum { IDD = IDD_HOTKEYS };

BEGIN_MSG_MAP(CHotkeysTabPage)
	MESSAGE_HANDLER( WM_INITDIALOG, OnInitDialog )
	COMMAND_HANDLER( IDC_HKEXCLUDECTRL, BN_CLICKED, OnHKExcludeCtrlClicked )
	COMMAND_HANDLER( IDC_HKEXCLUDEALT, BN_CLICKED, OnHKExcludeAltClicked )
	COMMAND_HANDLER( IDC_HKEXCLUDESHIFT, BN_CLICKED, OnHKExcludeShiftClicked )
	COMMAND_HANDLER( IDC_HKEXCLUDENONE, BN_CLICKED, OnHKExcludeNoneClicked )
	MESSAGE_HANDLER( WM_CLOSE, OnClose )
END_MSG_MAP()

LRESULT OnInitDialog( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	long val = AddRef( );
	DWORD hotkey;

	DWORD rule = HKCOMB_NONE;//HKCOMB_A | HKCOMB_C | HKCOMB_CA | HKCOMB_S | HKCOMB_SA | HKCOMB_SC | HKCOMB_SCA;

	DWORD newRule = HOTKEYF_CONTROL | HOTKEYF_ALT;//HOTKEYF_EXT;

	HWND hControl = GetDlgItem( IDC_HKCYCLELEVEL );

	GetDWORDSetting( _T( "Cycle Level Key" ), &hotkey, 0 );

	::SendMessage( hControl, HKM_SETRULES, (WPARAM)rule, newRule);

	::SendMessage( hControl, HKM_SETHOTKEY, (WPARAM)hotkey, NULL);

	//BOOL sub = m_impl.Subclass( hControl );

	hControl = GetDlgItem( IDC_HKMINIMIZE );

	GetDWORDSetting( _T( "Minimize Key" ), &hotkey, 0 );

	::SendMessage( hControl, HKM_SETRULES, (WPARAM)rule, newRule );

	::SendMessage( hControl, HKM_SETHOTKEY, (WPARAM)hotkey, NULL);

	hControl = GetDlgItem( IDC_HKSHOWHIDE );

	GetDWORDSetting( _T( "Show Thumb Key" ), &hotkey, 0 );

	::SendMessage( hControl, HKM_SETRULES, (WPARAM)rule, newRule );

	::SendMessage( hControl, HKM_SETHOTKEY, (WPARAM)hotkey, NULL);

	GetDWORDSetting( _T( "Exclude Key" ), &hotkey, 0 );

	switch( hotkey )
	{
		case 0:
			{
				hControl = GetDlgItem( IDC_HKEXCLUDENONE );
			}
			break;
		case VK_SHIFT:
			{
				hControl = GetDlgItem( IDC_HKEXCLUDESHIFT );
			}
			break;
		case VK_CONTROL:
			{
				hControl = GetDlgItem( IDC_HKEXCLUDECTRL );
			}
			break;
		case VK_MENU:
			{
				hControl = GetDlgItem( IDC_HKEXCLUDEALT );
			}
			break;
	}

	::SendMessage( hControl, BM_SETCHECK, (WPARAM)BST_CHECKED, (LPARAM)NULL );

	return 1;  // Let the system set the focus
}
LRESULT OnClose( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	GetHotkeys( );

	DestroyWindow( );

	return 0;  // Let the system set the focus
}

LRESULT OnHKExcludeNoneClicked( WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled )
{
	// store the value and update the setting
	SetDWORDSetting( _T( "Exclude Key" ), 0 );

	UpdateSettingsDWORD( SETTING_HKEXCLUSION, 0 );

	return 0;
}

LRESULT OnHKExcludeCtrlClicked( WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled )
{
	// store the value and update the setting
	SetDWORDSetting( _T( "Exclude Key" ), VK_CONTROL );

	UpdateSettingsDWORD( SETTING_HKEXCLUSION, VK_CONTROL );

	return 0;
}

LRESULT OnHKExcludeAltClicked( WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled )
{
	// store the value and update the setting
	SetDWORDSetting( _T( "Exclude Key" ), VK_MENU );

	UpdateSettingsDWORD( SETTING_HKEXCLUSION, VK_MENU );
	return 0;
}

LRESULT OnHKExcludeShiftClicked( WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled )
{
	// store the value and update the setting
	SetDWORDSetting( _T( "Exclude Key" ), VK_SHIFT );

	UpdateSettingsDWORD( SETTING_HKEXCLUSION, VK_SHIFT );
	return 0;
}

void GetHotkeys( void )
{
	LPTSTR text[10];

	// get the hotkey - Cycle Level
	HWND hControl = GetDlgItem( IDC_HKCYCLELEVEL );
	
	DWORD hotkey = (DWORD)::SendMessage( hControl, HKM_GETHOTKEY, NULL, NULL );

	// store it in the reg
	SetDWORDSetting( _T( "Cycle Level Key" ), hotkey );

	UpdateSettingsDWORD( SETTING_HKCYCLELEVEL, hotkey );

	// get the hotkey - minimize all
	hControl = GetDlgItem( IDC_HKMINIMIZE );
	
	hotkey = (DWORD)::SendMessage( hControl, HKM_GETHOTKEY, NULL, NULL );

	// store it in the reg
	SetDWORDSetting( _T( "Minimize Key" ), hotkey );

	UpdateSettingsDWORD( SETTING_HKMINIMIZEALL, hotkey );

	// get the hotkey - show/ hide thumbnails
	hControl = GetDlgItem( IDC_HKSHOWHIDE );
	
	hotkey = (DWORD)::SendMessage( hControl, HKM_GETHOTKEY, NULL, NULL );

	// store it in the reg
	SetDWORDSetting( _T( "Show Thumb Key" ), hotkey );

	UpdateSettingsDWORD( SETTING_HKSHOWHIDETHUMB, hotkey );
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
};

HRESULT CreateHotkeysTabPage( ITabPage **ppTabPage )
{
	return CComCreator< CComObject< CHotkeysTabPage > >::CreateInstance( NULL, __uuidof( ITabPage ), ( void** ) ppTabPage );
}