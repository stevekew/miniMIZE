//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// RegistrySettings.cpp
//
// Copyright (C) Stephen Kew. All Rights Reserved.
//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "stdafx.h"
#include "deskcon.h"
#include "deskconinternal.h"

#include "fabric.h"

extern CComPtr< IDebugWindow > _debugWindow;
extern HINSTANCE _hInstance;
extern HWND _mainHwnd;

#define BUFSIZE 100
#define REG_SETTING_PATH _T( "Software\\miniMIZE\\Settings" )

class CRegistrySettings : public CComObjectRootEx<CComMultiThreadModel>,
						  public ISettings
{
private:

public:
	CRegistrySettings()
	{
	}

	~CRegistrySettings()
	{
	}

//
// IUnknown
//
BEGIN_COM_MAP(CRegistrySettings)
	COM_INTERFACE_ENTRY(ISettings)
END_COM_MAP()

public:

STDMETHODIMP_( void )RefreshSettings( void )
{
	// in a normal settings class, for example the xml settings class loader
	// we would load all of the settings in the file.
	// do we really want to cache all the registry settings??
	// for now, we won't
}

STDMETHODIMP_( bool )GetBoolSetting( LPCTSTR settingName, bool defaultValue  )
{
	bool settingValue = defaultValue;
	LPTSTR szValue = NULL;

	GetStringSetting( settingName, &szValue, defaultValue ? _T( "True" ) : _T( "False" ) );

	if( szValue )
	{
		settingValue = ( _tcsicmp( szValue, _T( "true" ) ) == 0 );

		delete [] szValue;
		szValue = NULL;
	}

	return settingValue;
}

STDMETHODIMP_( int )GetIntSetting( LPCTSTR settingName, int defaultValue  )
{
	DWORD settingValue = 0;

	GetDWORDSetting( settingName, &settingValue, (DWORD)defaultValue );
	
	return settingValue;
}

STDMETHODIMP_( LPTSTR )GetStringSetting( LPCTSTR settingName, LPTSTR defaultValue  )
{
	return GetStringSetting( settingName, defaultValue );
}

STDMETHODIMP_( double )GetDoubleSetting( LPCTSTR settingName, double defaultValue  )
{
	// we have no way of getting doubles from the registry, so we will just give back ints
	return GetIntSetting( settingName, (int)defaultValue );
}

private:
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
	GetStringFromRegistry( HKEY_CURRENT_USER, REG_SETTING_PATH, name, value, defaultval, adddefault );
}

void GetStringSetting( LPCTSTR name, LPTSTR *value, LPCTSTR defaultval )
{
	GetStringSetting( name, value, defaultval, true );
}

void SetStringSetting( LPCTSTR name, LPCTSTR value )
{
	WriteStringToRegistry( HKEY_CURRENT_USER, REG_SETTING_PATH, name, value );
}

void GetDWORDSetting( LPCTSTR name, DWORD *value, DWORD defaultval, bool adddefault )
{
	GetDWORDFromRegistry(  HKEY_CURRENT_USER, REG_SETTING_PATH, name, value, defaultval, adddefault );
}

void GetDWORDSetting( LPCTSTR name, DWORD *value, DWORD defaultval )
{
	GetDWORDSetting( name, value, defaultval, true );
}

void SetDWORDSetting( LPCTSTR name, DWORD value )
{
	WriteDWORDToRegistry( HKEY_CURRENT_USER, REG_SETTING_PATH, name, value );
}
};

HRESULT CreateRegistrySettings( ISettings **ppSettingsClass )
{
	return CComCreator< CComObject< CRegistrySettings > >::CreateInstance( NULL, __uuidof( ISettings ), ( void** ) ppSettingsClass );
}