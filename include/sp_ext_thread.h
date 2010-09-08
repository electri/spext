/** Copyright (c) 2008-2010
* All rights reserved.
* 
* �ļ�����:	sp_ext_thread.h     
* ժ	 Ҫ:	��װ���̺߳�ͬ������
* 
* ��ǰ�汾��	1.0
* ��	 ��:	��ѧ��
* ��	 ��:	�½�
* �������:	2010��03��24��
*/
#ifndef __SP_EXT_THREAD_H__ 
#define __SP_EXT_THREAD_H__

#include <Windows.h>
#include <process.h>

namespace sp_ext
{

/// �뿪�������ִ��ĳ��callback����
struct exit_function_t			
{
	typedef void (function_t)(void*);
	function_t& func_;
	void *ctx_;

	exit_function_t( function_t& func, void *ctx )
		:func_(func), ctx_(ctx)
	{
	}

	~exit_function_t(void)
	{
		this->func_(this->ctx_);
	}
};

///	sp_ext_null_lock "����" ֻ��Ϊģ�����
class sp_ext_null_lock
{
public:
    bool open() { return true; }
    bool close() { return true; }
	bool lock() { return true; }
	bool unlock() { return true; }
	bool try_lock() { return true; }
};



/// �������װ
class sp_ext_mutex  
{
public:
	sp_ext_mutex()
	{
		InitializeCriticalSection(&metux_);         //�̻߳������
	}

	virtual ~sp_ext_mutex()
	{
		DeleteCriticalSection(&metux_);         //�̻߳������
	}

    bool open()
    {
        return true;
    }

    bool close() { return true; }

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
    bool try_lock() { return true; }
private:
	CRITICAL_SECTION    metux_;         //�̻߳������
};

typedef sp_ext_mutex thread_mutex;

//������
class process_mutex
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
    typedef HANDLE mutex_type;

    process_mutex() : mutex_(0)
    {;}
    ~process_mutex() { }

    //inline int type() const { return mutex_flag::e_process; }

    inline bool open(const char* name = NULL)
    {
        mutex_ = ::CreateMutex(NULL, FALSE, name);
        return (NULL != mutex_);
    }

    inline bool close()
    {
        if( mutex_ )
        {
            ::CloseHandle(mutex_);
            mutex_ = NULL;
        }
        return true;
    }

    inline bool lock()/// ������
    {
        switch (::WaitForSingleObject (mutex_, INFINITE))
        {
        case WAIT_OBJECT_0:
        case WAIT_ABANDONED:
            return true;
        default:
            return false;
        }
    }

    /// ������, tm: ��ʱʱ�䵥λ:ms
    inline int lock(long tm)
    {
        switch (::WaitForSingleObject (mutex_, tm))
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

    inline bool try_lock()//�������Ĳ�����
    {
        return 0 == lock(0);
    }

    inline bool unlock()//�ͷ���
    {
        if( mutex_ )
        {
            ::ReleaseMutex(mutex_);
        }
        return true;
    }

    inline mutex_type* mutex()
    {
        return &mutex_;
    }
private:
    mutex_type mutex_;
private:
    process_mutex& operator = (const process_mutex&);
    process_mutex (const process_mutex&);
};

//�Զ���
//MUTEX Ϊ: null_mutex, thread_mutex, process_mutex
template<class MUTEX>
class auto_lock
{
public:
    auto_lock(MUTEX& t) : lock_(t)
    {
        lock_.lock();
    }
    ~auto_lock()
    {
        lock_.unlock();
    }
private:
    MUTEX& lock_;
};

typedef auto_lock<sp_ext_mutex> thread_auto_lock;
typedef auto_lock<process_mutex> process_auto_lock;

/// �źŵ�
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

	//��ʼ���ں˶���
	//nInitCnt: ��ʼ��������Դ��
	//nMaxWaiters: ��������Դ��
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

	bool close()//�ͷ��ں˶���
	{
		if( m_hSema )
		{
			bool b = (TRUE == ::CloseHandle(m_hSema)); 
			m_hSema = NULL;
			return b;
		}
		return true;
	}

	/// �����ȴ�.�߳̽�������,ֱ��������Դ��(���ź���)>0, 
	/// ע��: ����ж���߳�ͬʱ����(��ͬʱ�ȴ���һ���źŵ���),
	/// ��һ��post����ʱ, ͬһʱ��,ֻ��һ���̱߳�����
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

	/// �ȴ�, tm:��ʱֵ
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

	//����һ���ź���
	bool post()
	{
		if( m_hSema )
		{
			return TRUE == ::ReleaseSemaphore(m_hSema, 1, NULL);
		}
		return false;
	}

	//����n���ź���
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


/// �¼���װ
class sp_ext_event  
{
public:
	sp_ext_event()
	{
		event_handle_ = ::CreateEvent(NULL, TRUE, FALSE, NULL); // �˹�
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

//��������, �����ĳ�����͵���һ��ʹ��, �Եȴ�ĳ���������ʽ
//MUTEX Ϊ: null_mutex, thread_mutex, process_mutex
template<class MUTEX>
class condition
{
public:
	class  cond_t
	{
	public:
	  bool open()
	  {
		  m_waiters = 0;
		  was_broadcast = 0;
		  if(!m_sema.open())
			  return false;
		  //if(!m_waiters_lock.open())
		  //	  return false;

		  m_waiters_done = ::CreateEvent(NULL,
							   FALSE,
							   FALSE,
							   NULL);

		  if(NULL == m_waiters_done)
			  return false;
		  
		  return true;
	  }

	  bool close()
	  {
		  if(m_waiters_done)
		  {
			::CloseHandle(m_waiters_done);
		  }

		  m_waiters = 0;
		  was_broadcast = 0;

		  m_waiters_done = NULL;
		  
		  //m_waiters_lock.close();
		  return m_sema.close();
	  }

	public:
	  long m_waiters;
	  MUTEX m_waiters_lock;
	  semaphore m_sema;
	  HANDLE m_waiters_done;
	  size_t was_broadcast;
	};
public:
	condition() { }
	~condition() { }
public:
	bool open(MUTEX* mt)
	{
		m_mutex = mt;
		return m_cond.open();
	}

	bool close()
	{
		bool b = true;
		//�ͷ��ں˶���ʱ, Ҫ�������й�����߳�
		while(!m_cond.close() && b)
		{
			b = broadcast();
		}
		return b;
	}

	MUTEX* mutex()
	{
		return m_mutex;
	}

	size_t waiters_num(){ return m_cond.m_waiters;}

	//�����ȴ�.�߳̽�������,ֱ��������Դ��(���ź���)>0, 
	//ע��: ����ж���߳�ͬʱ����(��ͬʱ�ȴ���һ���ں˶�����),
	//��һ��signal����ʱ, ͬһʱ��,ֻ��һ���̱߳�����
	//��: �ⲿ����waitǰ,�����ȼ���, wait����ʱ,���۽�����, ��Ȼ����
	//�ⲿ��
	bool wait()
	{
		m_cond.m_waiters_lock.lock();
		m_cond.m_waiters++;
		m_cond.m_waiters_lock.unlock();
		//���ͷ���, ����ⲿ����waitǰ,�����ȼ���
		if(!m_mutex->unlock())
			return false;
		//����,�ȴ�
		bool b = m_cond.m_sema.wait();

		m_cond.m_waiters_lock.lock();
		m_cond.m_waiters--;
		bool last_waiter = m_cond.was_broadcast && m_cond.m_waiters == 0;
		m_cond.m_waiters_lock.unlock();

		if(b && last_waiter)
		{	
			::SetEvent(m_cond.m_waiters_done);	
		}
		//���»�ȡ�ⲿ��
		m_mutex->lock();

		return b;
	}

	//�����ȴ�.�߳̽�������,ֱ��������Դ��(���ź���)>0, 
	//ע��: ����ж���߳�ͬʱ����(��ͬʱ�ȴ���һ���ں˶�����),
	//��һ��signal����ʱ, ͬһʱ��,ֻ��һ���̱߳�����
	//��: �ⲿ����waitǰ,�����ȼ���, wait����ʱ,���۽�����, ��Ȼ����
	//�ⲿ��
	int wait(long tm)
	{
		m_cond.m_waiters_lock.lock();
		m_cond.m_waiters++;
		m_cond.m_waiters_lock.unlock();

		if(!m_mutex->unlock())
			return err_code::e_unknow;

		int ret = m_cond.m_sema.wait(tm);

		m_cond.m_waiters_lock.lock();
		m_cond.m_waiters--;
		bool last_waiter = m_cond.was_broadcast && m_cond.m_waiters == 0;
		m_cond.m_waiters_lock.unlock();

		if(ret == err_code::e_ok && last_waiter)
		{	
			//�ж��Ƿ������һ���ȴ���, �����,�������¼�,
			//��������broadcast���߳̽�������
			::SetEvent(m_cond.m_waiters_done);	
		}

		m_mutex->lock();

		return ret;
	}
	//����һ���ź�,����һ����wait������߳�
	bool signal ()
	{	
		m_cond.m_waiters_lock.lock();
		int waiters = m_cond.m_waiters;
		m_cond.m_waiters_lock.unlock();
		
		if (waiters > 0)
			return m_cond.m_sema.post();
		else
			return true; 
	}
	//�㲥,�������б�wait������߳�
	bool broadcast ()
	{
		m_cond.m_waiters_lock.lock();
		
		int have_waiters = 0;
		
		if (m_cond.m_waiters > 0)
		{
			m_cond.was_broadcast = 1;
			have_waiters = 1;
		}
		m_cond.m_waiters_lock.unlock();
		
		bool bRet = true;
		if (have_waiters)
		{
			if (!m_cond.m_sema.post(m_cond.m_waiters))
			{
				return false;
			}

			switch (::WaitForSingleObject (m_cond.m_waiters_done, INFINITE))
			{
			case WAIT_OBJECT_0:
				bRet = true;
				break;

			default:
				bRet = false;
				break;
			}

			m_cond.was_broadcast = 0;
		}
		return bRet;
		
	}
	
private:
	MUTEX* m_mutex;
	cond_t m_cond;
private:
	condition& operator = (const condition&);
	condition (const condition&);
};

typedef condition<sp_ext_mutex> thread_condition;//�߳���������
typedef condition<process_mutex> process_condition;//�߳���������

///դ��
///MUTEX Ϊ: null_mutex, thread_mutex, process_mutex
template<class MUTEX>
class barrier
{
    typedef auto_lock<MUTEX> auto_lock_type;
public:
    //nWaitersNum: ��Ҫͬ�����߳���
    barrier(int nWaitersNum)
    {
        m_init_num = nWaitersNum;
        m_run_num = nWaitersNum;
        m_mutex.open();
        m_cond.open(&m_mutex);
    }

    ~barrier()
    {
        m_cond.close();
        m_mutex.close();
    }

    bool wait()
    {
        auto_lock_type __tmp_lock(m_mutex);

        if(m_run_num == 1)
        {//���һ��������߳�: �㲥,�������й�����դ���ϵ��߳�
            m_run_num = m_init_num;
            return m_cond.broadcast();
        }
        else
        {
            --m_run_num;
            while(m_run_num != m_init_num)
            {
                if(!m_cond.wait())
                    return false;
            }

            return true;
        }

        return true;
    }
private:
    condition<MUTEX> m_cond;
    MUTEX m_mutex;
    int m_init_num;
    int m_run_num;
};

typedef barrier<sp_ext_mutex> thread_barrier;

/// ��ʱ��
class time_waiter
{
public:
    enum e_tm_ret
    {
        e_tm_ret_time_out = 0, // ������ʱ
        e_tm_ret_break,        // ���ⲿ�ж�
    };
public:
    time_waiter(int tm = 1000) // ����
    {
        event_ = NULL;
        event_ = ::CreateEvent(NULL, FALSE, FALSE, NULL);
        this->time_out_ = tm; 
    }

    virtual ~time_waiter()
    {
        if ( event_ )
            ::CloseHandle(event_);
        event_ = NULL;
    }
    int wait()
    {
        return wait(this->time_out_);
    }

    int wait(int tm) // ����
    {
        DWORD dwWait;
        ResetEvent(event_);
        dwWait = ::WaitForSingleObject(event_, tm);
        if( dwWait == WAIT_TIMEOUT )
            return e_tm_ret_time_out;
        return e_tm_ret_break;
    }
    void stop() // �ڲ�ͬ��wait�߳��е���
    {
        ::SetEvent(event_);
    }
private:
    /// none copyable
    time_waiter(const time_waiter&) {}
    time_waiter& operator=(const time_waiter&) {}
    HANDLE event_;
    int time_out_; // millisecond
};


/// ���̷߳�װ
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
            /// ����"�����"�����߳�
            if (running_ == TRUE)
                running_ = FALSE;        //first, try to stop the thread nice

            GetExitCodeThread(thread_handle_, &exit_code_);

            if ( exit_code_ == STILL_ACTIVE && bForceKill )
            {
				/// ǿ��ɱ���߳�
                TerminateThread(thread_handle_, DWORD(-1));
                thread_handle_ = NULL;
            }
        }

        return exit_code_;
    }

    DWORD stop_timeout( WORD timeout )
    {
        stop(false);
        WaitForSingleObject(thread_handle_, timeout);//�ȴ�һ��ʱ��
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

   /// ������д�˷���
    virtual DWORD thread_method() = 0;

private:
    static unsigned int WINAPI entry_point(LPVOID pArg)
    {
        sp_ext_thread * pParent = reinterpret_cast<sp_ext_thread*>(pArg);
        pParent->thread_method();//��̬�ԣ����������ʵ�ʹ�������
        return 0;
    }

protected:
    HANDLE thread_handle_; 
    unsigned int thread_id_;
    LPVOID thread_parent_;
    DWORD exit_code_;

    // LPTHREAD_START_ROUTINE thread_function_;
    thread_fun_t thread_function_;
	BOOL running_;
};

} //namesapce sp_ext


#endif //__SP_EXT_THREAD_H__