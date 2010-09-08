/** Copyright (c) 2008-2009
* All rights reserved.
* 
* 文件名称:	sp_ext_msg_queue_thread_pool.h
* 摘	 要:    消息队列线程池实现
* 
* 当前版本：	1.0
* 作	 者:	陈学术
* 操	 作:	新建
* 完成日期:    2009年3月28日
*/
#ifndef __SP_EXT_MSG_QUEUE_THREAD_POOL_H__
#define __SP_EXT_MSG_QUEUE_THREAD_POOL_H__

#include "sp_ext_thread_pool.h"
#include <queue>

namespace sp_ext
{
/**
     TMsgPack:must hase destroy() method, which is used to dispose.
 */
template<class TMsgPack, class TActualOwner, int DEFAULT_THREAD_COUNT = 20>
class msgqueue_worker_threads : public worker_threads<TActualOwner, DEFAULT_THREAD_COUNT>
{
public:
	msgqueue_worker_threads()
	{
		exit_ = FALSE;
		//msg_event_.reset_event(); // 开始没有信号，表示队列中没有东西
		msg_semaphore_.open(0, DEFAULT_THREAD_COUNT);
	}

	virtual ~msgqueue_worker_threads()
	{
		queue_mutex_.lock();
	
		while ( this->queue_.size() > 0 )
		{
			TMsgPack* pMsgPack = this->queue_.front();
			this->queue_.pop();
			if(pMsgPack != NULL)
			{
				delete pMsgPack;
			}
		}
		queue_mutex_.unlock();
	}

protected:
    /// 覆盖父类的方法
	virtual void thread_method(worker_param_t_ptr work_para)
	{
		TMsgPack* pMsgPack = NULL;

		while( !exit_ )
		{
			//if( msg_event_.wait_for_event_signaled() ) // 等待生产者信号
			if ( msg_semaphore_.wait() )
			{
				if( exit_ )
				{
					break;
				}
			}

			on_proc_msgqueue_start(work_para);

			while( !exit_ )
			{
				queue_mutex_.lock();

				if( this->queue_.size() <= 0 )
				{
					//msg_event_.reset_event();
					queue_mutex_.unlock();
					break;
				}

				pMsgPack = this->queue_.front();
				this->queue_.pop();

				queue_mutex_.unlock();

				if ( pMsgPack )
				{
					msg_proc(pMsgPack, work_para);
					//delete pMsgPack;
                    pMsgPack->destroy();
				}
			}
		}
	}

	virtual void on_before_exiting()
	{
		exit_ = TRUE;
		this->msg_semaphore_.post(this->thread_count_); // 放行所有线程函数
		//msg_event_.SetEvent();
	}

public:

	virtual BOOL create(int nThreadCount = DEFAULT_THREAD_COUNT)
	{
		return create_threads(nThreadCount);
	}

	virtual BOOL destroy()
	{
		return destroy_threads();
	}

	void push(TMsgPack* pMsgPack)
	{
		queue_mutex_.lock();
		this->queue_.push(pMsgPack);
		queue_mutex_.unlock();

		//msg_event_.set_event(); // 生产者信号
		this->msg_semaphore_.post();
	}

public:
	/// 在某个线程处理新消息前调用
	virtual void on_proc_msgqueue_start(worker_param_t_ptr work_para){}
	virtual void msg_proc(TMsgPack * pMsgPack, worker_param_t_ptr work_para) = 0;

protected:
	BOOL exit_;
	std::queue<TMsgPack*> queue_; /// 消息队列
	sp_ext::sp_ext_mutex queue_mutex_;
	//sp_ext::sp_ext_event msg_event_;
	sp_ext::semaphore msg_semaphore_;
};

} // sp_ext
#endif // __SP_EXT_MSG_QUEUE_THREAD_POOL_H__
