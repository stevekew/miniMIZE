//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// Logger.h
//
// Copyright (C) Stephen Kew. All Rights Reserved.
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
