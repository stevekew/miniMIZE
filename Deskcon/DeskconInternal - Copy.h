//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// DeskconInternal.h
//
// Copyright (C) Stephen Kew. All Rights Reserved.
//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#pragma once

#include <vector>

using namespace std;
//
// General functions
//

//void CalculateAvailablePosition( HWND hWnd, int* x, int* y );

//
// Setting and registry function
//
void GetStringSetting( LPCTSTR name, LPTSTR *value, LPCTSTR defaultval, bool adddefault );
void GetStringSetting( LPCTSTR name, LPTSTR *value, LPCTSTR defaultval );
void SetStringSetting( LPCTSTR name, LPCTSTR value );
void GetDWORDSetting( LPCTSTR name, DWORD *value, DWORD defaultval, bool adddefault );
void GetDWORDSetting( LPCTSTR name, DWORD *value, DWORD defaultval );
void SetDWORDSetting( LPCTSTR name, DWORD value );

void WriteStringToRegistry( HKEY key, LPCTSTR path, LPCTSTR name, LPCTSTR value );
void GetStringFromRegistry( HKEY key, LPCTSTR path, LPCTSTR name, LPTSTR *value, LPCTSTR defaultval, bool adddefault );
void DeleteValueFromRegistry( HKEY key, LPCTSTR path, LPCTSTR name );

//
// Inline functions
//

int inline FindNearestTen( int val )
{
	return ( ( val + 5 ) / 10 ) * 10;
}

//
// Exclusion structure
//
typedef struct exclusionItem
{
	LPTSTR processName;
	LPTSTR className;
	LPTSTR windowTitle;

	exclusionItem( )
	{
		processName = NULL;
		className = NULL;
		windowTitle = NULL;
	}

	~exclusionItem( )
	{
		if( processName )
		{
			delete [] processName;
			processName = NULL;
		}

		if( className )
		{
			delete [] className;
			className = NULL;
		}

		if( windowTitle )
		{
			delete [] windowTitle;
			windowTitle = NULL;
		}

	}

	void SetProcessName( LPTSTR name )
	{
		if( name )
		{
			if( processName )
			{
				delete [] processName;
				processName = NULL;
			}
			processName = new TCHAR[ _tcslen( name ) + 1 ];

			_tcscpy( processName, name );
		}
	}

	void SetClassName( LPTSTR name )
	{
		if( name )
		{
			if( className )
			{
				delete [] className;
				className = NULL;
			}
			className = new TCHAR[ _tcslen( name ) + 1 ];

			_tcscpy( className, name );
		}
	}

	void SetWindowTitle( LPTSTR name )
	{
		if( name )
		{
			if( windowTitle )
			{
				delete [] windowTitle;
				windowTitle = NULL;
			}
			windowTitle = new TCHAR[ _tcslen( name ) + 1 ];

			_tcscpy( windowTitle, name );
		}
	}
} EXCLUSIONITEM, *PEXCLUSIONITEM;

//
// Exclusion functions
//
bool GetProcessExecutableName( HWND hwnd, LPTSTR name, int length );
PEXCLUSIONITEM MakeExclusionItem( LPTSTR process, LPTSTR classname, LPTSTR windowtext );
bool MatchExclusionItem( EXCLUSIONITEM item1, EXCLUSIONITEM item2, bool matchclass, bool matchtitle );
bool MatchExclusionItem( EXCLUSIONITEM item1, EXCLUSIONITEM item2, bool matchtitle );
bool MatchExclusionItem( EXCLUSIONITEM item1, EXCLUSIONITEM item2 );

int LoadExclusionList( vector<PEXCLUSIONITEM> *list );
int SaveExclusionList( vector<PEXCLUSIONITEM> list, bool clear );
//
// Window message #defines
//

#define UWM_WINMINIMIZE_MSG _T( "UWM_WINMINIMIZE_MSG-EC8C34C3-15FE-4911-AA41-46176D2251F9" )
#define UWM_WINMAXIMIZE_MSG _T( "UWM_WINMAXIMIZE_MSG-09DABCF4-A2CF-4d80-BBEF-5656C2FBFEE3" )
#define UWM_WINMINIMIZED_MSG _T( "UWM_WINMINIMIZED_MSG-AFB82EFD-5DD8-4a66-AA35-00EB8F630ACC" )
#define UWM_WINCLOSE_MSG _T( "UWM_WINCLOSE_MSG-6BC92757-63D7-47d5-B2CF-FE610C0BE290" )
#define UWM_SETTINGCHANGED_MSG _T( "UWM_SETTINGCHANGED_MSG-77D646CD-2A19-49BD-A6DB-DE2C15094B8B" )
#define UWM_SHOWNOTIFYICON_MSG _T( "UWM_SHOWNOTIFYICON_MSG-0C34B5FE-F788-477F-A7E4-24D72B172EAD" )
#define UWM_TASKBARCREATED_MSG _T( "TaskbarCreated" )

//
// General #defines
//

#define DESKCON_THUMBNAIL_WIDTH 100
#define DESKCON_THUMBNAIL_HEIGHT 100
#define DESKCON_THUMBNAIL_GAP 10

#define KEY_DOWN 0x8000

//
// Debug #defines
//
#define SAFEDEBUGWRITELINE( pdebugwin, message ) \
			if( pdebugwin != NULL ) \
			{ \
				pdebugwin->WriteLine( message ); \
			}\

#define SAFEDEBUGWRITE( pdebugwin, message ) \
			if( pdebugwin != NULL ) \
			{ \
				pdebugwin->Write( message ); \
			}

//
// Window level defines
//
#define WL_ALWAYSONBOTTOM 1
#define WL_NORMAL 2
#define WL_ALWAYSONTOP 3

//
// Size mode defines
//
#define SM_USEWIDTH 0
#define SM_USEHEIGHT 1
#define SM_SMARTSIZE 2

//
// Click style defines
//
#define CS_SINGLECLICK 0
#define CS_DOUBLECLICK 1

//
// Settings page defines
//
#define SP_GENERALSETTINGS 0
#define SP_THUMBNAILSETTINGS 1
#define SP_THUMBNAILPOSITION 2
#define SP_HOTKEYS 3
#define SP_EXCLUSIONLIST 4

//
// Hotkey defines
//
#define HOTKEY_CYCLELEVEL 0
#define HOTKEY_MINIMIZEALL 1
#define HOTKEY_HIDESHOW 2

//
// Position defines
//
#define POS_TOPLEFTRIGHT 0 // top left, moving right
#define POS_TOPRIGHTLEFT 1 // top right, moving left
#define POS_BOTTOMLEFTRIGHT 2 // bottom left, moving right
#define POS_BOTTOMRIGHTLEFT 3 // bottom right, moving left
#define POS_TOPLEFTDOWN 4 // top left, moving down
#define POS_TOPRIGHTDOWN 5 // top left, moving down
#define POS_BOTTOMLEFTUP 6 // bottom left, moving up
#define POS_BOTTOMRIGHTUP 7 // bottom right, moving up

//
// Settings Class Types
//
#define SETTINGS_CLASS_REGISTRY 0

#define T GET_SERVICE( T ) \
	FabricApp::Instance( )->Services( ).GetService<T>( );
