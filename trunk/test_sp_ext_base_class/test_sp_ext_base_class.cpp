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
#include "sp_ext_api_tools.h"
#include "sys/sys_thread.h"
#include "SharedMutex_example.h"

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
                sp_ext::auto_lock<sp_ext::sp_ext_mutex> _lock(lock_);
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
    thread_msg_t():id(0) {}
    thread_msg_t(int a_id):id(a_id) {}
    void destroy() { delete this; }
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
	sp_ext::sp_ext_hash_table<int, sp_ext::String, sp_ext::sp_ext_mutex> a_table;
	typedef sp_ext::sp_ext_hash_table<int, sp_ext::String, sp_ext::sp_ext_mutex> table_type;
	a_table.create(20000);
	
	for ( int i=0; i < 50000; i++ )
	{
		sp_ext::String* pvalue = a_table.insert(i);
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
	//sp_ext::timer_set(a_timer, on_timeout, 30*1000, now);
	//mgr.insert(a_timer);
	if ( mgr.insert(on_timeout, 30*1000, &a_timer) )
	{
		printf("insert timer id :%d \n", a_timer.id);
	}
	else
	{
		return;
	}
	
    
	int ret;
	sp_ext::inet_time_t now_1;
    sp_ext::HTIMER r_timer;
	while( true )
	{  
		//sp_ext::get_timeofday(&now_1);
		//ret = mgr.pull_expired(now_1, r_timer);
		ret = mgr.pull_expired(r_timer);
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

/// 测试条件变量

sp_ext::thread_mutex flag_mutex;
sp_ext::thread_condition flag_cont;

sp_ext::thread_barrier thread_get_barrier(2);

int flag;

class thread_get : public sp_ext::sp_ext_thread
{
 public:
     virtual DWORD thread_method()
     {
         printf("thread id: %d\n", this->thread_id_);
         thread_get_barrier.wait();
         flag_mutex.lock();
         while( !flag )
             flag_cont.wait();
         flag_mutex.unlock();
         
         printf("thread id: %d receive changed signal \n", this->thread_id_);
         return 0;
     }
};



class thread_put : public sp_ext::sp_ext_thread
{
public:
    virtual DWORD thread_method()
    {
        flag_mutex.lock();
        flag = 1;
        printf("flag has been changed. \n");
        flag_cont.broadcast();
        flag_mutex.unlock();
        return 0;
    }
};

void test_condition()
{
    flag = 0;
    flag_mutex.open();  
    flag_cont.open(&flag_mutex);
    thread_get tg;
    thread_get tg1;
    thread_put tp;

    tg.start();
    tg1.start();
    
    Sleep(5000);
    tp.start();

    getchar();
    printf("exit.\n");

};

/// 测试xy thread
class busi_driver : public xy::thread_svc<xy::MT_SYN_TRAITS, thread_msg_t*>
{
    typedef xy::thread_svc<xy::MT_SYN_TRAITS, thread_msg_t*> base_type;
public:
    busi_driver(void){}
    virtual ~busi_driver(void){}
protected:
    virtual int svc()
    {
        int r = 0;
        thread_msg_t* pkt = NULL;

        while( !is_shut_down() )
        {
            r = getq(pkt);

            if( xy::err_code::e_ok != r )
            {
                continue;
            }
            if( pkt )
            {
                //do_busi(pkt);
                printf("msg_id :%d\n", pkt->id);

                pkt->destroy();
                pkt = NULL;
            }
        }
        return 0;
    }
public:
    bool init()
    {
        open(5);
        active();
        return true;
    }

    void stop()
    {
        shut_down();
        wait(5000);

        thread_msg_t* b = NULL;
        while( xy::err_code::e_ok == getq(b, 0) )
        {
            b->destroy();
            b = NULL;
        }
    }
    int putq(thread_msg_t*& val, long tm = -1)
    {
        return m_queue.enqueue(val, tm);
    }

};

void test_xy_thread_pool()
{
    busi_driver b;
    printf("begin to start.\n");
    b.init();
    for ( int i = 0; i < 5000; i++ )
    {
        thread_msg_t* p = new thread_msg_t(i); 
        b.putq(p);
    }
    Sleep(5000);
    printf("begin to stop.\n");
    b.stop();
    getchar();
}

/// xy 单线程

class inet_driver : public xy::thread_base
{
public:
    inet_driver(void)
    {
        count_ = 0;
    }
public:
    virtual ~inet_driver(void){}
public:
    bool open()
    {
        /*
        if( !open_i(dll) )
            return false;
        */
        active();
        return true;
    }
    void stop()
    {
        shut_down();
        wait();
    }
protected:
    virtual int svc()
    {
        while( !is_shut_down() )
        {
            count_++;
            printf("loop count :%d\n", count_);
        }
        return 0;
    }
protected:
    int count_;
};

void test_xy_single_thread()
{
    inet_driver d;
    d.open();
    Sleep(5000);
    d.stop();
    getchar();
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
	//test_timer_manager();
    //test_condition();
    //test_xy_thread_pool();
    //test_xy_single_thread();
    test_share_mutex();
	return 0;
}

