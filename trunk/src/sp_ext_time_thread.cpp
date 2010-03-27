#include "stdafx.h"
#include "../include/sp_ext_time_thread.h"
#include <windows.h>

namespace sp_ext
{
sp_ext_time_thread::sp_ext_time_thread()
{
	this->timer_id_mgr_.reset(100);
}

sp_ext_time_thread::~sp_ext_time_thread()
{
	
}
void 
sp_ext_time_thread::thread_method(worker_param_t_ptr work_para)
{
	while ( !this->exit_ )
	{
		Sleep(1000);
        this->time_list_lock_.lock();
		for( int n = 0; n < time_list_.GetSize(); n++ )
		{
			timer_obj_t_ptr  pTimeObject = (timer_obj_t_ptr) time_list_.GetAt(n);
			if( pTimeObject != NULL )
			{
				pTimeObject->time_base++;
				if( pTimeObject->time_base >= pTimeObject->time_limit ) // 到期
				{
					pTimeObject->time_base = 0;
					if ( pTimeObject->listener )
						pTimeObject->listener->on_time_notify(pTimeObject->time_id);
				}
			}
		}
		time_list_lock_.unlock();
	}
}

void 
sp_ext_time_thread::on_before_exiting() //在线程结束之前调用
{
	/// 删除所有的定时器
	timer_obj_t_ptr pTimeObject = NULL;
	time_list_lock_.lock();
	
	if( time_list_.GetSize()>0 )
	{
		pTimeObject = (timer_obj_t_ptr)time_list_.GetAt(0);
	}

	while ( pTimeObject != NULL )
	{
		this->timer_id_mgr_.recycle_id(pTimeObject->time_id);
		delete pTimeObject;
		pTimeObject  = NULL;
		time_list_.RemoveAt(0);
		if( time_list_.GetSize() > 0 )
		{
			pTimeObject = (timer_obj_t_ptr) time_list_.GetAt(0);
		}
	}
	time_list_lock_.unlock();
	exit_ = TRUE;
}
bool 
sp_ext_time_thread::before_start() //在创建线程之前调用，用来初始化配置
{
	exit_ = FALSE;
	return true;
}

BOOL 
sp_ext_time_thread::create()
{
	before_start();
    return this->create_threads();
}
BOOL 
sp_ext_time_thread::destroy()
{
	return this->destroy_threads();
}

int 
sp_ext_time_thread::set_new_timer(timer_listener* listener, int interval)
{
	if( listener == NULL || interval <= 0 )
	{
		return -1; //参数错误
	}
	timer_obj_t_ptr pTimeObject = new timer_obj_t;
	if( pTimeObject == NULL )
	{
		return -1; //内存溢出
	}
	pTimeObject->time_id = timer_id_mgr_.create_new_id();
	pTimeObject->time_base = 0;
	pTimeObject->time_limit = interval;
	pTimeObject->listener = listener;
	/// 分配一个TimeID
    time_list_lock_.lock();
	time_list_.Add((void*)pTimeObject);
	time_list_lock_.unlock();
	return pTimeObject->time_id;
}
void 
sp_ext_time_thread::kill_timer(int timer_id)
{
	time_list_lock_.lock();
	for( int n = 0; n < time_list_.GetSize(); n++ )
	{
		timer_obj_t_ptr pTimeObject = (timer_obj_t_ptr) time_list_.GetAt(n);
		if( pTimeObject != NULL )
		{
			if( pTimeObject->time_id == timer_id )
			{
				this->timer_id_mgr_.recycle_id(pTimeObject->time_id);
				time_list_.RemoveAt(n);
				delete pTimeObject;
				break;
			}
		}
	}
	time_list_lock_.unlock();
}

} // sp_ext