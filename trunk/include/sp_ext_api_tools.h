/** Copyright (c) 2008-2010
* All rights reserved.
* 
* �ļ�����:		api_tools.h  
* ժ	 Ҫ:	һЩ�ǳ����õĹ��ߺ���
* 
* ��ǰ�汾��	1.0
* ��	 ��:	��ѧ��
* ��	 ��:	�½�
* �������:		2009��3��24��
*/

#ifndef __SP_EXT_API_TOOLS_H__
#define __SP_EXT_API_TOOLS_H__

#include <string>
#include <windows.h>

namespace sp_ext
{ 

/**
 *	ȡ��ģ������·��
 */
std::string	get_module_path(HMODULE h);

/**
 *	ȡexe����������·��, ��β���� \
 */
std::string	get_app_path();

/**
 *	��֤buf[buf_size - 1] ����Ϊ'\0'
 */
int safe_snprintf(char* buf, int buf_size, const char* fmt, ...);

/** 
 *	if( strlen(src) >= n) , ������n���ַ��� ����dest[n] = 0, ����Ӧ��: n < dest�Ļ���������
 *	 ����ʵ�ʿ�������Ч�ַ���������'\0')
 */
size_t safe_strncpy(char* dest, const char* src, size_t n);

/// �ж�Ŀ¼�Ƿ����
bool dir_exist(const char *path);

/// ����Ŀ¼, @path ĩβ��'\\'
bool dir_create(const char * path);

/// ɾ��Ŀ¼(������Ŀ¼���ļ�)
bool dir_delete(const char* dir);

bool del_all_files(const char* dir, bool enter_sub_dir = true);

/// ����ɾ���ļ������ɶ��ļ�
bool delete_file_ex(const char* file_name);

/// ��ȡ�ļ���С (�ֽ�)
long get_file_size(const char* file_name); 

/// �����ļ���Ŀ¼�Ƿ����  Ŀ¼ �� ������:/Ŀ¼��, �ļ� �� ������:/Ŀ¼��/�ļ���
bool find_file(const char* file_name);

enum e_win_ver
{
	e_win_ver_95 = 0,
	e_win_ver_98,
	e_win_ver_MELLINNIUM,
	e_win_ver_NT,
	e_win_ver_2000,
	e_win_ver_XP,
	e_win_ver_SERVER2003,
	e_win_ver_VISTA,
	e_win_ver_7,
};
//�жϵ�ǰ�û�ʹ�õĲ���ϵͳ����
e_win_ver get_windows_ver();

/*	����str�����һ�� ==key ���ַ�, ���ظ�λ�õ�ָ��, ���򷵻�NULL
 *	���ָ����len, ���str[len-1]��ʼ�������, Ч�ʿ��ܻ����
 */
char* str_find_lastof(char* str, char key, int len = -1);
/// ͬ��
const char* str_find_lastof(const char* str, char key, int len = -1);

/**	�������str, ���key�е�����һ���ַ���str�г���, �򷵻ظ�λ�õ�ָ��, ���򷵻�NULL
 *	���ָ����len, ���str[len-1]��ʼ�������, Ч�ʿ��ܻ����
 */
char* str_find_lastof(char* str, const char* key, int len = -1);
/// ͬ��	
const char* str_find_lastof(const char* str, const char* key, int len = -1);

std::string to_str(int v);
std::string to_str(unsigned int v);
std::string to_str(long v);
std::string to_str(unsigned long v);
std::string to_str(float v);
std::string to_str(double v);

char* to_str(char* buf, int v);
char* to_str(char* buf, unsigned int v);
char* to_str(char* buf, long v);
char* to_str(char* buf, unsigned long v);
char* to_str(char* buf, float v);
char* to_str(char* buf, double v);

bool str_format(std::string& s, const char* fmt, ...);

void str_trim_all(std::string& s, const char* k = "\t\n\f\v\b\r ");
void str_trim_left(std::string& s, const char* k = "\t\n\f\v\b\r ");
void str_trim_right(std::string& s, const char* k = "\t\n\f\v\b\r ");


/**
 *  ��ȡ��ǰʱ��
 *	nType == 1: YYYYMMDDhhmmss
 *	nType == 2: YYYY-MM-DD hh:mm:ss
 *	nType == 3: YYYYMMDD
 */
std::string get_cur_time(int nType);

/// ����һ��GUID
bool create_guid(std::string& new_guid);

/// ��ȡ�����
void get_random_string(char* str, const unsigned int len);


}// sp_ext






#endif // __SP_EXT_API_TOOLS_H__