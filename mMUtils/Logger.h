//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// Logger.h
//
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
//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// CLogger
class CLogger
{
private:
	// data variables
	FILE* _stream;
	DWORD _processId;
	TCHAR _processName[MAX_PATH];
	bool _newLine;

public:
	CLogger(LPCSTR filename);
	CLogger(LPCSTR filename, LPTSTR startupString);
	virtual ~CLogger( );

private:
	void InitialiseLoggerInfo(void);

public:
	void WriteLine( LPCTSTR buffer );
	void Write( LPCTSTR buffer );
	void Write( LPCTSTR buffer, bool EOL );

	BOOL StartLogger( LPCSTR filename );
	BOOL StopLogger( void );
};


#ifdef _DEBUG
#define DEFINE_LOGGER() CLogger* _logger = NULL;
#else
#define DEFINE_LOGGER()
#endif

#ifdef _DEBUG
#define CREATE_LOGGER(filename)	_logger = new CLogger( filename );
#else
#define CREATE_LOGGER(filename)
#endif

#ifdef _DEBUG
#define CREATE_LOGGER_WITH_MSG(filename,message) _logger = new CLogger( filename, message );
#else
#define CREATE_LOGGER_WITH_MSG(filename, message)
#endif

#ifdef _DEBUG
#define LOG_WRITELINE(message)	_logger->WriteLine( message );
#else
#define LOG_WRITELINE(message)
#endif 
