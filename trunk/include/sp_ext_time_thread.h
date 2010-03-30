/** Copyright (c) 2008-2010
* All rights reserved.
* 
* 文件名称:	sp_ext_time_thread.h
* 摘	 要:    定时器线程
* 
* 当前版本：	1.0
* 作	 者:	陈学术
* 操	 作:	新建
* 完成日期:    2009年3月28日
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
//	usleep	"微秒级休眠" (百万分之一秒)
////////////////////////////////////////////////////////////////////
inline void usleep(int u_sec)
{
	DWORD milli_sec = (unsigned)u_sec /1000;
	::Sleep( milli_sec );
}

////////////////////////////////////////////////////////////////////
//		定时器
//		HTIMER			"句柄"
//		timer_manager	"管理器"
////////////////////////////////////////////////////////////////////
struct HTIMER
{
	/// 唯一的ID
	unsigned int id;
    
	/// 到期时间
	inet_time_t expire;

	/// 上下文
	struct CONTEXT
	{
		unsigned int number;
		void* pointer;
	} context;
	

	void (*callback)(HTIMER&);
	//回调函数
};
typedef	void (TIMER_CALLBACK)(HTIMER&);

/*
	如果多个timer和一个对象关联，且该对象可能在timer到期前就被释放，可以通过设立一个单独的引用来实现
	解除关联的作用，例如:
	HTIMER.context.p 指向对象引用
	当对象释放时，设置引用的成员ptr为空
	timer到期时检查引用的ptr，只有非空时才回调对象的on_timer()方法

	delay 单位毫秒
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
	bool insert(HTIMER& timer);	//timer.id将被设置
	bool remove(HTIMER& timer);
	void clear(void);
	/*
	pull_expired: 取出一个到期的timer
	返回:
	0 成功取出一个timer, 并通过timer参数返回到期的timer
	>0 最近一个timer还需要多久到期, 单位(毫秒)	1毫秒=1/1000秒
	-1 当前没有任何timer
	*/
	int pull_expired(inet_time_t now, HTIMER& timer);	
};

////////////////////////////////////////////////////////////////////
//		低精度定时器线程(秒)
//		timer_listener  "定时器监听者"
//		sp_ext_time_thread "定时器线程类"
////////////////////////////////////////////////////////////////////

/// 定时器监听者
class timer_listener  
{
public:
	timer_listener(){};
	virtual ~timer_listener(){};
	virtual  int on_time_notify(int time_id) = 0;
};

/// 定时器线程
class sp_ext_time_thread  : public worker_threads<sp_ext_time_thread,1>
{
	typedef struct timer_obj_t
	{
		int time_id;
		UINT time_base; // 计时时间，单位秒
		UINT time_limit; // 间隔时间
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
	virtual void on_before_exiting(); /// 在线程结束之前调用
	virtual bool before_start(); /// 在创建线程之前调用，用来初始化配置
public:	
	/// 以下函数为用户调用
	BOOL create(); /// 开启定时器线程
	BOOL destroy(); /// 关闭定时器线程

	/// 注册一个定时器，interval单位:秒
	int set_new_timer(timer_listener* listener, int interval);
	void kill_timer(int timer_id);
private:
	BOOL exit_; /// 线程退出标志
	sp_ext::id_recycle timer_id_mgr_;
	sp_ext::ptr_array time_list_;
	sp_ext::sp_ext_mutex time_list_lock_;
};

} // sp_ext

#endif // __SP_EXT_TIME_THREAD_H__
