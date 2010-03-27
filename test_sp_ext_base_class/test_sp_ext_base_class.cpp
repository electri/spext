// test_sp_ext_base_class.cpp : �������̨Ӧ�ó������ڵ㡣
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

/// ����boyer�㷨
void test_boyermoore()
{
	std::string strToFind	=	"birthday"; 
	std::string	strSrc		=	"   dfhdfffdkfdkfjdfjf birthday is :2009-03-14";
	sp_ext::BoyerMoore moore( strToFind.c_str() );
	LONG value = moore.GetNumberAfter( strSrc.c_str(), strSrc.size(), 0 );
	LONG date  = moore.GetDateAfter( strSrc.c_str(), strSrc.size(), 0 );

	printf("number is %d, birthday number is:%d", value, date);
	/* ���н����number is 2009, birthday number is:20090314 */
	getchar();
}

/// ����id recycle
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
	/// ���Ǵ˷���
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
			this->event_ptr_->set_event(); // �����¼�
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
	test_event.wait_for_event_signaled(); /// �ȴ��߳����н���
	
	for ( int i = 0; i < 10; i++ )
	{
		getchar();
		new_id = a_recycle.create_new_id();
		printf("new created id is:%d \n.", new_id);
	}
}

/// �����̳߳�
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
    printf("�����̳߳�");
	worker.create();
    Sleep(10000);
	worker.destroy();
    printf("�˳��̳߳�"); 
}

/// ���Զ�ʱ���߳�
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

/// ������Ϣ�����̳߳�

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
	printf("�̳߳ؿ�ʼ");
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
	printf("�̳߳��˳�");
}

/// ���Զ������վ

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



int _tmain(int argc, _TCHAR* argv[])
{
	//test_boyermoore();
	//test_id_recycle();
	//test_thread_pool();
	//test_timer();
	//test_msgqueue();
	test_recyble_obj();
	return 0;
}

