/** Copyright (c) 2008-2010
* All rights reserved.
* 
* 文件名称:	sp_ext_obj_recycle.h
* 摘	 要:    对象回收器
* 
* 当前版本：	1.0
* 作	 者:	陈学术
* 操	 作:	新建
* 完成日期:    2009年3月28日
*/
#ifndef __SP_EXT_OBJ_RECYCLE_H__ 
#define __SP_EXT_OBJ_RECYCLE_H__ 

#include "sp_ext_circle_queue.h"
#include "sp_ext_thread.h"

namespace sp_ext
{
/// T的构造函数只能为T()
template <class T> 
class sp_ext_obj_recycle
{
public:
	/// queue_size:队列容量
	/// init_length:队列初始化长度，必须<= queue_size
   sp_ext_obj_recycle(int queue_size, int init_length = 0)
	   :queue_(queue_size)  
   {
	   if ( init_length > queue_size )
	   {
		   return ;
	   }
	   T* p;
	   for ( int i = 0; i < init_length; i++ )
	   {
		   p = new T();
		   queue_.enqueue(p);
	   }
   }

   virtual ~sp_ext_obj_recycle()
   {
	   T* p;
	   queue_lock_.lock();
	   /// 清空回收站中的对象
	   while ( this->queue_.length() > 0 )
	   {
		   p = this->queue_.dequeue();
		   if ( p )
		   {
			   delete p;
		   }
	   }
	   queue_lock_.unlock();
   }

   /// 从回收站中取一个对象
   T* create()
   {
	    sp_ext::auto_lock(this->queue_lock_);
   		if ( !queue_.empty() )
   		{
   		    return this->queue_.dequeue();
   		}
   		else
   		{
   			return new T();
   		}
   };

   /// 回收一个对象
   bool recycle(T* p)
   {
        if ( p == NULL )
		{
            return false;
		}

        sp_ext::auto_lock(this->queue_lock_);
		if ( !queue_.full() )
		{
			queue_.enqueue( p );
			return true;
		}
		else
		{
		   delete p;
		   return false;
		}
   }

   /// 是否空
   bool empty()
   {
	   sp_ext::auto_lock(this->queue_lock_);
	   return this->queue_.empty();
   }
   
   /// 是否满
   bool full()
   {
	   sp_ext::auto_lock(this->queue_lock_);
	   return this->queue_.full();
   }
   
   /// 回收站可用对象长度
   bool length()
   {
	   sp_ext::auto_lock(this->queue_lock_);
	   return this->queue_.length();
   }

   /// 回收站容量
   int size()
   {	
	   sp_ext::auto_lock(this->queue_lock_);
	   return this->queue_.container_size();
   }
   
protected:
   sp_ext::sp_ext_circle_queue<T*> queue_;
   sp_ext::sp_ext_mutex queue_lock_;
};

} // sp_ext

#endif /// __SP_EXT_OBJ_RECYCLE_H__