
//////////////////////////////////////////////////////////////////////

//#include "stdafx.h"
#include "sp_ext_ini.h"
#include <tchar.h >


namespace sp_ext
{
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ini_file::ini_file()
{
   ini_file_name_.erase();
}

ini_file::ini_file(const char * ini_file_name)
{
   set_ini_file_name( ini_file_name );
}

ini_file::~ini_file()
{
   ::WritePrivateProfileString( NULL, NULL, NULL, ini_file_name_.c_str() );
}


//////////////////////////////////////////////////////////////////////
// Methods
//////////////////////////////////////////////////////////////////////

#define MAX_INI_BUFFER 300   // Defines the maximum number of chars we can
                             // read from the ini file 
UINT 
ini_file::set_ini_file_name(const char * ini_file_name)
{
	ini_file_name_ = ini_file_name;

   if( ini_file_name_.empty() )
	   return -1;

   return 0;
}


UINT 
ini_file::get_int(const char * lpszSection, const char * lpszEntry, int nDefault)
{
   if( ini_file_name_.empty() )
	   return 0; // error
   
   std::string sDefault;
   TCHAR aN[12] = {0};

   _ltot(nDefault, aN, 12);
   
   std::string s = get_string( lpszSection, lpszEntry, aN);

   return atol( s.c_str() );
}

std::string 
ini_file::get_string(const char * lpszSection, const char * lpszEntry, const char * lpszDefault)
{
   if( ini_file_name_.empty() )
	   return std::string(_T(""));

   TCHAR aTemp[MAX_INI_BUFFER] = {0};
   long ret = ::GetPrivateProfileString( lpszSection, lpszEntry, lpszDefault, aTemp, MAX_INI_BUFFER, ini_file_name_.c_str() );
   
   if( ret==0 )
	   return std::string(lpszDefault);
   
   return std::string(aTemp);
};

BOOL 
ini_file::get_boolean(const char * lpszSection, const char * lpszEntry, BOOL bDefault)
{
   std::string s = get_string(lpszSection,lpszEntry);

   if( s.empty() ) 
	   return bDefault;
   
   TCHAR c = _totupper( s[0] );
   
   switch( c )
   {
   case _T('Y'): // YES
   case _T('1'): // 1 (binary)
   case _T('O'): // OK
      return TRUE;
   default:
      return FALSE;
   };
};

BOOL 
ini_file::get_binary(const char * lpszSection, const char * lpszEntry, BYTE** ppData, UINT* pBytes)
{
   return FALSE;
};

BOOL 
ini_file::write_int(const char * lpszSection, const char * lpszEntry, int nValue)
{
   TCHAR aN[12] = {0};
   _ltot(nValue, aN, 12);
   
   return write_string( lpszSection, lpszEntry, aN);
};

BOOL 
ini_file::write_boolean(const char * lpszSection, const char * lpszEntry, BOOL bValue)
{
   std::string s;
   bValue ? s=_T("Y") : s=_T("N");

   return write_string( lpszSection, lpszEntry, s.c_str() );
};

BOOL 
ini_file::write_string(const char * lpszSection, const char * lpszEntry, const char * lpszValue)
{
	if( ini_file_name_.empty() )
		return FALSE;

   return ::WritePrivateProfileString( lpszSection, lpszEntry, lpszValue, ini_file_name_.c_str() );
};

BOOL 
ini_file::write_binary(const char * lpszSection, const char * lpszEntry, LPBYTE pData, UINT nBytes)
{
   return FALSE;
};

BOOL 
ini_file::write_expand_string(const char * lpszSection, const char * lpszEntry, const char * lpszValue)
{
   return FALSE;
};

BOOL 
ini_file::delete_key(const char * lpszSection, const char * lpszEntry)
{
   if( ini_file_name_.empty() )
	   return FALSE;

   return ::WritePrivateProfileString( lpszSection, lpszEntry, NULL, ini_file_name_.c_str() );
};

BOOL 
ini_file::delete_section(const char * lpszSection)
{
	if( ini_file_name_.empty() ) 
		return FALSE;

   return ::WritePrivateProfileString( lpszSection, NULL, NULL, ini_file_name_.c_str() );
};

}// sp_ext
