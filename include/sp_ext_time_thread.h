/** Copyright (c) 2008-2010
* All rights reserved.
* 
* �ļ�����:	sp_ext_time_thread.h
* ժ	 Ҫ:    ��ʱ���߳�
* 
* ��ǰ�汾��	1.0
* ��	 ��:	��ѧ��
* ��	 ��:	�½�
* �������:    2009��3��28��
*/
#ifndef __SP_EXT_TIME_THREAD_H__ 
#define __SP_EXT_TIME_THREAD_H__

#include "sp_ext_thread_pool.h"
#include "sp_ext_id_recycle.h"

namespace sp_ext
{
/// ��ʱ��������
class timer_listener  
{
public:
	timer_listener(){};
	virtual ~timer_listener(){};
	virtual  int on_time_notify(int time_id) = 0;
};

/// ��ʱ���߳�
class sp_ext_time_thread  : public worker_threads<sp_ext_time_thread,1>
{
	typedef struct timer_obj_t
	{
		int time_id;
		UINT time_base; // ��ʱʱ�䣬��λ��
		UINT time_limit; // ���ʱ��
		timer_listener* listener;
		timer_obj_t()
		{
			time_id = -1;
			time_base = 0;
			time_limit = 0;
			listener = NULL;
		}
	}timer_obj_t, *timer_obj_t_ptr;
public:
	sp_ext_time_thread();
	virtual ~sp_ext_time_thread();
	virtual void thread_method(worker_param_t_ptr work_para);
	virtual void on_before_exiting(); /// ���߳̽���֮ǰ����
	virtual bool before_start(); /// �ڴ����߳�֮ǰ���ã�������ʼ������
public:	
	/// ���º���Ϊ�û�����
	BOOL create(); /// ������ʱ���߳�
	BOOL destroy(); /// �رն�ʱ���߳�

	/// ע��һ����ʱ����interval��λ����
	int set_new_timer(timer_listener* listener, int interval);
	void kill_timer(int timer_id);
private:
	BOOL exit_; /// �߳��˳���־
	sp_ext::id_recycle timer_id_mgr_;
	sp_ext::ptr_array time_list_;
	sp_ext::sp_ext_mutex time_list_lock_;
};

} // sp_ext

#endif // __SP_EXT_TIME_THREAD_H__
