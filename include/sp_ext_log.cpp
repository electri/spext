//#include "stdafx.h"
#include "sp_ext_log.h"
#include <stdio.h>
#include <io.h>
#include <fcntl.h>

#define SP_EXT_LOG_OFF 0 // 是否关闭日志, 2011-2-22

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
	max_file_size_ = max_file_size;
	curr_file_size_ = 0;
	print_to_console_ = false;
}

sp_ext_log::~sp_ext_log()
{
    un_init_log();
}

bool 
sp_ext_log::find_file(const char* file_name)
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

std::string 
sp_ext_log::get_curr_date_str(int try_index)
{
    char buf[255] = {0x0};
    std::string ret;
    SYSTEMTIME systime;
    GetLocalTime(&systime);

    /*
    if ( 0 == try_index   )
    {
        sprintf(buf, "%4d-%02d-%02d", systime.wYear, systime.wMonth, systime.wDay);
    }
    else
    */
    {
        sprintf(buf, "%4d-%02d-%02d(%02d)", systime.wYear, systime.wMonth, systime.wDay, try_index);
    }
    ret = buf;
    return ret;
}

BOOL 
sp_ext_log::init_log(int level,  bool print_to_console, int max_size, const char* log_file/* = NULL*/)
{
#if SP_EXT_LOG_OFF
    return false; // 禁止日志
#endif

	this->level_ = level;
	this->print_to_console_ = print_to_console;
    this->max_file_size_ = max_size;


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
			strncpy(fileName, log_file, sizeof(fileName)-1);
            fileName[sizeof(fileName)-1] = '\0';
		}

		file_name_ = fileName;

        file_handle_ = CreateFile(fileName, GENERIC_WRITE, FILE_SHARE_READ, NULL,
            OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        if( file_handle_ == INVALID_HANDLE_VALUE )
        {
            return FALSE;
        }

        curr_file_size_ = GetFileSize(file_handle_, NULL);

		if( ( (float)this->curr_file_size_ / 1024.0 ) > this->max_file_size_ )  // 超过最大值重命名
		{
			CloseHandle( file_handle_ );
			file_handle_ = INVALID_HANDLE_VALUE;

			//::DeleteFile(fileName);
            this->rename_logfile();
            
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
#if SP_EXT_LOG_OFF
    return false; // 禁止日志
#endif
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
sp_ext_log::log(int log_level, const char* log_str)
{
#if SP_EXT_LOG_OFF
    return; // 禁止日志
#endif
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

    int len_src = strlen(log_str);

    if ( len_src < 1 || len_src > max_buf_size-1 )
    {
        return;
    }

	const char* log_pri = get_level_pri(log_level);

    SYSTEMTIME systime;
    char final_buf[max_buf_size + 100];
      
    GetLocalTime(&systime);
    sprintf(final_buf,"[ %04d-%02d-%02d %02d:%02d:%02d %s (%d)] %s\r\n",
        systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond, log_pri, GetCurrentThreadId(), log_str);
    final_buf[sizeof(final_buf)-1] = '\0';
    int len_dst = strlen(final_buf);
    write_log(log_level, final_buf, len_dst);

	/// 输出到控制台
	if ( print_to_console_ )
	{
		EnterCriticalSection(&console_op::instance()->crit_sec_);
		printf("%s", final_buf);
		LeaveCriticalSection(&console_op::instance()->crit_sec_);
	}
}

bool 
sp_ext_log::log_fmt(int log_level, const char* fmt, ...)
{
#if SP_EXT_LOG_OFF
    return true; // 禁止日志
#endif
	if ( log_level > this->level_ ) 
	{
		return false;
	}

    char buf[max_buf_size];
    int len = 0;
    bool too_long = false; 

    va_list vl;
    va_start( vl, fmt );
    len = ::_vsnprintf( buf, sizeof(buf)-1, fmt, vl );
    va_end(vl);

    buf[sizeof(buf)-1] = 0;

    //if( len == 0 || len > sizeof(buf)-1 )
    if( len == 0 )
        return false;

    too_long = ( len > sizeof(buf)-1 );

    /*
    if the number of characters to write is greater than count, these functions return -1 indicating that output has been truncated.
    if (  len <= -1 ) /// 
    {

    }
    */
   
    const char* log_pri = get_level_pri(log_level);
    char buf_pre[512];
    SYSTEMTIME systime;
    GetLocalTime(&systime);
    sprintf(buf_pre, "[ %04d-%02d-%02d %02d:%02d:%02d %s (%d)]", systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond, log_pri, GetCurrentThreadId());
   
    char final_buf[max_buf_size + 100];
    if ( !too_long )
    {
        sprintf(final_buf, "%s %s\r\n", buf_pre, buf);
    }
    else
    {
        sprintf(final_buf, "%s %s\r\n...............................\r\n", buf_pre, buf);
    }
    final_buf[sizeof(final_buf)-1] = '\0';
    int final_len = strlen(final_buf);
    write_log(log_level, final_buf, final_len);

    /// 输出到控制台
    if ( print_to_console_ )
    {
        EnterCriticalSection(&console_op::instance()->crit_sec_);
        printf("%s", final_buf);
        LeaveCriticalSection(&console_op::instance()->crit_sec_);
    }
    return too_long;
}

const char* 
sp_ext_log::get_level_pri(int log_level)
{
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
    return log_pri;
}

void 
sp_ext_log::rename_logfile()
{
    std::string rename_filename;
    int try_index = 0;
    do 
    {
        rename_filename = file_name_ + "-" + get_curr_date_str(try_index);
        try_index++;

    } while ( find_file(rename_filename.c_str()) );

    MoveFile(this->file_name_.c_str(), rename_filename.c_str());
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

		if( ( (float)this->curr_file_size_ / 1024.0 ) > this->max_file_size_ ) 
		{
			CloseHandle( file_handle_ );
			file_handle_ = INVALID_HANDLE_VALUE;

			//::DeleteFile(file_name_.c_str());
            this->rename_logfile();
			
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