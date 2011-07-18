/** Copyright (c) 2008-2010
* All rights reserved.
* 
* 文件名称:	sp_ext_circle_queue.h 
* 摘	 要:    环形队列实现
* 
* 当前版本：	1.0
* 作	 者:	陈学术
* 操	 作:	新建
* 完成日期:		2009年3月28日
*/
#ifndef __SP_EXT_CIRCLE_QUEUE_H__ 
#define __SP_EXT_CIRCLE_QUEUE_H__ 

namespace sp_ext
{
/**
    T只能为int,void * ,char等简单类型
 */
template <class T, int DefaultQueueSize = 50> 
class sp_ext_circle_queue { 
public: 
	/// initSize: 循环队列容量
    sp_ext_circle_queue(int container_size = DefaultQueueSize) 
	{ 
        container_size++; // 加1，因为有个元素用来判空
        qlist = new T[container_size]; 
        front = 0; 
        rear = 0; 
        count = 0; 
        size = container_size; 
    } 
    /// 析构函数 
    ~sp_ext_circle_queue() 
	{ 
        if ( qlist ) delete [] qlist; 
        front = 0; 
        rear = 0; 
        count = 0; 
        size = 0; 
    } 

    /// 判断队列是否为空 
    int empty() 
	{ 
        return front == rear; 
    } 

    /// 判断队列是否已满 
    int full() 
	{ 
        return (rear+1) % size == front; 
    }

	/// 队列容量
	int container_size()
	{
		return ( this->size - 1 );
	}

	/// 队列长度(元素个数) 
    int length() 
	{ 
        return count; 
    }

	/// 入队
    void enqueue(const T &item) 
	{ 
        if ( !full() ) 
		{ 
            count ++; 
            qlist[rear] = item; 
            rear = (rear + 1) % size; //rear始终指向最后一个元素的下一个位置 
        } 
    } 
    /// 出队 
    T dequeue() 
	{ 
		T data;
        if ( !empty()) 
		{ 
            data = qlist[front]; 
            count --; 
            front = (front + 1) % size; //front移向下一位置 
        } 
        return data; 
    } 
    /// 读取队首元素 
    T queue_front() 
	{  
		T data;
        if ( !empty() ) 
            data = qlist[front]; 
        
        return data; 
    } 
    /// 清空队列 
    void clear() 
	{ 
        front = 0; 
        rear = 0; 
        count = 0; 
    } 

private: 
	T* qlist; /// 存放队列元素的指针(数组) 
	int size; /// 数组长度
	int front; /// 队首位置 
	int rear; /// 队尾位置(最后一个元素的下一位置) 
	int count; /// 队列中元素的个数
}; 

}// sp_ext
#endif // __SP_EXT_CIRCLE_QUEUE_H__

