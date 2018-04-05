//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// Settings.cpp
//
// Copyright (C) Stephen Kew. All Rights Reserved.
//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "stdafx.h"
#include "deskcon.h"

#define BUFSIZE 100
#define DESKCON_SETTING_PATH _T( "Software\\miniMIZE\\Settings" )

void GetStringFromRegistry( HKEY key, LPCTSTR path, LPCTSTR name, LPTSTR *value, LPCTSTR defaultval, bool adddefault )
{
	HKEY hSettingsKey = NULL;
	TCHAR newValue[BUFSIZE];
    DWORD dwBufLen=BUFSIZE;

	if( key )
	{
		// open the key
		if( RegOpenKeyEx( key, path, 0, KEY_ALL_ACCESS, &hSettingsKey ) != ERROR_SUCCESS )
		{
			RegCreateKey( key, path, &hSettingsKey );
		}

		if( hSettingsKey )
		{
			if( RegQueryValueEx( hSettingsKey, name, NULL, NULL, ( LPBYTE ) newValue, &dwBufLen ) == ERROR_SUCCESS )
			{
				(*value) = new TCHAR[ dwBufLen + 1 ];

				_tcscpy( (*value), newValue );
			}
			else
			{
				if( defaultval )
				{
					( *value ) = new TCHAR[ _tcslen( defaultval ) + 1 ];

					_tcscpy( ( *value ), defaultval );

					// put the default in the reg
					if( adddefault )
					{
						RegSetValueEx( hSettingsKey, name, NULL, REG_SZ, ( LPBYTE ) ( *value ), _tcslen( ( *value ) ) + 1 );
					}
				}
			}

			RegCloseKey( hSettingsKey );
		}
	}
}

void GetDWORDFromRegistry( HKEY key, LPCTSTR path, LPCTSTR name, DWORD *value, DWORD defaultval, bool adddefault )
{
	HKEY hSettingsKey = NULL;
	DWORD newValue;
    DWORD dwBufLen = sizeof( newValue );

	if( key )
	{
		// open the key
		if( RegOpenKeyEx( key, path, 0, KEY_ALL_ACCESS, &hSettingsKey ) != ERROR_SUCCESS )
		{
			RegCreateKey( key, path, &hSettingsKey );
		}

		if( hSettingsKey )
		{
			if( RegQueryValueEx( hSettingsKey, name, NULL, NULL, ( LPBYTE )&newValue, &dwBufLen ) == ERROR_SUCCESS )
			{
				( *value ) = newValue;
			}
			else
			{
				( *value ) = defaultval;

				// put the default in the reg
				if( adddefault )
				{
					RegSetValueEx( hSettingsKey, name, NULL, REG_DWORD, ( LPBYTE )value, sizeof( ( *value ) ) );
				}
			}

			RegCloseKey( hSettingsKey );
		}
	}
}

void WriteStringToRegistry( HKEY key, LPCTSTR path, LPCTSTR name, LPCTSTR value )
{
	HKEY hSettingsKey = NULL;

	if( key )
	{
		// open the key
		if( RegOpenKeyEx( key, path, 0, KEY_ALL_ACCESS, &hSettingsKey ) != ERROR_SUCCESS )
		{
			RegCreateKey( key, path, &hSettingsKey );
		}

		if( hSettingsKey )
		{
			RegSetValueEx( hSettingsKey, name, NULL, REG_SZ, ( LPBYTE ) value , _tcslen( value ) + 1 );

			RegCloseKey( hSettingsKey );
		}
	}
}

void WriteDWORDToRegistry( HKEY key, LPCTSTR path, LPCTSTR name, DWORD value )
{
	HKEY hSettingsKey;

	if( key )
	{
		// open the key
		if( RegOpenKeyEx( key, path, 0, KEY_ALL_ACCESS, &hSettingsKey ) != ERROR_SUCCESS )
		{
			RegCreateKey( key, path, &hSettingsKey );
		}

		if( hSettingsKey )
		{
			RegSetValueEx( hSettingsKey, name, NULL, REG_DWORD, ( LPBYTE ) &value, sizeof( value ) );

			RegCloseKey( hSettingsKey );
		}
	}
}

void DeleteValueFromRegistry( HKEY key, LPCTSTR path, LPCTSTR name )
{
	HKEY hSettingsKey;

	if( key )
	{
		// open the key
		if( RegOpenKeyEx( key, path, 0, KEY_ALL_ACCESS, &hSettingsKey ) == ERROR_SUCCESS )
		{
			RegDeleteValue( hSettingsKey, name );

			RegCloseKey( hSettingsKey );
		}
	}
}


void GetStringSetting( LPCTSTR name, LPTSTR *value, LPCTSTR defaultval, bool adddefault )
{
	GetStringFromRegistry( HKEY_CURRENT_USER, DESKCON_SETTING_PATH, name, value, defaultval, adddefault );
}

void GetStringSetting( LPCTSTR name, LPTSTR *value, LPCTSTR defaultval )
{
	GetStringSetting( name, value, defaultval, true );
}

void SetStringSetting( LPCTSTR name, LPCTSTR value )
{
	WriteStringToRegistry( HKEY_CURRENT_USER, DESKCON_SETTING_PATH, name, value );
}

void GetDWORDSetting( LPCTSTR name, DWORD *value, DWORD defaultval, bool adddefault )
{
	GetDWORDFromRegistry(  HKEY_CURRENT_USER, DESKCON_SETTING_PATH, name, value, defaultval, adddefault );
}

void GetDWORDSetting( LPCTSTR name, DWORD *value, DWORD defaultval )
{
	GetDWORDSetting( name, value, defaultval, true );
}

void SetDWORDSetting( LPCTSTR name, DWORD value )
{
	WriteDWORDToRegistry( HKEY_CURRENT_USER, DESKCON_SETTING_PATH, name, value );
}