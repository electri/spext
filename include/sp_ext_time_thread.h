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

#include "sp_ext_define.h"
#include "sp_ext_thread_pool.h"
#include "sp_ext_id_recycle.h"
#include <set>

namespace sp_ext
{
////////////////////////////////////////////////////////////////////
//	usleep	"΢�뼶����" (�����֮һ��)
////////////////////////////////////////////////////////////////////
inline void usleep(int u_sec)
{
	DWORD milli_sec = (unsigned)u_sec /1000;
	::Sleep( milli_sec );
}

////////////////////////////////////////////////////////////////////
//		��ʱ��
//		HTIMER			"���"
//		timer_manager	"������"
////////////////////////////////////////////////////////////////////
struct HTIMER
{
	/// Ψһ��ID
	unsigned int id;
    
	/// ����ʱ��
	inet_time_t expire;

	/// ������
	struct CONTEXT
	{
		unsigned int number;
		void* pointer;
	} context;
	

	void (*callback)(HTIMER&);
	//�ص�����
};
typedef	void (TIMER_CALLBACK)(HTIMER&);

/*
	������timer��һ������������Ҹö��������timer����ǰ�ͱ��ͷţ�����ͨ������һ��������������ʵ��
	������������ã�����:
	HTIMER.context.p ָ���������
	�������ͷ�ʱ���������õĳ�ԱptrΪ��
	timer����ʱ������õ�ptr��ֻ�зǿ�ʱ�Żص������on_timer()����

	delay ��λ����
*/
inline void timer_set(HTIMER& timer, TIMER_CALLBACK* callback, uint32_t delay, inet_time_t now)
{
	inet_time_t* expire = &timer.expire;
	expire->sec = now.sec + delay/1000;
	expire->usec = now.usec + (delay%1000)*1000;
	if( expire->usec >1000000 )
	{
		expire->usec -=1000000;
		expire->sec++;
	}
	timer.callback = callback;
}

/// timer_manager
class timer_manager
{
	struct LESS_TIMER : public std::binary_function<HTIMER,HTIMER,bool> 
	{
		bool operator()(const HTIMER& x, const HTIMER& y) const
		{
			inet_time_t t1 = x.expire;
			inet_time_t t2 = y.expire;
			return t1.sec==t2.sec? (t1.usec==t2.usec? x.id<y.id : t1.usec<t2.usec) : t1.sec<t2.sec;
		}
	};
	typedef std::set<HTIMER,LESS_TIMER> timer_set;
	timer_set m_set;
	unsigned int m_new_id;

public:
	timer_manager(void): m_new_id(0)
	{
	}
	bool insert(HTIMER& timer);	//timer.id��������
	bool remove(HTIMER& timer);
	void clear(void);
	/*
	pull_expired: ȡ��һ�����ڵ�timer
	����:
	0 �ɹ�ȡ��һ��timer, ��ͨ��timer�������ص��ڵ�timer
	>0 ���һ��timer����Ҫ��õ���, ��λ(����)	1����=1/1000��
	-1 ��ǰû���κ�timer
	*/
	int pull_expired(inet_time_t now, HTIMER& timer);	
};

////////////////////////////////////////////////////////////////////
//		�;��ȶ�ʱ���߳�(��)
//		timer_listener  "��ʱ��������"
//		sp_ext_time_thread "��ʱ���߳���"
////////////////////////////////////////////////////////////////////

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

	/// ע��һ����ʱ����interval��λ:��
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
