// test_sp_ext_base_class.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include  "sp_ext_boyermor.h"
#include "sp_ext_id_recycle.h"
#include "sp_ext_thread_pool.h"
#include "sp_ext_time_thread.h"
#include <string>
#include <stdio.h>
#include "sp_ext_msg_queue_thread_pool.h"
#include "sp_ext_obj_recycle.h"
#include "sp_ext_hash_table.h"
#include "sp_ext_string_util.h"

/// 测试boyer算法
void test_boyermoore()
{
	std::string strToFind	=	"birthday"; 
	std::string	strSrc		=	"   dfhdfffdkfdkfjdfjf birthday is :2009-03-14";
	sp_ext::BoyerMoore moore( strToFind.c_str() );
	LONG value = moore.GetNumberAfter( strSrc.c_str(), strSrc.size(), 0 );
	LONG date  = moore.GetDateAfter( strSrc.c_str(), strSrc.size(), 0 );

	printf("number is %d, birthday number is:%d", value, date);
	/* 运行结果：number is 2009, birthday number is:20090314 */
	getchar();
}

/// 测试id recycle
class use_id_thread : public sp_ext::sp_ext_thread
{
public:
	use_id_thread(sp_ext::id_recycle* p, sp_ext::sp_ext_event* p1 )
     :recycle_ptr_(p),
	  event_ptr_(p1)
	{

	}

	virtual ~use_id_thread()
	{

	}
	/// 覆盖此方法
	virtual DWORD thread_method()
	{
		while( this->running_ )
		{
			int new_id;
			for ( int i = 0; i < 10; i++ )
			{
				new_id = recycle_ptr_->create_new_id();
				printf("new created id in thread is:%d \n.", new_id);
			}
			this->event_ptr_->set_event(); // 激活事件
			break;
		}
		return 0;
	}
private:
	sp_ext::id_recycle* recycle_ptr_;
	sp_ext::sp_ext_event* event_ptr_;
};


void test_id_recycle()
{
	sp_ext::id_recycle a_recycle;
	int new_id;
	printf("begin to create id.\n");
	for ( int i = 0; i < 10; i++ )
	{
		getchar();
		new_id = a_recycle.create_new_id();
		printf("new created id is:%d \n.", new_id);
	}

	printf("reset id to 10000.\n");

	sp_ext::sp_ext_event test_event;
	test_event.reset_event();
	use_id_thread test_thread(&a_recycle, &test_event);
    a_recycle.reset(10000);

	test_thread.start();
	test_event.wait_for_event_signaled(); /// 等待线程运行结束
	
	for ( int i = 0; i < 10; i++ )
	{
		getchar();
		new_id = a_recycle.create_new_id();
		printf("new created id is:%d \n.", new_id);
	}
}

/// 测试线程池
class CTestWorkThread : public sp_ext::worker_threads_ex<CTestWorkThread, 20>
{
public:
	CTestWorkThread(void)
	{
		m_nSeq = 1;
	}
	virtual ~CTestWorkThread(void)
	{

	}

	void thread_method( worker_param_t_ptr work_para )
	{
		while ( !this->exit_ )
		{
			Sleep(GetTickCount() % 1000 * m_nSeq%3);
			{
				sp_ext::auto_lock _lock(lock_);
				printf("work thread id:%d, worker id:%d, seq:%d \n", work_para->thread_id, work_para->worker_id, m_nSeq);
				m_nSeq++;
			}
		}
	}
protected:
	int m_nSeq;
	sp_ext::sp_ext_mutex lock_;
};

void test_thread_pool()
{
	CTestWorkThread worker;
    printf("开启线程池");
	worker.create();
    Sleep(10000);
	worker.destroy();
    printf("退出线程池"); 
}

/// 测试定时器线程
class test_timer_listen: public sp_ext::timer_listener  
{
public:
    test_timer_listen()
	{
		this->notify_count_ = 0;
	}
	virtual  int on_time_notify(int time_id)
	{
		this->notify_count_++;

		printf("time id:%d, notify count :%d \n", time_id, this->notify_count_);
		return 0;
	}
    int notify_count_;
};

void test_timer()
{
	sp_ext::sp_ext_time_thread time_thread;
	test_timer_listen a_listener;

	int new_id = time_thread.set_new_timer(& a_listener, 1);
	printf( "register a new timer,id %d \n", new_id );
	printf( "begin time thread \n");
	time_thread.create();
	Sleep( 15 * 1000);
	time_thread.destroy();
    printf( "stop time thread \n");
}

/// 测试消息队列线程池

struct thread_msg_t
{	
	int id;
};



class my_msgqueue_worker_threads: public sp_ext::msgqueue_worker_threads<thread_msg_t, my_msgqueue_worker_threads>
{
public:
	virtual void on_proc_msgqueue_start(worker_param_t_ptr work_para)
	{
		printf("work thread id:%d, worker id:%d \n", work_para->thread_id, work_para->worker_id);
		//Sleep(20);
	}
	virtual void msg_proc(thread_msg_t * pMsgPack, worker_param_t_ptr work_para)
	{
		printf("work thread id:%d, worker id:%d , msg_id :%d\n", work_para->thread_id, work_para->worker_id, pMsgPack->id);
		//Sleep(1000);
		Sleep(50);
	}
};

void test_msgqueue()
{
	my_msgqueue_worker_threads t;
	printf("线程池开始");
	t.create(4);
    Sleep(2000);
    for ( int i=0; i< 50; i++)
    {
		thread_msg_t* p = new thread_msg_t();
		p->id = i;
		t.push(p);
    }

	Sleep(3000);
	thread_msg_t* p = new thread_msg_t();
	p->id = 50;
	t.push(p);
	Sleep(2000);
    t.destroy();
	printf("线程池退出");
}

/// 测试对象回收站

struct recycle_obj_t
{
    int age;
    recycle_obj_t()
	{
		age = 0;
	}
    virtual ~recycle_obj_t()
	{
		printf("age is : %d deleted \n", age);
	}
    void reset()
	{
		age = 0;
	}
};


void test_recyble_obj()
{
	sp_ext::sp_ext_obj_recycle<recycle_obj_t> recycle_(50, 3);

	recycle_obj_t* ptr_array[60] = { NULL };
   
	for ( int i = 0; i < 60; i++  )
	{
		ptr_array[i] = recycle_.create();
        ptr_array[i]->age = i+1;
	}

	for ( int i = 0; i < 60; i++  )
	{
        recycle_obj_t* p;

		p = ptr_array[i];
		printf("age is : %d \n", p->age);
        Sleep(50);
	}
    
    for ( int i = 0; i < 60; i++  )
	{
		recycle_.recycle(ptr_array[i]);
	}
}

/// 测试hashtable

void test_hashtable()
{
	sp_ext::sp_ext_hash_table<int, std::string, sp_ext::sp_ext_mutex> a_table;
	typedef sp_ext::sp_ext_hash_table<int, std::string, sp_ext::sp_ext_mutex> table_type;
	a_table.create(20000);
	
	for ( int i=0; i < 50000; i++ )
	{
		std::string* pvalue = a_table.insert(i);
		*pvalue = sp_ext::StrUtil::Printf("name_%d", i);
	}
    
    
	int id_to_find[] = {8,7, 6, 9,67, 0, 9999, 49999};
    
	for ( int j = 0; j < sizeof(id_to_find)/sizeof(id_to_find[0]); j++ )
	{
		table_type::Iterator it = a_table.find(id_to_find[j]);
		printf("%d->%s \n", *it.key(), it.value()->c_str());
	}
	getchar();
}

/// 测试timer_manager

void on_timeout(sp_ext::HTIMER& h)
{
	sp_ext::inet_time_t now;
	sp_ext::get_timeofday(&now);

	printf("%d, %d \n", now.sec, now.usec);
}

void test_timer_manager()
{
	sp_ext::timer_manager mgr;
    sp_ext::HTIMER a_timer;
	sp_ext::inet_time_t now;
	sp_ext::get_timeofday(&now);

	/// 30 秒后触发
	sp_ext::timer_set(a_timer, on_timeout, 30*1000, now);
	mgr.insert(a_timer);
    
	int ret;
	sp_ext::inet_time_t now_1;
    sp_ext::HTIMER r_timer;
	while( true )
	{  
		sp_ext::get_timeofday(&now_1);
		ret = mgr.pull_expired(now_1, r_timer);
		if ( ret == 0 ) /// 有个定时器到期
		{
			printf("定时器到期.\n");
			break;
		}
		else if ( ret > 0 )  // 剩余毫秒
		{

			printf("left %d s..\n", ret / 1000);
			sp_ext::usleep(1*1000*1000);
		}
		else
		{
			 /// 没有任何定时器
			break;
		}
	}

	printf("结束\n");
   
}

int _tmain(int argc, _TCHAR* argv[])
{
	//test_boyermoore();
	//test_id_recycle();
	//test_thread_pool();
	//test_timer();
	//test_msgqueue();
	//test_recyble_obj();
	//test_hashtable();
	test_timer_manager();
	return 0;
}

