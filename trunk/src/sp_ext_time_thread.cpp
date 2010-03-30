#include "stdafx.h"
#include "../include/sp_ext_time_thread.h"
#include <windows.h>

namespace sp_ext
{

////////////////////////////////////////////////
//			timer_manager
////////////////////////////////////////////////
bool timer_manager::insert(HTIMER& timer)
{
	timer.id = ++m_new_id;
	std::pair<timer_set::iterator,bool> ret = m_set.insert(timer);
	return ret.second;
}


int timer_manager::insert(TIMER_CALLBACK* callback, uint32_t delay, HTIMER* ret)
{
	HTIMER a_timer;
	sp_ext::inet_time_t now;
	sp_ext::get_timeofday(&now);
	sp_ext::timer_set(a_timer, callback, delay, now);
	if ( this->insert(a_timer) )
	{
		if ( ret )
		{
			*ret = a_timer;
		}
		return a_timer.id;
	}

	return 0;
}

bool timer_manager::remove(HTIMER& timer)
{
	timer_set::iterator it = m_set.find(timer);
	if( it==m_set.end() )
		return false;
	m_set.erase(it);
	return true;
}
#if 0	//太低效了，所以废弃
void timer_manager::remove_if( bool (*can_remove)(const HTIMER& timer,long context), long context )
{
	for( timer_set::iterator it=m_set.begin();	it!=m_set.end(); )
	{
		if( can_remove( *it, context ) )
		{
			m_set.erase( it++ );
		}
		else
		{
			it++;
		}
	}
}
#endif
int timer_manager::pull_expired(HTIMER& timer)
{
	if( m_set.empty() )
	{
		return -1;	//没有任何定时器
	}
	sp_ext::inet_time_t now;
	sp_ext::get_timeofday(&now);

	timer_set::iterator it = m_set.begin();
	if( it->expire < now )
	{	//到期了, 返回timer并移除记录
		timer = *it;
		m_set.erase(it);
		return 0;
	}

	//还未到期，计算等待时间(in 毫秒)
	inet_time_t interval = it->expire - now;
	int ms = interval.sec*1000 + interval.usec/1000;
	return ms>0 ? ms : 1;
}
void timer_manager::clear(void)
{
	m_set.clear();
}

////////////////////////////////////////////////
//			sp_ext_time_thread
////////////////////////////////////////////////
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