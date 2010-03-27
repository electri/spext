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

#include "sp_ext_thread_pool.h"
#include "sp_ext_id_recycle.h"

namespace sp_ext
{
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

	/// 注册一个定时器，interval单位是秒
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
