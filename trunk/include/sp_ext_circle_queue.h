/** Copyright (c) 2008-2010
* All rights reserved.
* 
* �ļ�����:	sp_ext_circle_queue.h 
* ժ	 Ҫ:    ���ζ���ʵ��
* 
* ��ǰ�汾��	1.0
* ��	 ��:	��ѧ��
* ��	 ��:	�½�
* �������:		2009��3��28��
*/
#ifndef __SP_EXT_CIRCLE_QUEUE_H__ 
#define __SP_EXT_CIRCLE_QUEUE_H__ 

namespace sp_ext
{
/**
    Tֻ��Ϊint,void * ,char�ȼ�����
 */
template <class T, int DefaultQueueSize = 50> 
class sp_ext_circle_queue { 
public: 
	/// initSize: ѭ����������
    sp_ext_circle_queue(int container_size = DefaultQueueSize) 
	{ 
        container_size++; // ��1����Ϊ�и�Ԫ�������п�
        qlist = new T[container_size]; 
        front = 0; 
        rear = 0; 
        count = 0; 
        size = container_size; 
    } 
    /// �������� 
    ~sp_ext_circle_queue() 
	{ 
        if ( qlist ) delete [] qlist; 
        front = 0; 
        rear = 0; 
        count = 0; 
        size = 0; 
    } 

    /// �ж϶����Ƿ�Ϊ�� 
    int empty() 
	{ 
        return front == rear; 
    } 

    /// �ж϶����Ƿ����� 
    int full() 
	{ 
        return (rear+1) % size == front; 
    }

	/// ��������
	int container_size()
	{
		return ( this->size - 1 );
	}

	/// ���г���(Ԫ�ظ���) 
    int length() 
	{ 
        return count; 
    }

	/// ���
    void enqueue(const T &item) 
	{ 
        if ( !full() ) 
		{ 
            count ++; 
            qlist[rear] = item; 
            rear = (rear + 1) % size; //rearʼ��ָ�����һ��Ԫ�ص���һ��λ�� 
        } 
    } 
    /// ���� 
    T dequeue() 
	{ 
		T data;
        if ( !empty()) 
		{ 
            data = qlist[front]; 
            count --; 
            front = (front + 1) % size; //front������һλ�� 
        } 
        return data; 
    } 
    /// ��ȡ����Ԫ�� 
    T queue_front() 
	{  
		T data;
        if ( !empty() ) 
            data = qlist[front]; 
        
        return data; 
    } 
    /// ��ն��� 
    void clear() 
	{ 
        front = 0; 
        rear = 0; 
        count = 0; 
    } 

private: 
	T* qlist; /// ��Ŷ���Ԫ�ص�ָ��(����) 
	int size; /// ���鳤��
	int front; /// ����λ�� 
	int rear; /// ��βλ��(���һ��Ԫ�ص���һλ��) 
	int count; /// ������Ԫ�صĸ���
}; 

}// sp_ext
#endif // __SP_EXT_CIRCLE_QUEUE_H__

