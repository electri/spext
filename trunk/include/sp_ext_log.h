/** Copyright (c) 2008-2010
* All rights reserved.
* 
* 文件名称:	sp_ext_log.h     
* 摘	 要:	封装日志操作
* 
* 当前版本：	1.0
* 作	 者:	陈学术
* 操	 作:	新建
* 完成日期:	2010年03月22日
*/
#ifndef __SP_EXT_LOG_H__
#define __SP_EXT_LOG_H__

#include <windows.h>
#include <string>

namespace sp_ext
{ 
/** console_op
 *	将输出重定向到控制台(进程全局)
 */
class console_op
{
public:
	static console_op* console_op::instance()
	{
		if ( !instance_ )   
		{   
			instance_ = new console_op();    
		}   
		return instance_;   
	}

	void open(long height, long width, const char* title = 0);
	void close();
private:	
	console_op();
	virtual ~console_op();

	class destroy_helper // 它的唯一工作就是在析构函数中删除CSingleton的实例.   
	{   
	public:   
		~destroy_helper()   
		{   
			if ( console_op::instance_ )   
			{   
				delete console_op::instance_;   
				console_op::instance_ = NULL;
			}   
		}   
	};   

	static destroy_helper helper_; // 定义一个静态成员, 在程序结束时, 系统会调用它的析构函数

public:
	CRITICAL_SECTION crit_sec_;
private:
	static console_op* instance_; 
};

class sp_ext_log
{
public:
	enum e_log_enum
	{
		e_log_enum_none = 0, /// 不打日志
		// 客户自能用以下枚举
		e_log_enum_err,/// 错误
        e_log_enum_debug,/// 调试
		e_log_enum_warn,/// 警告
		e_log_enum_info,/// 信息

	};
public:
    sp_ext_log();
    virtual ~sp_ext_log();
public:
    BOOL init_log(int level, bool print_to_console = false, const char* log_file = NULL);
    BOOL un_init_log();
public:
    void log(int log_level, const char* logStr);
    void log_fmt(int log_level, const char* fmt,...);
protected:
    void write_log(int log_level, const char* buf,int len);
protected:
    HANDLE file_handle_;
    CRITICAL_SECTION crit_sec_;
	int level_;
	int curr_file_size_; // 单位Byte
	int max_file_size_; // 最大日志文件大小,单位K,最大100*1024k
	std::string file_name_;
	bool print_to_console_; // 打印到控制台

};
}/// namespace sp_ext
#endif /// __SP_EXT_LOG_H__
