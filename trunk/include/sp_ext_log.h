/** Copyright (c) 2008-2010
* All rights reserved.
* 
* �ļ�����:	sp_ext_log.h     
* ժ	 Ҫ:	��װ��־����
* 
* ��ǰ�汾��	1.0
* ��	 ��:	��ѧ��
* ��	 ��:	�½�
* �������:	2010��03��22��
*/
#ifndef __SP_EXT_LOG_H__
#define __SP_EXT_LOG_H__

#include <windows.h>
#include <string>

namespace sp_ext
{ 
/** console_op
 *	������ض��򵽿���̨(����ȫ��)
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

	class destroy_helper // ����Ψһ��������������������ɾ��CSingleton��ʵ��.   
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

	static destroy_helper helper_; // ����һ����̬��Ա, �ڳ������ʱ, ϵͳ�����������������

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
		e_log_enum_none = 0, /// ������־
		// �ͻ�����������ö��
		e_log_enum_err,/// ����
        e_log_enum_debug,/// ����
		e_log_enum_warn,/// ����
		e_log_enum_info,/// ��Ϣ

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
	int curr_file_size_; // ��λByte
	int max_file_size_; // �����־�ļ���С,��λK,���100*1024k
	std::string file_name_;
	bool print_to_console_; // ��ӡ������̨

};
}/// namespace sp_ext
#endif /// __SP_EXT_LOG_H__
