//#include "stdafx.h"  
#include "sp_ext_api_tools.h"
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <cguid.h>
#include <objbase.h>

#pragma warning(disable:4996)
#pragma warning(disable:4311)

namespace sp_ext
{
std::string 
get_module_path(HMODULE h)
{
	int i , n;
	char buf[MAX_PATH+1];

	memset(buf, 0, sizeof(buf));
	::GetModuleFileName(h, buf, MAX_PATH);

	n = (int)strlen(buf);

	for( i = n-1; i > 0; --i){

		if( buf[i]=='\\' || buf[i]=='/' ){

			buf[i+1] = 0;
			break;
		}
	}
	return buf;
}

std::string 
get_app_path()
{
	std::string appPath = get_module_path(NULL);
	return appPath;
}

int 
safe_snprintf(char* buf, int buf_size, const char* fmt, ...)
{
	int len = 0;

	va_list vl;
	va_start( vl, fmt );
	len = ::_vsnprintf( buf, buf_size - 1, fmt, vl );
	va_end(vl);

	buf[buf_size - 1] = 0;
	
	return len < 1 ? 0 : len;

}

std::string 
get_sys_path()
{
    std::string ret;
    char system_root[MAX_PATH] = {'\0'}; 
    
    if( ::GetSystemDirectory(system_root, sizeof(system_root)) )   
    {
        ret = system_root;
    }

    return ret;
}

std::string 
get_windows_path()
{
    std::string ret;
    char windows_path[MAX_PATH] = {'\0'}; 
    
    if( ::GetWindowsDirectory(windows_path, sizeof(windows_path)) )   
    {
        ret = windows_path;
    }

    return ret;
}

std::string 
get_sys_temp_path()
{
    std::string ret;
    char temp_path[MAX_PATH] = {'\0'}; 
    
    if( ::GetTempPath(sizeof(temp_path), temp_path) )   
    {
        ret = temp_path;
    }

    return ret;
}

size_t	
safe_strncpy(char* dest, const char* src, size_t n)
{
	size_t i = 0;
	char* ret = dest;

	if(n == 0 || NULL == src){
		dest[0] = 0; return 0;
	}

	for( i = 0; i < n; ++i ){
		if( 0 == (ret[i] = src[i]) ) return (i);
	}

	dest[n] = 0;	
	return n;
}

bool 
dir_exist(const char *path)
{
	bool b = false;
	char tmp[MAX_PATH];

	size_t n = safe_strncpy(tmp, path, sizeof(tmp)-2);

	for( int i=0; i< (int)n; ++i){
		if(tmp[i] == '/') tmp[i] = '\\';
	}

	if( tmp[n-1] == '\\' ) {
		tmp[n-1] = 0;
	}

	WIN32_FIND_DATA fd; 
	HANDLE hFind = FindFirstFile(tmp, &fd);

	if ( hFind != INVALID_HANDLE_VALUE ) {

		if( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			b = true;
	}

	FindClose(hFind);
	return b;
}

bool 
dir_create(const char * path)
{
	if( dir_exist(path) )
		return true;

	int i, n;
	char szPath[MAX_PATH] = {0};
	char szTmp[MAX_PATH] = {0};

	n = (int)safe_strncpy( szTmp, path, sizeof(szTmp)-1 );

	for( i=0; i<n; i++ ) {		

		if( szTmp[i] == '\\' || szTmp[i] == '/' ) {

			safe_strncpy( szPath, szTmp, i );
			::CreateDirectory( szPath, NULL );
		}
	}

	return dir_exist(path);
}

bool
dir_delete(const char* dir)
{
	del_all_files(dir, true);

	return TRUE == ::RemoveDirectory(dir);

}

bool 
del_all_files(const char* dir, bool enter_sub_dir)
{
	bool ret = true;
	WIN32_FIND_DATA wfd; 
	HANDLE h = NULL;
	std::string tmp = "";
	std::string cur_dir = dir;
	std::string fmt = "";
	char c = cur_dir[cur_dir.length()-1];

	if( c != '\\' && c != '/' )
		cur_dir += "\\";

	fmt = cur_dir + "*.*";

	h = ::FindFirstFile(fmt.c_str(), &wfd); 

	while( h != INVALID_HANDLE_VALUE && 0 != ::FindNextFile(h, &wfd))
	{
		tmp = cur_dir + wfd.cFileName;

		if( !(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
		{
			::DeleteFile(tmp.c_str());
		}
		else
		{
			if( enter_sub_dir ) /// ÔÊÐíÉ¾³ý×ÓÄ¿Â¼
			{
				bool b = false;
				if (wfd.cFileName[0] == '.')
				{
					if (wfd.cFileName[1] == '\0' || (wfd.cFileName[1] == '.' && wfd.cFileName[2] == '\0'))
						b = true;
				}

				if( !b )
				{
					del_all_files(tmp.c_str(), enter_sub_dir);
					::RemoveDirectory(tmp.c_str());
				}
			}
		}
	}

	FindClose(h);
	return true;
}

bool 
delete_file_ex(const char* file_name)
{
	int nRet = access(file_name, 0);
	if ( nRet == -1 )
	{
		return false;
	}
	else
	{
		SetFileAttributes(file_name, FILE_ATTRIBUTE_NORMAL);   
		return ( 0 != DeleteFile(file_name) );
	}
}

long 
get_file_size(const char* file_name)
{
	int nRet = access(file_name, 0);
	if (nRet==-1)
	{
		return -1;
	}
	else
	{
		long nFileSize=0;
		struct stat fFileMesg;
		stat(file_name, &fFileMesg);
		nFileSize = fFileMesg.st_size;
		return nFileSize;
	}    
}

bool 
find_file(const char* file_name)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	hFind = FindFirstFile(file_name, &FindFileData);
	if ( hFind == INVALID_HANDLE_VALUE ) 
	{
		return (false);
	} 
	else 
	{
		FindClose(hFind);
		return (true);
	}
}

e_win_ver 
get_windows_ver()
{
	OSVERSIONINFO   osvi;     
	memset(&osvi,   0,   sizeof(OSVERSIONINFO));     
	osvi.dwOSVersionInfoSize   =   sizeof(OSVERSIONINFO);     
	GetVersionEx   (&osvi);     
	char   OSName[50]   =   {0};   

	if( osvi.dwPlatformId == 1 )   
	{   
		if( osvi.dwMinorVersion == 0 )   
		{   
			return e_win_ver_95;
		}
		else   if( osvi.dwMinorVersion == 10 )   
		{   
			return e_win_ver_98;
		}
		else   if(osvi.dwMinorVersion   ==   90)   
		{   
			return e_win_ver_MELLINNIUM;
		}   

	}
	else if( osvi.dwPlatformId == 2 )     
	{   
		if ( osvi.dwMajorVersion == 3 )   
		{   
			return e_win_ver_NT;
		}
		else if( osvi.dwMajorVersion == 4 )   
		{   
			return e_win_ver_NT;
		}
		else if( osvi.dwMajorVersion == 5 )   
		{   
			if( osvi.dwMinorVersion == 0 )   
			{   
				return e_win_ver_2000;
			}
			else if( osvi.dwMinorVersion == 1 )   
			{   
				return e_win_ver_XP;
			}
			else   if( osvi.dwMinorVersion == 2 )   
			{   
				return e_win_ver_SERVER2003;
			}   
		}
		else if ( osvi.dwMajorVersion == 6 )
		{
			return e_win_ver_VISTA;
		}
		else if ( osvi.dwMajorVersion == 7 )
		{
			return e_win_ver_7;
		}
	}   
	return e_win_ver_XP;
}

char* 
str_find_lastof(char* str, char key, int len/* = -1*/)
{
	char* p = str;
	char* r = NULL;

	if( -1 == len )
	{
		while( *p ) {

			if( *p == key)	{ r = p; }

			++p;
		}
	}
	else
	{
		while( --len >= 0 ) {

			if( str[len] == key)
				return (str + len);
		}
	}

	return r;
}

const char* 
str_find_lastof(const char* str, char key, int len/* = -1*/)
{
	const char* p = str;
	const char* r = NULL;

	if( -1 == len )
	{
		while( *p ) {

			if( *p == key)	{ r = p; }

			++p;
		}
	}
	else {

		while( --len >= 0 ) {

			if( str[len] == key)
				return (str + len);
		}
	}

	return r;
}


char* 
str_find_lastof(char* str, const char* key, int len/* = -1*/)
{
	char* p = str;
	char* r = NULL;
	const char* k = key;

	if( -1 == len )
	{
		while( *p ) {

			k = key;

			while( *k ) {

				if( *k == *p ) {
					r = p; break;
				}

				++k;
			}

			++p;
		}
	}
	else {

		while( --len >= 0 )
		{
			k = key;

			while( *k ) {

				if( *k == str[len] ) {
					return (str + len);
				}

				++k;
			}

		}
	}

	return r;
}

const char* 
str_find_lastof(const char* str, const char* key, int len/* = -1*/)
{
	const char* p = str;
	const char* r = NULL;
	const char* k = key;

	if( -1 == len )
	{
		while( *p ) {

			k = key;

			while( *k ) {

				if( *k == *p ) {
					r = p; break;
				}

				++k;
			}

			++p;
		}
	}
	else
	{
		while( --len >= 0 )
		{
			k = key;

			while( *k ) {

				if( *k == str[len] ) {
					return (str + len);
				}

				++k;
			}

		}
	}

	return r;
}

char* to_str(char* buf, int v)
{
	sprintf(buf, "%d", v);
	return buf;
}
char* to_str(char* buf, unsigned int v)
{
	sprintf(buf, "%u", v);
	return buf;
}
char* to_str(char* buf, long v)
{
	sprintf(buf, "%ld", v);
	return buf;
}
char* to_str(char* buf, unsigned long v)
{
	sprintf(buf, "%lu", v);
	return buf;
}
char* to_str(char* buf, float v)
{
	sprintf(buf, "%.2f", v);
	return buf;
}
char* to_str(char* buf, double v)
{
	sprintf(buf, "%.2f", v);
	return buf;
}

std::string to_str(int v)
{
	char s[32];
	sprintf(s, "%d", v);
	return s;
}

std::string to_str(unsigned int v)
{
	char s[32];
	sprintf(s, "%u", v);
	return s;
}

std::string to_str(long v)
{
	char s[32];
	sprintf(s, "%ld", v);
	return s;
}

std::string to_str(unsigned long v)
{
	char s[32];
	sprintf(s, "%lu", v);
	return s;
}

std::string to_str(float v)
{
	char s[32];
	sprintf(s, "%.2f", v);
	return s;
}

std::string to_str(double v)
{
	char s[32];
	sprintf(s, "%.2f", v);
	return s;
}

bool
str_format(std::string& s, const char* fmt, ...)
{
	char* buf = NULL;
	int len = 0;
	bool ret = true;

	va_list vl;
	va_start( vl, fmt );

	len = ::_vscprintf(fmt, vl);

	if( len > 0)
	{
		buf = new char[len + 1];
		len = ::_vsnprintf( buf, len, fmt, vl );
		buf[len] = 0;
		s = buf;
		delete []buf;
	}
	else
	{
		s.clear();
		ret = false;
	}

	va_end(vl);
	return ret;
}

void 
str_trim_all(std::string& s, const char* k/* = "\t\n\f\v\b\r "*/)
{
	str_trim_left(s, k);
	str_trim_right(s, k);
}

void 
str_trim_left(std::string& s, const char* k/* = "\t\n\f\v\b\r "*/)
{
	if(!s.empty())
	{
		std::string::size_type pos = s.find_first_not_of(k);
		s.erase(0, pos);
	}
}

void 
str_trim_right(std::string& s, const char* k/* = "\t\n\f\v\b\r "*/)
{
	if(!s.empty())
	{
		std::string::size_type pos = s.find_last_not_of(k);
		if( std::string::npos == pos )
		{
			s = "";
		}
		else
		{
			s.erase(pos + 1);
		}
	}
}

std::string 
get_cur_time(int nType)
{
	char outstr[128];
	char str[100];

	time_t t; 
	struct tm  *area; 

	t = time(NULL); 
	area = localtime(&t); 

	memset(outstr,0x0,sizeof(outstr));

	if(1 == nType)
	{
		sprintf(str,"%4d",1900+area->tm_year);
		strcat(outstr,str);
		sprintf(str,"%02d",area->tm_mon+1);
		strcat(outstr,str);
		sprintf(str,"%02d",area->tm_mday);
		strcat(outstr,str);
		sprintf(str,"%02d",area->tm_hour);
		strcat(outstr,str);
		sprintf(str,"%02d",area->tm_min);
		strcat(outstr,str);
		sprintf(str,"%02d",area->tm_sec);
		strcat(outstr,str);
	}
	else if(3 == nType)
	{
		sprintf(str,"%4d",1900+area->tm_year);
		strcat(outstr,str);
		sprintf(str,"%02d",area->tm_mon+1);
		strcat(outstr,str);
		sprintf(str,"%02d ",area->tm_mday);
		strcat(outstr,str);    
	}
	else
	{
		sprintf(str,"%4d-",1900+area->tm_year);
		strcat(outstr,str);
		sprintf(str,"%02d-",area->tm_mon+1);
		strcat(outstr,str);
		sprintf(str,"%02d ",area->tm_mday);
		strcat(outstr,str);
		sprintf(str,"%02d:",area->tm_hour);
		strcat(outstr,str);
		sprintf(str,"%02d:",area->tm_min);
		strcat(outstr,str);
		sprintf(str,"%02d",area->tm_sec); 
		strcat(outstr,str);
	}

	return outstr;
}

bool 
create_guid(std::string& new_guid)
{
	// generated GUID
	GUID m_guid = GUID_NULL; 

	::CoCreateGuid(&m_guid);

	if ( m_guid == GUID_NULL )
	{
		return false;
	}

	char ret[BUFSIZ] = { 0 };

	_snprintf(ret, BUFSIZ, "%08lX-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X", m_guid.Data1, m_guid.Data2, m_guid.Data3, m_guid.Data4[0], m_guid.Data4[1], m_guid.Data4[2], m_guid.Data4[3], m_guid.Data4[4], m_guid.Data4[5], m_guid.Data4[6], m_guid.Data4[7]);

	new_guid = ret;
	return true;
}

void 
get_random_string(char* str, const unsigned int len)
{
	char* tStr = new char[len];
	memset(tStr, '\0', len);
	const char CCH[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	srand((unsigned)time(NULL));
	for (unsigned int i = 0; i < len; ++i)
	{
		int x = rand() % (sizeof(CCH) - 1);  
		tStr[i] = CCH[x];
	}
	strncpy(str, tStr, len);
	delete[] tStr;
}

////////////////////////////////////////////////////////////////////
//		random
////////////////////////////////////////////////////////////////////
static int RAND_SEED = 0x20091114;

#define RAND_NUMBER() (((RAND_SEED=RAND_SEED*214013L+2531011L) >>16) &0x7fff)

const char* random::string(char *buf, int len)
{
	static const char CS[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	for( int i=0; i<len; i++ )
	{
		buf[i] = CS[ RAND_NUMBER() % 36 ];
	}
	buf[len]=0;
	return buf;
}
int random::number(void)
{
	return RAND_NUMBER();
}
void random::seed(int n)
{
	RAND_SEED ^=n;
}

}// sp_ext