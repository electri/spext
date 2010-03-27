/** Copyright (c) 2008-2010
* All rights reserved.
* 
* 文件名称:		api_tools.h  
* 摘	 要:	一些非常有用的工具函数
* 
* 当前版本：	1.0
* 作	 者:	陈学术
* 操	 作:	新建
* 完成日期:		2009年3月24日
*/

#ifndef __SP_EXT_API_TOOLS_H__
#define __SP_EXT_API_TOOLS_H__

#include <string>
#include <windows.h>

namespace sp_ext
{ 

/**
 *	取该模块所在路径
 */
std::string	get_module_path(HMODULE h);

/**
 *	取exe主程序所在路径, 结尾包含 \
 */
std::string	get_app_path();

/**
 *	保证buf[buf_size - 1] 总是为'\0'
 */
int safe_snprintf(char* buf, int buf_size, const char* fmt, ...);

/** 
 *	if( strlen(src) >= n) , 将拷贝n个字符， 并且dest[n] = 0, 所以应该: n < dest的缓冲区长度
 *	 返回实际拷贝的有效字符数（不含'\0')
 */
size_t safe_strncpy(char* dest, const char* src, size_t n);

/// 判断目录是否存在
bool dir_exist(const char *path);

/// 创建目录, @path 末尾加'\\'
bool dir_create(const char * path);

/// 删除目录(包括子目录及文件)
bool dir_delete(const char* dir);

bool del_all_files(const char* dir, bool enter_sub_dir = true);

/// 彻底删除文件包括可读文件
bool delete_file_ex(const char* file_name);

/// 获取文件大小 (字节)
long get_file_size(const char* file_name); 

/// 查找文件、目录是否存在  目录 － 分区名:/目录名, 文件 － 分区名:/目录名/文件名
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
//判断当前用户使用的操作系统类型
e_win_ver get_windows_ver();

/*	查找str中最后一个 ==key 的字符, 返回该位置的指针, 否则返回NULL
 *	如果指定了len, 则从str[len-1]开始反向查找, 效率可能会更高
 */
char* str_find_lastof(char* str, char key, int len = -1);
/// 同上
const char* str_find_lastof(const char* str, char key, int len = -1);

/**	反向查找str, 如果key中的任意一个字符在str中出现, 则返回该位置的指针, 否则返回NULL
 *	如果指定了len, 则从str[len-1]开始反向查找, 效率可能会更高
 */
char* str_find_lastof(char* str, const char* key, int len = -1);
/// 同上	
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
 *  获取当前时间
 *	nType == 1: YYYYMMDDhhmmss
 *	nType == 2: YYYY-MM-DD hh:mm:ss
 *	nType == 3: YYYYMMDD
 */
std::string get_cur_time(int nType);

/// 创建一个GUID
bool create_guid(std::string& new_guid);

/// 获取随机串
void get_random_string(char* str, const unsigned int len);


}// sp_ext






#endif // __SP_EXT_API_TOOLS_H__