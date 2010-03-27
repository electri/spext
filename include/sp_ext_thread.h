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
	CRITICAL_SECTION    metux_;         //�̻߳������
};


/// ��ջ��������ʱ�Զ�����������ʹ��
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

    DWORD stop( WORD timeout )
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