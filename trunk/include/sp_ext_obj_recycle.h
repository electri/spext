/** Copyright (c) 2008-2010
* All rights reserved.
* 
* �ļ�����:	sp_ext_obj_recycle.h
* ժ	 Ҫ:    ���������
* 
* ��ǰ�汾��	1.0
* ��	 ��:	��ѧ��
* ��	 ��:	�½�
* �������:    2009��3��28��
*/
#ifndef __SP_EXT_OBJ_RECYCLE_H__ 
#define __SP_EXT_OBJ_RECYCLE_H__ 

#include "sp_ext_circle_queue.h"
#include "sp_ext_thread.h"

namespace sp_ext
{
/// T�Ĺ��캯��ֻ��ΪT()
template <class T> 
class sp_ext_obj_recycle
{
public:
	/// queue_size:��������
	/// init_length:���г�ʼ�����ȣ�����<= queue_size
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
	   /// ��ջ���վ�еĶ���
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

   /// �ӻ���վ��ȡһ������
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

   /// ����һ������
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

   /// �Ƿ��
   bool empty()
   {
	   sp_ext::auto_lock(this->queue_lock_);
	   return this->queue_.empty();
   }
   
   /// �Ƿ���
   bool full()
   {
	   sp_ext::auto_lock(this->queue_lock_);
	   return this->queue_.full();
   }
   
   /// ����վ���ö��󳤶�
   bool length()
   {
	   sp_ext::auto_lock(this->queue_lock_);
	   return this->queue_.length();
   }

   /// ����վ����
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