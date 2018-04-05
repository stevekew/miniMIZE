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

#include "stdafx.h"
#include "Predicates.h"

//
// FindExclusionItemREPredicate
//
FindExclusionItemREPredicate::FindExclusionItemREPredicate( LPTSTR processname, LPTSTR classname, LPTSTR title )
{
	_tcsncpy( m_process, processname, 100 );
	_tcsncpy( m_class , classname, 100 );
	_tcsncpy( m_title, title, 255 );
}

bool FindExclusionItemREPredicate::operator() ( const PEXCLUSIONITEM item )
{
	CAtlRegExp<> regEx;
	CAtlREMatchContext<> regExMatch;

	bool retval = false;

	if(item != NULL)
	{
		if( item->processName )
		{
			// only bother if the process matches
			// process name is a regex
			REParseError status = regEx.Parse( item->processName, FALSE );

			if( status == REPARSE_ERROR_OK )
			{
				if( regEx.Match( m_process, &regExMatch ) )
				{
					if( item->className )
					{
						// we have a match, so continue
						status = regEx.Parse( item->className, FALSE );

						if( status == REPARSE_ERROR_OK )
						{
							if( regEx.Match( m_class, &regExMatch ) )
							{
								// we have a match, so continue
								if( item->windowTitle )
								{
									status = regEx.Parse( item->windowTitle, FALSE );

									if( status == REPARSE_ERROR_OK )
									{
										if( regEx.Match( m_title, &regExMatch ) )
										{
											retval = true;
										}
									}
								}
								else
								{
									retval = true;
								}
							}
						}
					}
					else
					{
						retval = true;
					}
				}
			}
		}
	}

	return retval;
}

//
// FindExclusionItemPredicate
//
FindExclusionItemPredicate::FindExclusionItemPredicate( LPTSTR processname, LPTSTR classname, LPTSTR title )
{
	_tcsncpy( m_process, processname, 100 );
	_tcsncpy( m_class , classname, 100 );
	_tcsncpy( m_title, title, 255 );
}

bool FindExclusionItemPredicate::operator() ( const PEXCLUSIONITEM item )
{
	bool retval = false;

	if(item != NULL)
	{
		if( item->processName )
		{
			// only bother if the process matches
			if( _tcsicmp( item->processName, m_process ) == 0 )
			{
				// once here, are we matching more?
				if( item->className )
				{
					if( _tcsicmp( item->className, m_class ) == 0 )
					{
						// are we matching window titles?
						if( item->windowTitle )
						{
							if( _tcsicmp( item->windowTitle, m_title ) == 0 )
							{
								retval = true;
							}
						}
						else
						{
							retval = true;
						}
					}
				}
				else
				{
					retval = true;
				}
			}
		}
	}

	return retval;
}

//
// FindThumbnailSnaptoPredicate - find a snapto window
//
FindThumbnailSnaptoPredicate::FindThumbnailSnaptoPredicate( RECT r )
{
	m_r = r;
}

bool FindThumbnailSnaptoPredicate::operator() ( CComPtr< IThumbWindow > thumbWindow )
{
	bool retval = false;

	RECT r = {0};

	r = thumbWindow->GetThumbnailRect( );

	if( ( m_r.bottom > r.top ) && ( m_r.top < r.bottom ) )
	{
		if( ( ( m_r.right - r.left) <= 10 )  && ( ( m_r.right - r.left) > 0 ) )
		{
			retval = true;
		}
		else if( ( ( m_r.left - r.right ) <= 10 ) && ( ( m_r.left - r.right ) > 0 ) )
		{
			retval = true;
		}
	}
	else if ( ( m_r.right > r.left ) && ( m_r.left < r.right ) )
	{
		if( ( ( m_r.top - r.bottom ) <= 10 ) && ( ( m_r.top - r.bottom ) > 0 ) )
		{
			retval = true;
		}
		else if( ( ( m_r.bottom - r.top ) <= 10 ) && ( ( m_r.bottom - r.top ) > 0 ) )
		{
			retval = true;
		}
	}

	return retval;
}