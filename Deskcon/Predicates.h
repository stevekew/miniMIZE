#pragma once
#include "stdafx.h"
#include "Deskcon.h"
#include "DeskconInternal.h"

#include <atlrx.h>

//
// FindExclusionItemREPredicate - Find an exclusion using regular expressions
//
class FindExclusionItemREPredicate
{
public:
	FindExclusionItemREPredicate( LPTSTR processname, LPTSTR classname, LPTSTR title );
	bool operator() ( const PEXCLUSIONITEM item );
private:
	TCHAR m_process[100];
	TCHAR m_class[100];
	TCHAR m_title[255];
};

//
// FindExclusionItemPredicate - Find an exclusion using stricmp
//

class FindExclusionItemPredicate
{
public:
	FindExclusionItemPredicate( LPTSTR processname, LPTSTR classname, LPTSTR title );
	bool operator() ( const PEXCLUSIONITEM item );
	
private:
	TCHAR m_process[100];
	TCHAR m_class[100];
	TCHAR m_title[255];
};

//
// FindThumbnailSnapto - Find a thumbnail within the snapto region
//
class FindThumbnailSnaptoPredicate
{
public:
	FindThumbnailSnaptoPredicate( RECT r );
	bool operator() ( CComPtr< IThumbWindow > thumbWindow );
	
private:
	RECT m_r;
};