/** Copyright (c) 2008-2010
* All rights reserved.
* 
* 文件名称:	sp_ext_thread.h     
* 摘	 要:	封装单线程和同步对象
* 
* 当前版本：	1.0
* 作	 者:	陈学术
* 操	 作:	新建
* 完成日期:	2010年03月24日
*/
#ifndef __SP_EXT_THREAD_H__ 
#define __SP_EXT_THREAD_H__

#include <Windows.h>
#include <process.h>

namespace sp_ext
{
/// 互斥体封装
class sp_ext_mutex  
{
public:
	sp_ext_mutex()
	{
		InitializeCriticalSection(&metux_);         //线程互斥对象
	}

	virtual ~sp_ext_mutex()
	{
		DeleteCriticalSection(&metux_);         //线程互斥对象
	}

	bool lock()
	{
		EnterCriticalSection(&metux_);
		return true;
	}

    bool unlock()
    {
        LeaveCriticalSection(&metux_);
        return true;
    }
private:
	CRITICAL_SECTION    metux_;         //线程互斥对象
};


/// 堆栈锁，析构时自动解锁，便于使用
class auto_lock
{
public:
    auto_lock(sp_ext_mutex& mutex)
        : mutex_(mutex)
    {
        mutex_.lock();
    }

    ~auto_lock()
    {
        mutex_.unlock();
    }	

protected:
    sp_ext_mutex& mutex_;
};

/// 信号灯
class semaphore
{
public:
	enum
	{
		e_ok = 0,
		e_time_out = -1,
		e_unknow = -2,
		e_shut_down = -3
	};
	
public:
	semaphore() 
	{
		m_hSema = NULL;
		m_nMaxWaiters = 0;	
	};

	~semaphore() { ;}

	//初始化内核对象
	//nInitCnt: 初始化可用资源数
	//nMaxWaiters: 最大可用资源数
	bool open(long nInitCnt = 0, long nMaxWaiters = 0x7fffffff)
	{
		m_hSema = ::CreateSemaphore(NULL, nInitCnt, nMaxWaiters, NULL);

		if (m_hSema == NULL) 
		{
			return false;
		}
		m_nMaxWaiters = nMaxWaiters;
		return true;
	}

	bool close()//释放内核对象
	{
		if( m_hSema )
		{
			bool b = (TRUE == ::CloseHandle(m_hSema)); 
			m_hSema = NULL;
			return b;
		}
		return true;
	}

	/// 阻塞等待.线程将被挂起,直到可用资源数(即信号量)>0, 
	/// 注意: 如果有多个线程同时挂起(即同时等待在一个信号灯上),
	/// 当一个post到达时, 同一时刻,只有一个线程被唤醒
	bool wait() 
	{ 
		switch (::WaitForSingleObject (m_hSema, INFINITE))
		{
		case WAIT_OBJECT_0:
		case WAIT_ABANDONED:
			return true;
		default:
			return false;
		}
	}

	/// 等待, tm:超时值
	int wait(long tm)
	{
		if(tm < 0)
		{
			tm = INFINITE;
		}
		switch (::WaitForSingleObject (m_hSema, tm))
		{
		case WAIT_OBJECT_0:
		case WAIT_ABANDONED:
			return 0;
		case WAIT_TIMEOUT:
			return e_time_out;
		default:
			return e_unknow;
		}
	}

	//增加一个信号量
	bool post()
	{
		if( m_hSema )
		{
			return TRUE == ::ReleaseSemaphore(m_hSema, 1, NULL);
		}
		return false;
	}

	//增加n个信号量
	bool post(long n)
	{
		if( m_hSema )
		{
			return TRUE == ::ReleaseSemaphore(m_hSema, n, NULL);
		}
		return false;
	}

private:
	HANDLE	m_hSema;
	long  m_nMaxWaiters;
private:
	semaphore& operator = (const semaphore&);
	semaphore (const semaphore&);
};


/// 事件封装
class sp_ext_event  
{
public:
	sp_ext_event()
	{
		event_handle_ = ::CreateEvent(NULL, TRUE, FALSE, NULL); // 人工
	}

	virtual ~sp_ext_event()
	{
		CloseHandle(event_handle_);
	}

	bool wait_for_event_signaled()
	{
		bool ret = false;
		if( WAIT_OBJECT_0 == WaitForSingleObject(event_handle_, INFINITE) )
			ret = true;

		return ret;
	}

	bool set_event()
	{
		bool ret = false;
		ret = ( TRUE == ::SetEvent(event_handle_) );
		return ret;
	}

	bool reset_event()
	{
		bool    ret = false;
		ret = ( TRUE == ::ResetEvent(event_handle_) );
		return ret;
	}
private:
	HANDLE  event_handle_;
};

/// 单线程封装
class sp_ext_thread
{
protected:
	typedef  unsigned int (__stdcall * thread_fun_t)(void *);
public:
    sp_ext_thread()
    { 
        thread_function_ = sp_ext_thread::entry_point;
        running_ = FALSE;
    }

    virtual ~sp_ext_thread()
    {
        if ( thread_handle_ )
            stop(true);                    //thread still running, so force the thread to stop!
    }
   
    DWORD start(DWORD dwCreationFlags = 0)
    {
        running_ = true;
        //thread_handle_ = CreateThread(NULL, 0, thread_function_, this, 0, &dwCreationFlags);
        thread_handle_ = (HANDLE)_beginthreadex( NULL, 0, thread_function_, this, 0, &this->thread_id_ );
		exit_code_ = (DWORD)-1;

        return GetLastError();
    }
    
    DWORD stop ( bool bForceKill = false )
    {
        if ( thread_handle_ )
        {
            /// 尝试"温柔地"结束线程
            if (running_ == TRUE)
                running_ = FALSE;        //first, try to stop the thread nice

            GetExitCodeThread(thread_handle_, &exit_code_);

            if ( exit_code_ == STILL_ACTIVE && bForceKill )
            {
				/// 强制杀死线程
                TerminateThread(thread_handle_, DWORD(-1));
                thread_handle_ = NULL;
            }
        }

        return exit_code_;
    }

    DWORD stop( WORD timeout )
    {
        stop(false);
        WaitForSingleObject(thread_handle_, timeout);//等待一段时间
        return stop(true);
    }
    
    DWORD suspend()
    {
        return SuspendThread(thread_handle_);
    }

    DWORD resume()
    {
        return ResumeThread(thread_handle_);
    }

    BOOL set_priority(int priority)
    {
        return SetThreadPriority(thread_handle_, priority);
    }

    int get_priority()
    {
        return GetThreadPriority(thread_handle_);
    }

protected:

   /// 子类重写此方法
    virtual DWORD thread_method() = 0;

private:
    static unsigned int WINAPI entry_point(LPVOID pArg)
    {
        sp_ext_thread * pParent = reinterpret_cast<sp_ext_thread*>(pArg);
        pParent->thread_method();//多态性，调用子类的实际工作函数
        return 0;
    }

private:
    HANDLE thread_handle_; 
    unsigned int thread_id_;
    LPVOID thread_parent_;
    DWORD exit_code_;

protected:
    // LPTHREAD_START_ROUTINE thread_function_;
    thread_fun_t thread_function_;
	BOOL running_;
};

} //namesapce sp_ext


#endif //__SP_EXT_THREAD_H__