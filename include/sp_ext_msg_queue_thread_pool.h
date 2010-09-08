/** Copyright (c) 2008-2009
* All rights reserved.
* 
* �ļ�����:	sp_ext_msg_queue_thread_pool.h
* ժ	 Ҫ:    ��Ϣ�����̳߳�ʵ��
* 
* ��ǰ�汾��	1.0
* ��	 ��:	��ѧ��
* ��	 ��:	�½�
* �������:    2009��3��28��
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
		//msg_event_.reset_event(); // ��ʼû���źţ���ʾ������û�ж���
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
    /// ���Ǹ���ķ���
	virtual void thread_method(worker_param_t_ptr work_para)
	{
		TMsgPack* pMsgPack = NULL;

		while( !exit_ )
		{
			//if( msg_event_.wait_for_event_signaled() ) // �ȴ��������ź�
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
		this->msg_semaphore_.post(this->thread_count_); // ���������̺߳���
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

		//msg_event_.set_event(); // �������ź�
		this->msg_semaphore_.post();
	}

public:
	/// ��ĳ���̴߳�������Ϣǰ����
	virtual void on_proc_msgqueue_start(worker_param_t_ptr work_para){}
	virtual void msg_proc(TMsgPack * pMsgPack, worker_param_t_ptr work_para) = 0;

protected:
	BOOL exit_;
	std::queue<TMsgPack*> queue_; /// ��Ϣ����
	sp_ext::sp_ext_mutex queue_mutex_;
	//sp_ext::sp_ext_event msg_event_;
	sp_ext::semaphore msg_semaphore_;
};

} // sp_ext
#endif // __SP_EXT_MSG_QUEUE_THREAD_POOL_H__
