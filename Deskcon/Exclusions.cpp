//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// Exclusions.h
//
// Copyright (C) Stephen Kew. All Rights Reserved.
//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#include "stdafx.h"
#include "deskconinternal.h"
#include <Psapi.h>
#include <vector>

using namespace std;


//
// Exclusion registry functions
//
#define EXCLUSION_REG_PATH _T( "Software\\miniMIZE\\Exclusions" )

int GetExclusionWindowCount( HKEY classkey )
{
	DWORD count = 0;

	if( !classkey )
		return -1;

	RegQueryInfoKey( classkey, NULL, NULL, NULL, NULL, NULL, NULL, &count, NULL, NULL, NULL, NULL );

    return (int) count;
}

int GetExclusionClassCount( HKEY processkey )
{
	DWORD count = 0;

	if( !processkey )
		return -1;

	RegQueryInfoKey( processkey, NULL, NULL, NULL, &count, NULL, NULL, NULL, NULL, NULL, NULL, NULL );

    return (int) count;
}

// this function is because when saving the list, we don't want to keep opening the key
bool SaveExclusion( HKEY key, PEXCLUSIONITEM item )
{
	HKEY processKey = NULL;
	HKEY classKey = NULL;
	TCHAR name[5];
	bool retval = false;

	// save the item to the registry
	if( !key )
		return false;

	// first, create the key
	if( item->processName )
	{
		// if we have a process, create its key
		if( RegCreateKey( key, item->processName, &processKey ) == ERROR_SUCCESS )
		{
			if( item->className )
			{
				// if we have a window class, create its key
				if( RegCreateKey( processKey, item->className, &classKey ) == ERROR_SUCCESS )
				{
					// write the window text to the key
					if( item->windowTitle )
					{
						int index = GetExclusionWindowCount( classKey );

						// no need to increment because we start at 0 and count is num+1
						_stprintf( name, _T( "%d" ), index );

						RegSetValueEx( classKey, name, NULL, REG_SZ, ( LPBYTE ) item->windowTitle , _tcslen( item->windowTitle ) + 1 );
						retval = true;
					}

					RegCloseKey( classKey );
				}
			RegCloseKey( processKey );
			}
		}
	}

	return retval;
}

// save a single key
bool SaveExclusion( PEXCLUSIONITEM item )
{
	// open the key
	HKEY exclusionKey = NULL;
	bool retval = false;

	if( RegOpenKeyEx( HKEY_CURRENT_USER, EXCLUSION_REG_PATH, 0, KEY_ALL_ACCESS, &exclusionKey ) != ERROR_SUCCESS )
	{
		RegCreateKey( HKEY_CURRENT_USER, EXCLUSION_REG_PATH, &exclusionKey );
	}

	retval = SaveExclusion( exclusionKey, item );

	RegCloseKey( exclusionKey );

	return retval;
}


int SaveExclusionList( vector<PEXCLUSIONITEM> list, bool clear )
{
	HKEY exclusionKey = NULL;

	// iterate through the list and save the items
	vector<PEXCLUSIONITEM>::iterator it;

	if( clear )
	{
		if( RegOpenKeyEx( HKEY_CURRENT_USER, _T( "Software\\miniMIZE" ), 0, KEY_ALL_ACCESS, &exclusionKey ) == ERROR_SUCCESS )
		{
			long ret = SHDeleteKey( exclusionKey, _T( "Exclusions" ) );

			int i = 0;

			RegCloseKey( exclusionKey );
		}
	}

	// open the registrykey
	if( RegOpenKeyEx( HKEY_CURRENT_USER, EXCLUSION_REG_PATH, 0, KEY_ALL_ACCESS, &exclusionKey ) != ERROR_SUCCESS )
	{
		RegCreateKey( HKEY_CURRENT_USER, EXCLUSION_REG_PATH, &exclusionKey );
	}

	for( it = list.begin( ); it != list.end( ); it++ )
	{
		if( (*it) != NULL )
		{
			SaveExclusion( exclusionKey, (*it) );
		}
	}

	if( exclusionKey )
	{
		RegCloseKey( exclusionKey );
		exclusionKey = NULL;
	}

	return list.size( );
}
int LoadExclusionList( vector<PEXCLUSIONITEM> *list )
{
	// key enumeration variables
	HKEY exclusionKey = NULL;
	HKEY processKey = NULL;
	HKEY classKey = NULL;
	FILETIME ftLastWriteTime;
	TCHAR		processName[MAX_PATH+1];
	TCHAR		className[MAX_PATH+1];
	DWORD		processNameSize;
	DWORD		classNameSize;
	DWORD		processRet;
	DWORD		classRet;
	long procCount = 0;
	long classCount = 0;
    
	// value enumeration 
	TCHAR name[10];
	TCHAR value[255];
	DWORD nameSize = 10;
	DWORD valueSize = 255;
	long valueCount = 0;
	DWORD	valueRet;

	// empty the list
	if( !list->empty( ) )
	{
		vector<PEXCLUSIONITEM>::iterator it;

		for( it = list->begin( ); it != list->end( ); it++ )
		{
			if( (*it) != NULL )
			{
				delete (*it);
			}
		}

		list->clear( );
	}

	// open the registrykey
	if( RegOpenKeyEx( HKEY_CURRENT_USER, EXCLUSION_REG_PATH, 0, KEY_ALL_ACCESS, &exclusionKey ) != ERROR_SUCCESS )
	{
		RegCreateKey( HKEY_CURRENT_USER, EXCLUSION_REG_PATH, &exclusionKey );
	}

	if( exclusionKey )
	{
		// enumerate through all the processes
		processRet = ERROR_SUCCESS;

		procCount = 0;
		while(processRet == ERROR_SUCCESS)
		{
			processNameSize = MAX_PATH; 
			processName[0] = '\0';
			
			processRet = RegEnumKeyEx(exclusionKey,
										procCount,
										processName, 
										&processNameSize, 
										NULL, 
										NULL,
										NULL,
										&ftLastWriteTime);

			if(processRet == ERROR_SUCCESS)
			{
				// we have the process key
				
				// open the key
				if( RegOpenKeyEx( exclusionKey, processName, NULL, KEY_READ, &processKey) == ERROR_SUCCESS )
				{
					
					// enumerate the window classes
					if( processKey )
					{
						// how many classes does it have?
						if( GetExclusionClassCount( processKey ) > 0 )
						{
							// enumerate through all the classes
							classRet = ERROR_SUCCESS;

							classCount = 0;
							while(classRet == ERROR_SUCCESS)
							{
								classNameSize = MAX_PATH; 
								className[0] = '\0';
								
								classRet = RegEnumKeyEx( processKey,
															classCount,
															className, 
															&classNameSize, 
															NULL, 
															NULL,
															NULL,
															&ftLastWriteTime);

								if(classRet == ERROR_SUCCESS)
								{
									// we have class key, so see how many windows it has
									// open the key
									if( RegOpenKeyEx( processKey, className, NULL, KEY_READ, &classKey) == ERROR_SUCCESS )
									{
										if( classKey )
										{
											if( GetExclusionWindowCount( classKey ) > 0 )
											{
												// we need an item for each window
												valueRet = ERROR_SUCCESS;

												valueCount = 0;
												while(valueRet == ERROR_SUCCESS)
												{
													nameSize = 10;
													name[0] = '\0';
													valueSize = 255;
													value[0] = '\0';

													valueRet = RegEnumValue( classKey,
																			 valueCount, 
																			 name, 
																			 &nameSize, 
																			 0, 
																			 NULL, 
																			 (LPBYTE)value, 
																			 &valueSize );

													if(valueRet == ERROR_SUCCESS)
													{
														//create an item with a process and class
														PEXCLUSIONITEM item = MakeExclusionItem( processName, className, value );

														// add it to the list
														list->push_back( item );
													}
													valueCount++;
												}
											}
											else
											{
												//create an item with a process and class
												PEXCLUSIONITEM item = MakeExclusionItem( processName, className, NULL );

												// add it to the list
												list->push_back( item );
											}
											// enumerate the window titles
											RegCloseKey( classKey );
										}
									}

									classCount++;
								}
							}
						}
						else
						{
							// just a simple process exclusion
							PEXCLUSIONITEM item = MakeExclusionItem( processName, NULL, NULL );

							// add it to the list
							list->push_back( item );
						}

						RegCloseKey( processKey );
					}
				}

				procCount++;
			}
		}
		RegCloseKey( exclusionKey );
	}
	return list->size( );
}

//
// General Exclusion functions
// 
PEXCLUSIONITEM MakeExclusionItem( LPTSTR process, LPTSTR classname, LPTSTR windowtext )
{
	PEXCLUSIONITEM pE = new EXCLUSIONITEM( );

	if( process )
	{
		pE->SetProcessName( process );
	}

	if( classname )
	{
		pE->SetClassName( classname );
	}

	if( windowtext )
	{
		pE->SetWindowTitle( windowtext );
	}

	return pE;
}

// makes a copy of an item
PEXCLUSIONITEM MakeExclusionItem( PEXCLUSIONITEM item )
{
	PEXCLUSIONITEM pE = new EXCLUSIONITEM( );

	if( item->processName )
	{
		pE->SetProcessName( item->processName );
	}

	if( item->className )
	{
		pE->SetClassName( item->className );
	}

	if( item->windowTitle )
	{
		pE->SetWindowTitle( item->windowTitle );
	}

	return pE;
}

bool MatchExclusionItem( EXCLUSIONITEM item1, EXCLUSIONITEM item2, bool matchclass, bool matchtitle )
{
	bool retval = false;

	if( ( item1.processName && item2.processName ) &&
		 ( _tcsicmp( item1.processName, item2.processName ) == 0 ) )
	{
		if( matchclass )
		{
			if( ( item1.className && item2.processName ) && 
					( _tcsicmp( item1.className, item2.className ) == 0 ) )
			{
				if( matchtitle )
				{
					if( ( item1.windowTitle && item2.windowTitle ) && 
						 ( _tcsicmp( item1.windowTitle, item2.windowTitle ) == 0 ) )
					{
						retval = true;
					}
				}
				else
					retval = true;
			}
		}
		else
			retval = true;
	}

	return retval;
}

bool MatchExclusionItem( EXCLUSIONITEM item1, EXCLUSIONITEM item2 )
{
	return MatchExclusionItem( item1, item2, true, true );
}

bool MatchExclusionItem( EXCLUSIONITEM item1, EXCLUSIONITEM item2, bool matchtitle )
{
	return MatchExclusionItem( item1, item2, true, matchtitle );
}

//
// Exclusion helper functions
//
bool GetProcessExecutableName( HWND hwnd, LPTSTR name, int length )
{
	bool bRet = false; 

	// get process id from window handle
	//
	DWORD dwProcess = 0;

	::GetWindowThreadProcessId( hwnd, &dwProcess );

	if ( dwProcess != NULL )
	{
		// get process handle from id
		HANDLE hProcess = ::OpenProcess(  PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwProcess );
		
		if ( hProcess != NULL )
		{
			// get module handle from process handle
			//
			DWORD count = 0;
			HMODULE hm[1] = { 0 };
			EnumProcessModules( (HANDLE)hProcess, hm, sizeof(hm), &count );

			// get file name from module handle
			GetModuleBaseName( hProcess, hm[0], name, length );

			// close process handle
			::CloseHandle( hProcess );

			bRet = true;
		}
	}		

    return bRet;

}

