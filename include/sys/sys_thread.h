/** Copyright (c) 2008 
* All rights reserved.
* 
* 文件名称:		sys_thread.h
* 摘	 要:	提供一些线程的基本操作
* 
* 当前版本：	1.0
* 作	 者:	范涛涛
* 操	 作:	新建
* 完成日期:		2008年5月14日
*/

#ifndef __SYS_THREAD__
#define __SYS_THREAD__

#include "sys_globa_def.h"
#include "sys_mm_pool.h"
#include "sys_asy.h"
#include "sys_queue.h"

#include <memory>
#include <list>


#pragma warning(disable:4482)

namespace xy
{
	//线程局部储存, VAL_TYPE: 值类型
	template<class VAL_TYPE>
	class thread_local_storage
	{
		typedef VAL_TYPE val_type;
	public:
		thread_local_storage() { m_nIndex = -1; }

		//初始化
		bool open()
		{
			m_nIndex = ::TlsAlloc();
			return m_nIndex != -1;
		}
		//释放
		bool close()
		{
			if(m_nIndex != -1)
			{
				::TlsFree(m_nIndex);
				m_nIndex = -1;
			}
			
			return true;
		}
		//设置值
		bool set_value(const val_type& val) const
		{
			assert(m_nIndex != -1);
			return TRUE == ::TlsSetValue(m_nIndex, static_cast<LPVOID>(val));
		}
		//获取值
		val_type get_value()
		{
			assert(m_nIndex != -1);
			return static_cast<val_type>(::TlsGetValue(m_nIndex));
		}
	private:
		long m_nIndex;
	};

	//线程句柄
	class thread_handle
	{
	public:
		typedef HANDLE handle_type;
		typedef DWORD id_type;
	public:
		thread_handle(handle_type h, id_type id)
			: m_th_handle(h), m_th_id(id)
		{
		}

		thread_handle(id_type id)
			: m_th_handle(0), m_th_id(id)
		{
		}

		thread_handle() 
			: m_th_handle(0), m_th_id(0)
		{
		}

		thread_handle(const thread_handle& other)
		{
			m_th_handle = other.m_th_handle;
			m_th_id = other.m_th_id;
		}

		inline void close()
		{
			if(m_th_handle)
			{
				::CloseHandle(m_th_handle);
				m_th_handle = 0;
			}
			m_th_id = 0;
		}
		inline handle_type handle() const { return m_th_handle; }

		inline void handle(handle_type h){m_th_handle = h;}

		inline id_type id() const {return m_th_id;}
		inline void id(id_type th_id){m_th_id = th_id;}

		bool operator== (const thread_handle& other) const
		{
			return (m_th_handle == other.m_th_handle) && 
				(m_th_id == other.m_th_id);
		}

		bool operator!= (const thread_handle& other) const
		{
			return (m_th_handle != other.m_th_handle) || 
				(m_th_id != other.m_th_id);
		}
	private:
		handle_type m_th_handle;
		id_type m_th_id;
	};


	//线程操作
	class thread_op
	{
	public:

		enum E_PRIORITY{e_low=0, e_normal, e_high};	
		typedef int (*THREAD_FUN)(void*);
		typedef int (*THREAD_HOOK)(void*);
		
		struct __thread_arg
		{
			THREAD_FUN m__thread_entry_point;
			void* m_arg;

			__thread_arg(THREAD_FUN f, void* a)
				: m__thread_entry_point(f), m_arg(a)
			{	
			}
		};

	public:
		//创建一个线程
		static bool spawn(THREAD_FUN th_fun,
						  void* arg=0, 
						  thread_handle* phandle=0,
						  long flag = 0, 
						  long priority = E_PRIORITY::e_normal )
		{
			thread_handle th;
			thread_handle::id_type id;

			th.handle(::CreateThread(NULL,
				0, 
				thread_op::__thread_fun,
				(LPVOID)(new __thread_arg(th_fun, arg)),
				flag,
				&id));

			if (0 == th.handle())
			{
				return false;
			}
			th.id(id);

			if(E_PRIORITY::e_normal != priority)
			{
				set_priority(&th, priority);
			}

			if(phandle)
			{
				*phandle = th;
			}
			else
			{
				th.close();
			}
			return true;
		}

		//创建N个线程, 返回实际创建的线程数
		static int spawn_n(THREAD_FUN th_fun,
						  void* arg=0,
						  int num = 1,
						  long flag = 0, 
						  long priority = -1,
						  thread_handle th_handle[] = 0)
		{
			int i=0;
			for(; i < num; ++i)
			{
				if(!spawn(th_fun, 
							arg, 
							th_handle == 0 ? 0 : &th_handle[i], 
							flag, 
							priority))
					break;
			}
			return i;
		}

		//创建N个线程, 返回实际创建的线程数
		template <class std_contain>
		static int spawn_n(std_contain& contain,
						  THREAD_FUN th_fun,
						  void* arg=0,
						  int num = 1,
						  long flag = 0, 
						  long priority = -1)
		{
			int i=0;
			thread_handle th;
			for(; i < num; ++i)
			{
				if(!spawn(th_fun, 
							arg, 
							&th, 
							flag, 
							priority))
					break;

				contain.push_back(th);
			}
			return i;
		}
		//设置优先级
		static bool set_priority(thread_handle* th_h, int prio)
		{
			int n;
			switch(prio) 
			{
			case E_PRIORITY::e_low:
				n = THREAD_PRIORITY_BELOW_NORMAL;
				break;
			case E_PRIORITY::e_high:
				n = THREAD_PRIORITY_ABOVE_NORMAL;
				break;
			case E_PRIORITY::e_normal:
			default:
				n = THREAD_PRIORITY_NORMAL;
			}

			return THREAD_PRIORITY_ERROR_RETURN != ::SetThreadPriority(th_h->handle(), n);
		}
		//获取优先级
		static bool get_priority(thread_handle* th_h, int& prio)
		{
			switch(::GetThreadPriority(th_h->handle())) 
			{
			case THREAD_PRIORITY_ERROR_RETURN:
				return false;
			case THREAD_PRIORITY_BELOW_NORMAL:
				prio = E_PRIORITY::e_low;
				break;
			case THREAD_PRIORITY_ABOVE_NORMAL:
				prio = E_PRIORITY::e_high;
				break;
			case THREAD_PRIORITY_NORMAL:
			default:
				prio = E_PRIORITY::e_normal;
				break;
			}

			return true;
		}
		//等待线程退出
		static bool wait(thread_handle* th_h)
		{
			if(th_h && th_h->handle() != NULL)
			{
				if(::WaitForSingleObject(th_h->handle(), INFINITE) != WAIT_OBJECT_0) 
					return false;
			}
			
			return true;
		}
		//等待线程退出
		static bool wait(thread_handle th_handle[], int num)
		{
			if(num < 1) return true;
		
			for(int i=0; i<num; ++i)
			{
				wait(&th_handle[i]);
			}
			
			return true;
		}
		//等待线程退出. std_contain: stl容器
		template <class std_contain>
		static bool wait(std_contain& contain)
		{
			if(contain.size() < 1) return true;
		
			const int MAX_WAIT = 62;
			int i = 0;
			thread_handle::handle_type v_th[MAX_WAIT];
			std_contain::iterator iter = contain.begin();

			for(; iter != contain.end(); ++iter, ++i)
			{
				v_th[i] = iter->handle();
				if(i == MAX_WAIT - 1)
				{
					::WaitForMultipleObjects(MAX_WAIT, v_th, TRUE, INFINITE);
				}
				i = 0;
			}

			if(i > 0)
				::WaitForMultipleObjects(i, v_th, TRUE, INFINITE);
			
			return true;
		}
		//获取当前线程描述符
		static thread_handle self()
		{
			return thread_handle(::GetCurrentThreadId());
		}
		//终止线程
		static bool kill(thread_handle* th_h)
		{
			return TRUE == ::TerminateThread(th_h->handle(), -1);
		}
		//放弃时间片,强制切换线程
		static void yield(){ ::Sleep(0);}

		
	private:
		//所有线程统一的入口函数
		static DWORD WINAPI  __thread_fun(void* arg)
		{
			std::auto_ptr<__thread_arg> param_ptr((__thread_arg*)arg);
			int ret = param_ptr->m__thread_entry_point(param_ptr->m_arg);
			return ret;
		}

	};


	class run_able
	{
	public:
		virtual ~run_able(){;}
	protected:
		virtual int svc() = 0;
	};

	//线程基类
	class thread_base : public run_able
	{
	public:
		thread_base(int priority= thread_op::E_PRIORITY::e_normal)
		{
			m_is_shut_down = true;
			m_priority = priority;
			m_sema_exit.open();
		}


		virtual ~thread_base()
		{
			m_thread_handle.close();
			m_sema_exit.close();
		}
		
		virtual void shut_down()
		{
			m_is_shut_down = true;
		}
		virtual bool is_shut_down()
		{
			return m_is_shut_down;
		}
		//等待线程退出
		virtual int wait(long tm = -1)
		{
			//bool ret = thread_op::wait(&m_thread_handle);
			if(m_thread_handle.handle() != NULL)
			{
				m_thread_handle.close();
				return m_sema_exit.wait(tm);
			}
			
			return err_code::e_unknow;
		}
		//激活线程
		virtual int active()
		{
			m_is_shut_down = false;
			return thread_op::spawn(thread_base::__thread_fun, 
								 (void*)this, 
								 &m_thread_handle,
								 0,
								 m_priority) ? 1: 0;
		}
		//获取线程描述符
		thread_handle& get_thread_handle() {return m_thread_handle;}
	protected:
		//线程函数
		virtual int svc() = 0;
	protected:
		thread_base& operator= (const thread_base &);
		thread_base (const thread_base &);

	private:
		static int  __thread_fun(void* param)
		{
			thread_base* p = (thread_base*)param;
			int ret = p->svc();
			p->m_sema_exit.post();
			return ret;
		}
	protected:
		thread_handle m_thread_handle;
		semaphore m_sema_exit;
		int m_priority;
		bool m_is_shut_down;

	};


	//线程服务类, 也是一个简单的线程池
	template<class SYN_TRAITS, class T>
	class thread_svc : public thread_base
	{
	public:
		typedef typename thread_svc<SYN_TRAITS, T> this_type;
		typedef typename SYN_TRAITS::THREAD_MUTEX mutex_type;
		typedef typename SYN_TRAITS::THREAD_CONDITION condition_type;
		typedef typename SYN_TRAITS::SEMAPHORE semaphore_type;
		typedef auto_lock<mutex_type> auto_lock_type;
		typedef xy::queue<SYN_TRAITS, T> queue_type;

		typedef auto_lock<mutex_type> auto_lock_type;
	public:
		thread_svc(size_t th_num, //线程数
						size_t queue_high_water = queue_type::e_high_water, //队列高水位
						size_t queue_low_water = 0,//队列低水位
						long priority = thread_op::E_PRIORITY::e_normal)
			: m_queue(queue_high_water, queue_low_water)
		{
			assert(th_num > 0);
			assert(queue_high_water > 0);

			m_active = 0;
			m_th_num = th_num;
			m_priority = priority;
			m_run_num = 0;
			m_sema_exit.open();
			m_mutex.open();
		}

		//默认构造函数, 随后需要通过open来设置参数
		thread_svc()
		{
			m_active = 0;
			m_th_num = 0;
			m_priority = thread_op::E_PRIORITY::e_normal;
			m_run_num = 0;
			m_sema_exit.open();
			m_mutex.open();
		}
		bool open(size_t th_num, 
					size_t queue_high_water = queue_type::e_high_water,
					size_t queue_low_water = 0,//队列低水位
					long priority = thread_op::E_PRIORITY::e_normal)
		{
			assert(is_exit_all());
			assert(th_num > 0);
			assert(queue_high_water > 0);

			if(!is_exit_all())
				return false;

			m_queue.high_water(queue_high_water);
			m_queue.low_water(queue_low_water);
			m_th_num = (int)th_num;
			m_priority = priority;
			m_run_num = 0;

			return true;
		}

		virtual ~thread_svc()
		{
			if(!is_shut_down())
			{
				shut_down();
				wait();
			}
			m_sema_exit.close();
			m_mutex.close();
		}
		

		//激活线程池, 返回实际创建的线程数
		virtual int active()
		{
			assert(is_exit_all());
			if(!is_exit_all())
				return 0;
			m_queue.active();
			
			
			wait();
			
			m_run_num = thread_op::spawn_n(thread_svc::__thread_fun, 
									 (void*)this, 
									 m_th_num,
									 0,
									 m_priority);

			m_active = m_run_num;
			return m_run_num;
		}

		//关闭线程池
		virtual void shut_down()
		{
			if(m_queue.status() != queue_type::e_shutdown)
			{
				m_queue.shut_down();
			}
		}

		virtual int wait(long tm = -1)
		{
			int ret = err_code::e_ok;
			if(m_active > 0)
			{
				ret = m_sema_exit.wait(tm);
				__clear();
			}
			return ret;
		}
		//判断线程池是否已经关闭
		virtual bool is_shut_down(){return m_queue.is_shut_down();}
		//判断池中所有线程是否已经完全退出
		virtual bool is_exit_all()
		{
			return is_shut_down() && (m_run_num == 0);
		}
		//消息入队
		int putq(T& val, long tm = -1)
		{
			return m_queue.enqueue(val, tm);
		}
		//出队
		int getq(T& out, long tm = -1)
		{
			return m_queue.dequeue(out, tm);
		}
		//
		template<class _Ty> 
		bool removeq(_Ty _Pred)
		{

			return m_queue._remove(_Pred);
		}

		//获取消息队列指针
		queue_type* msg_queue(){return &m_queue;}

	protected:
		//线程函数
		virtual int svc() = 0;
		
		
	private:
		static int __thread_fun(void* arg)
		{
			this_type* pthis = (this_type*)arg;
			int ret = pthis->svc();
			pthis->__dec_thread_cnt();
			return ret;
		}

		void __clear()
		{
			m_run_num = 0;
			m_active = 0;
		}

		size_t __dec_thread_cnt()
		{
			auto_lock_type __mon(m_mutex);
			--m_run_num;

			if((int)m_run_num <= 0)
			{
				m_sema_exit.post();
			}
	
			return m_run_num;
		}
	protected:
		int m_th_num;
		int m_run_num;
		long m_priority;
		queue_type m_queue;
		semaphore_type m_sema_exit;
		mutex_type m_mutex;
		int m_active;
	};
	
};

#pragma warning(default:4482)

#endif /// __SYS_THREAD__