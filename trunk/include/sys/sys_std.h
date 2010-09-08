#ifndef __AOT_STD_20100504_H__
#define __AOT_STD_20100504_H__

#include <commondef/aot_typedef.h>
#include <new>

namespace aot_std{

struct default_alloc
{
	void* alloc(aot_uint32_t size)
	{
		return (void*) ::operator new (size);
	}
	void* calloc(aot_uint32_t size)
	{
		void* p = (void*) ::operator new (size);
		if( p )
		{
			memset(p, 0, size);
		}
		return p;
	}
	void dealloc(void* p)
	{
		::operator delete (p);
	}
};

template<class T, class ALLOC>
class list
{
	struct node_t
	{
		__AOT_QUEUE_DECALRE__(node_t);
		T val;
		node_t(const T& t) : val(t)
		{
			;
		}
		node_t()
		{
			;
		}
	};
public:
	typedef list<T, ALLOC> this_type;
	typedef node_t* iterator;

	iterator goto_begin()
	{
		if( aot_queue_is_empty(&this->header_) )
			return NULL;
		return aot_queue_head(&this->header_);
	}
	iterator goto_next(iterator it)
	{
		if( NULL == it )
			return NULL;
		return it->next;
	}
	bool is_end(iterator it)
	{
		return (NULL == it) || (it == &this->header_);
	}
	void remove_iter(iterator it)
	{
		if( is_end(it) )
		{
			return;
		}
		aot_queue_remove(it);
		--this->size_;
		it->~node_t();
		this->alloc_.dealloc(it);
	}
	T& get_val(iterator it)
	{
		return it->val;
	}
public:
	list()
	{
		size_ = 0;
		aot_queue_init(&this->header_);
	}
	~list()
	{
		clear();
	}
public:
	aot_uint32_t size()
	{
		return this->size_;
	}
	bool push_back(const T& val)
	{
		node_t* p = (node_t*)this->alloc_.alloc(sizeof(node_t));
		if( NULL == p )
		{
			return false;
		}
		new (p) node_t(val);
		aot_queue_insert_tail(&this->header_, p);
		++this->size_;
		return true;
	}
	bool push_front(const T& val)
	{
		node_t* p = (node_t*)this->alloc_.alloc(sizeof(node_t));
		if( NULL == p )
		{
			return false;
		}
		new (p) node_t(val);
		aot_queue_insert_head(&this->header_, p);
		++this->size_;
		return true;
	}
	T& back()
	{
		return (aot_queue_tail(&this->header_))->val;
	}
	void pop_back()
	{
		node_t* t = aot_queue_tail(&this->header_);
		remove_iter(t);
	}
	void clear()
	{
		node_t* t = NULL;
		while( !aot_queue_is_empty(&this->header_) )
		{
			t = aot_queue_tail(&this->header_);
			aot_queue_remove(t);
			t->~node_t();
			this->alloc_.dealloc(t);
		}
		this->size_ = 0;
	}
private:
	node_t header_;
	ALLOC alloc_;
	aot_uint32_t size_;
};

}
#endif /// __AOT_STD_20100504_H__