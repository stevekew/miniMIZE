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