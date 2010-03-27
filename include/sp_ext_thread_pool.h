/** Copyright (c) 2008-2010
* All rights reserved.
* 
* 文件名称:	sp_ext_thread_pool.h
* 摘	 要:    普通线程池实现
* 
* 当前版本：	1.0
* 作	 者:	陈学术
* 操	 作:	新建
* 完成日期:    2009年3月28日
*/
#ifndef __SP_EXT_THREAD_POOL_H__ 
#define __SP_EXT_THREAD_POOL_H__

#include <Windows.h>
#include <process.h>

namespace sp_ext
{

template<class T, int DEFAULT_THREAD_COUNT = 20>
class worker_threads  
{
protected:
	typedef  unsigned int (__stdcall * thread_fun_t)(void *);
public:
	worker_threads()
	{
		worker_handles_	= NULL;
		worker_parameters_	= NULL;
		thread_count_		= 0;
	}

	virtual ~worker_threads()
	{
		destroy_threads();
	}

	/// <summary>create worker threads</summary>
	/// <param name="nThreadCount">threads to be created</param>
	/// <returns>TRUE if create successfully</returns>
	inline BOOL create_threads(int nThreadCount = DEFAULT_THREAD_COUNT)
	{
		destroy_threads();
		thread_count_ = nThreadCount ;
		
		// create thread handle array
		worker_handles_ = new HANDLE[nThreadCount];
		
		if(worker_handles_ == NULL)
			return FALSE;
		
		// create thread parameter array
		worker_parameters_ = new worker_param_t[nThreadCount];
		
		if(worker_parameters_ == NULL)
			return FALSE;
		
		//create the thread one by one
		for( int i = 0; i < nThreadCount; i++ )
		{
			// thread parameter
			worker_parameters_[i].parent = (T*)this;
			worker_parameters_[i].result = 0;
			worker_parameters_[i].worker_id = i;

			/*
			worker_handles_[i] = CreateThread(
				NULL,
				0,
				worker_thread_proc,
				(PVOID)&worker_parameters_[i],
				0,
				NULL);
			*/

			worker_handles_[i] = (HANDLE)_beginthreadex( NULL, 0, worker_thread_proc, (PVOID)&worker_parameters_[i], 0, &worker_parameters_[i].thread_id );
			
			/* if(worker_handles_ == INVALID_HANDLE_VALUE) */ 
			if( worker_handles_[i] == NULL ) 
			{
				break;
			}
		}
		
		return TRUE;
	}
	
	/// <summary>destroy all threads</summary>
	/// <returns>TRUE if destroy successfully</returns>
	inline BOOL destroy_threads()
	{	
		if( worker_handles_ == NULL && worker_parameters_ == NULL )
		{
			return TRUE;
		}
		
		on_before_exiting();
		
		wait_threads_exit();
		
		for(int i=0; i<thread_count_; i++)
		{
			CloseHandle(worker_handles_[i]);
		}

		/// free thread handles
		if( worker_handles_ != NULL )
		{
			delete [] worker_handles_;
			worker_handles_ = NULL;
		}
		
		if( worker_parameters_ != NULL )
		{
			delete [] worker_parameters_;
			worker_parameters_ = NULL;
		}
		
		thread_count_ = 0;
		
		return TRUE;
	}
	
	/// <summary>Used for pass parameters to each worker thread</summary>
	typedef struct _WORKER_PARAMETER
	{
		int worker_id;
		unsigned int thread_id; // 线程id 
		//worker_threads* parent;
		T* parent;
		int result;
	}worker_param_t, *worker_param_t_ptr;
	
protected:
	/// <summary>below is the thread procedure in both Windows & Linux</summary>
	/// <param name="lpParameter">the parameter passed to thread</param>
	/// <returns>the thread return code</returns>
	/// windows version
	static  unsigned int WINAPI worker_thread_proc(LPVOID lpParameter)
	{
		worker_param_t_ptr pWorderParameter = (worker_param_t_ptr)lpParameter;
		worker_threads *worker = pWorderParameter->parent;

		if(!worker->on_work_start(pWorderParameter))
			return 0;

		worker->thread_method(pWorderParameter);
		
		if(!worker->on_work_end(pWorderParameter))
			return 0;
		return (DWORD)lpParameter;
	}
protected:
	virtual BOOL on_work_start(worker_param_t_ptr work_para)
	{
		return TRUE;
	}
	virtual BOOL on_work_end(worker_param_t_ptr work_para)
	{
		return TRUE;
	}

	//<summary>wait until all the threads exit</summary>
	void wait_threads_exit()
	{
		WaitForMultipleObjects(thread_count_, worker_handles_, TRUE, INFINITE);
	}

protected:
	/// 继承类覆盖此方法
	virtual void thread_method(worker_param_t_ptr work_para) = 0;
	virtual void on_before_exiting(){};
protected:
	HANDLE* worker_handles_;  /// 线程句柄数组
	int thread_count_;
	worker_param_t* worker_parameters_; /// 线程参数数组
};

///    worker_threads_ex,比worker_threads实用
template<class T, int DEFAULT_THREAD_COUNT = 20>
class worker_threads_ex  : public worker_threads<T, DEFAULT_THREAD_COUNT>
{
public:
	worker_threads_ex()
	{
		exit_ = FALSE;
	}

	virtual ~worker_threads_ex()
	{

	}

	virtual BOOL create(int nThreadCount = DEFAULT_THREAD_COUNT)
	{
		return create_threads(nThreadCount);
	}

	virtual BOOL destroy()
	{
		return destroy_threads();
	}

	virtual void on_before_exiting()
	{
		exit_ = TRUE;
	}
protected:
	BOOL exit_;
};

} // sp_ext


#endif // __SP_EXT_THREAD_POOL_H__
