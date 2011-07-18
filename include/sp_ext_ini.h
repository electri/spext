/** Copyright (c) 2008-2010
* All rights reserved.
* 
* 文件名称:	sp_ext_ini.h     
* 摘	 要:	封装ini文件的读写操作
* 
* 当前版本：	1.0
* 作	 者:	陈学术
* 操	 作:	新建
* 完成日期:	2010年03月224日
*/
#ifndef __SP_EXT_INI_H__
#define __SP_EXT_INI_H__

#include <string>
#include <windows.h>

namespace sp_ext
{
class ini_file 
{
public:
   ini_file();
   // ini_file_name must be full path
   ini_file( const char * ini_file_name );
   virtual ~ini_file();

public:
   UINT set_ini_file_name(const char * ini_file_name);
   //
   // Reads an integer from the ini-file.
   UINT get_int(const char * lpszSection, const char * lpszEntry, int nDefault=0);
   // Reads a boolean value from the ini-file.
   BOOL get_boolean(const char * lpszSection, const char * lpszEntry, BOOL bDefault=FALSE);
   // Reads a string from the ini-file.
   std::string get_string(const char * lpszSection, const char * lpszEntry, const char * lpszDefault=NULL);
   // Reads a binaryt lump of data from the ini-file.
   BOOL get_binary(const char * lpszSection, const char * lpszEntry, BYTE** ppData, UINT* pBytes);
   //
   // Writes an integer to the ini-file.
   BOOL write_int(const char * lpszSection, const char * lpszEntry, int nValue);
   // Writes a boolean value to the ini-file.
   BOOL write_boolean(const char * lpszSection, const char * lpszEntry, BOOL bValue);
   // Writes a string to the ini-file.
   BOOL write_string(const char * lpszSection, const char * lpszEntry, const char * lpszValue);
   // Writes a binary lump of data to the ini-file.
   BOOL write_binary(const char * lpszSection, const char * lpszEntry, LPBYTE pData, UINT nBytes);
   // Writes an 'expand string' to the ini-file.
   BOOL write_expand_string(const char * lpszSection, const char * lpszEntry, const char * lpszValue);
   //
   // Removes an item from the current ini-file.
   BOOL delete_key(const char * lpszSection, const char * lpszEntry);
   // Removes a complete section from the ini-file.
   BOOL delete_section(const char * lpszSection);

// Variables
protected:
	std::string ini_file_name_; // The current ini-file used.
};

}// sp_ext
#endif // __SP_EXT_INI_H__
