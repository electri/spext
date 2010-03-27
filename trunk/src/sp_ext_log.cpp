#include "stdafx.h"
#include "../include/sp_ext_log.h"
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
namespace sp_ext
{ 
/* console_op */
console_op* console_op::instance_ = NULL; 

console_op::console_op()
{
	InitializeCriticalSection(&crit_sec_);
	open(300, 80);
}
console_op::~console_op()
{
	close();
	DeleteCriticalSection(&crit_sec_);
}

void 
console_op::open(long height, long width, const char* title)
{
	AllocConsole();
	if(title)
		SetConsoleTitle(title);

	HANDLE  hStd;
	int     fd;
	FILE    *file;

	/// 重定向标准输入流句柄到新的控制台窗口
	hStd = GetStdHandle(STD_INPUT_HANDLE);
	fd = _open_osfhandle(reinterpret_cast<long>(hStd), _O_TEXT); // 文本模式
	file = _fdopen(fd, "r");
	setvbuf(file, NULL, _IONBF, 0); // 无缓冲
	*stdin = *file;

	/// 重定向标准输出流句柄到新的控制台窗口
	hStd = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD size;
	size.X = (short)width;
	size.Y = (short)height;
	SetConsoleScreenBufferSize(hStd, size);
	fd = _open_osfhandle(reinterpret_cast<long>(hStd), _O_TEXT); //文本模式
	file = _fdopen(fd, "w");
	setvbuf(file, NULL, _IONBF, 0); // 无缓冲
	*stdout = *file;

	/// 重定向标准错误流句柄到新的控制台窗口
	hStd = GetStdHandle(STD_ERROR_HANDLE);
	fd = _open_osfhandle(reinterpret_cast<long>(hStd), _O_TEXT); // 文本模式
	file = _fdopen(fd, "w");
	setvbuf(file, NULL, _IONBF, 0); // 无缓冲
	*stderr = *file;
}

void 
console_op::close()
{
	FreeConsole();
}

/* sp_ext_log */
sp_ext_log::sp_ext_log()
{
    file_handle_ = INVALID_HANDLE_VALUE;
	level_ = e_log_enum_none;
	max_file_size_ = 100 * 1024;
	curr_file_size_ = 0;
	print_to_console_ = false;
}

sp_ext_log::~sp_ext_log()
{
    un_init_log();
}

BOOL 
sp_ext_log::init_log(int level,  bool print_to_console, const char* log_file/* = NULL*/)
{
	this->level_ = level;
	this->print_to_console_ = print_to_console;
    if( file_handle_ == INVALID_HANDLE_VALUE )
    {
        char fileName[512];
        memset(fileName, 0, 512);
       
		if ( NULL == log_file )
		{
			SYSTEMTIME systime;
		    GetLocalTime(&systime);
            sprintf(fileName,"Log-%d-%d-%d.log",
            systime.wYear, systime.wMonth, systime.wDay);
		}
		else
		{
			strncpy(fileName, log_file, sizeof(fileName));
		}

		file_name_ = fileName;

        file_handle_ = CreateFile(fileName, GENERIC_WRITE, FILE_SHARE_READ, NULL,
            OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        if( file_handle_ == INVALID_HANDLE_VALUE )
        {
            return FALSE;
        }

        curr_file_size_ = GetFileSize(file_handle_, NULL);

		if( ( this->curr_file_size_ / 1024 ) > this->max_file_size_ )  // 超过最大值删除
		{
			CloseHandle( file_handle_ );
			file_handle_ = INVALID_HANDLE_VALUE;
			
			::DeleteFile(fileName);
            
			file_handle_ = CreateFile(fileName, GENERIC_WRITE, FILE_SHARE_READ, NULL,
				OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			curr_file_size_ = 0;

		}

        SetFilePointer(file_handle_, 0, 0, FILE_END);
        InitializeCriticalSection( &crit_sec_ );

        return TRUE;
    }

    return FALSE;
}

BOOL 
sp_ext_log::un_init_log()
{
    if( file_handle_ != INVALID_HANDLE_VALUE )
    {
		write_log( this->level_, "\r\n\0\0", 2);
        
        DeleteCriticalSection(&crit_sec_);

        CloseHandle( file_handle_ );
        file_handle_ = INVALID_HANDLE_VALUE;
        return TRUE;
    }
    return FALSE;
}

void 
sp_ext_log::log(int log_level, const char* logStr)
{
	/*
	log形象模型：
				NONE
				ERR
	log_level-> DEBUG
				INFO <-level_
	*/
	if ( log_level > this->level_ ) 
	{
		return;
	}

	const char* log_pri = "INFO";
	switch ( log_level )
	{
	case e_log_enum_err:
        log_pri = "ERR";
		break;
	case e_log_enum_debug:
        log_pri = "DEBUG";
		break;
	case e_log_enum_warn:
		log_pri = "WARN";
		break;
	case e_log_enum_info:
        log_pri = "INFO";
		break;
	
	}

    SYSTEMTIME systime;
    char LogString[4096];
    memset(LogString, 0, 4096);
    GetLocalTime(&systime);
    sprintf(LogString,"[ %04d-%02d-%02d %02d:%02d:%02d %s] %s\r\n",
        systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond, log_pri, logStr);
    int Len = strlen(LogString);
    write_log(log_level, LogString, Len);

	/// 输出到控制台
	if ( print_to_console_ )
	{
		EnterCriticalSection(&console_op::instance()->crit_sec_);
		printf("%s", LogString);
		LeaveCriticalSection(&console_op::instance()->crit_sec_);
	}
}

void 
sp_ext_log::log_fmt(int log_level, const char* fmt, ...)
{
	if ( log_level > this->level_ ) 
	{
		return;
	}

    va_list ap;
    va_start(ap, fmt);
    char    str[4096];
    vsprintf(str, fmt, ap);
    va_end(ap);

    log(log_level, str);
}

void 
sp_ext_log::write_log(int log_level, const char* buf, int len)
{
	if( file_handle_ == INVALID_HANDLE_VALUE )
	{
		return;
   	}

    if ( log_level <= this->level_ )  // 小于等于当前级别才输出
	{
		EnterCriticalSection(&crit_sec_);

		if( ( this->curr_file_size_ / 1024 ) > this->max_file_size_ ) 
		{
			CloseHandle( file_handle_ );
			file_handle_ = INVALID_HANDLE_VALUE;

			::DeleteFile(file_name_.c_str());
			
			file_handle_ = CreateFile(this->file_name_.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL,
				OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			curr_file_size_ = 0;
		}

		DWORD realWrite = 0;
		WriteFile(file_handle_, buf, len, &realWrite, NULL);
		curr_file_size_ += realWrite;
		
		LeaveCriticalSection(&crit_sec_);
	}
}

}