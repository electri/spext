/** Copyright (c) 2008-2010
* All rights reserved.
* 
* �ļ�����:	sp_ext_hash_table.h 
* ժ	 Ҫ:    �򵥹�ϣ��ʵ��
* 
* ��ǰ�汾��	1.0
* ��	 ��:	��ѧ��
* ��	 ��:	�½�
* �������:		2009��3��29��
*/
#ifndef __SP_EXT_HASH_TABLE_H__ 
#define __SP_EXT_HASH_TABLE_H__ 

#include "sp_ext_define.h"
#include "sp_ext_thread.h"

#ifndef offsetof
#define FROM_MEMBER(mPtr,mName,sName) \
	(sName*) ( (char*)mPtr - (size_t) (& ((sName*)0)->mName) )
#else
#define FROM_MEMBER(mPtr,mName,sName) \
	(sName*) ( (char*)mPtr - offsetof(sName,mName) )
#endif

namespace sp_ext
{
////////////////////////////////////////////////////////////////////
//	Allocator			"ͨ�÷�����" (һ�����ٷ���64k�ڴ�)
//	locked_alloc(T**)	"�̰߳�ȫ����"
//	locked_free(T*)		"�̰߳�ȫ�ͷ�"
////////////////////////////////////////////////////////////////////
template<class T,class LockT>
class Allocator
{
	struct UNIT
	{
		Allocator *allocator;
		UNIT *next;
		T body;
	};
	struct GROUP
	{
		GROUP *next;
		UNIT *units;
	};
	unsigned int group_units_;
	GROUP *group_list_;
	UNIT *free_list_;
	LockT lock_;

public:
	Allocator(void): group_list_(NULL), free_list_(NULL)
	{
		group_units_ = (64*1024) /sizeof(UNIT);
		if( group_units_<100 )
			group_units_=100;	//����һ�η���100����Ԫ
	}
	~Allocator(void)
	{
		this->clear();
	}
	static Allocator& single(void)
	{
		static Allocator s;
		return s;
	}
	void clear(void)
	{
		this->lock_.lock();
		GROUP* p;
		if( p=this->group_list_ )
		{
			this->group_list_=NULL;
			this->free_list_=NULL;
		}
		this->lock_.unlock();
		while( p )
		{
			GROUP* next=p->next;
			delete[] p->units;
			delete p;
			p = next;
		}
	}
	T* alloc(void)
	{
		UNIT *pU;
		this->lock_.lock();
		pU = this->free_list_;
		if( pU )
		{
			this->free_list_ = pU->next;
		}
		else
		{
			GROUP* pG;
			if( pG = new GROUP )
			{
				//assert( this->group_units_ );
				if( pG->units = new UNIT[this->group_units_] )
				{
					pG->next = this->group_list_;
					this->group_list_ = pG;
					pU = pG->units;
					pU->next = NULL;
					for( unsigned int i=this->group_units_-1; i--; )
					{
						pU[1].next = pU;
						pU++;
					}
					this->free_list_=pU->next;
				}
			}
		}
		this->lock_.unlock();
		if( pU )
		{
			pU->allocator=this;
			return &pU->body;
		}
		return NULL;
	}
	void recycle(T *ptr)
	{
		assert(ptr);
		UNIT *pU = FROM_MEMBER(ptr,body,UNIT);
		if( pU->allocator==this )
		{
			pU->allocator=NULL;
			this->lock_.lock();
			pU->next = this->free_list_;
			this->free_list_ = pU;
			this->lock_.unlock();
		}
		else 
		{
			assert(0); //invalid unit
		}		
	}
};

#ifdef WIN32
//��gcc�±��룬��ΪCLockδ����, ����MSVC�¾�û������
//��δ��뱻�ŵ�network.h, ���ﶨ����CLock
template<class T>
inline bool locked_alloc(T** pp)
{
	if( *pp=Allocator<T, sp_ext::sp_ext_mutex>::single().alloc() )
	{
		return true;
	}
	return false;
}
template<class T>
inline void locked_free(T* p)
{
	Allocator<T, sp_ext::sp_ext_mutex>::single().recycle(p);
}
#endif

////////////////////////////////////////////////////////////////////
//		ListX "��������"
////////////////////////////////////////////////////////////////////
class ListX
{
public:
	struct entry
	{
		entry *next;
		entry *prev;
	};

private:
	entry* m_head;
	entry* m_tail;
	uint32_t m_count;

public:
	ListX(void): m_head(NULL), m_tail(NULL), m_count(0)
	{
	}
	entry* head(void) const
	{
		return m_head;
	}
	entry* tail(void) const
	{
		return m_tail;
	}
	size_t count(void) const
	{
		return m_count;
	}
	void clear(void)
	{
		m_head=NULL;
		m_tail=NULL;
		m_count=0;
	}
	void push_back(entry* e)
	{
		//assert(e);
		if( m_tail )
		{
			m_tail->next=e;
			e->prev=m_tail;
			m_tail=e;
			m_count++;
		}
		else
		{
			e->prev=NULL;
			m_tail=e;
			m_head=e;
			m_count=1;
		}
		e->next=NULL;
	}
	void remove(entry* e)
	{
		//assert(e);
		entry* next = e->next;
		entry* prev = e->prev;
		if( prev )
		{
			prev->next = next;
		}
		if( next )
		{
			next->prev = prev;
		}
		if( m_head==e )
		{
			m_head=next;
		}
		if( m_tail==e )
		{
			m_tail=prev;
		}
		--m_count;
	}
};

////////////////////////////////////////////////////////////////////
//		sp_ext_hash_table "ͨ��ɢ�б�"
////////////////////////////////////////////////////////////////////

/// KeyT ֻ��Ϊ������
template<class KeyT,class ValueT,class LockT>
class sp_ext_hash_table
{
	struct Record : public ListX::entry
	{
		sp_ext_hash_table* owner;
		ListX::entry order_entry;	//˳������
		KeyT key;
		ValueT value;
	};
	unsigned int table_size_;
	ListX* table_;	//ɢ�б�
	ListX list_;	//˳���, ���ԶԱ��е�Ԫ�ذ�����˳�����ö��
	Allocator<Record, sp_ext::sp_ext_null_lock> allocator_;	//��Ԫ������
	LockT lock_;

public:
	sp_ext_hash_table(void): table_(NULL), table_size_(0)
	{
	}
	~sp_ext_hash_table(void)
	{
		//assert( table_==NULL );
	}

	class Iterator
	{
		Record *record_;
	public:
		Iterator(void): record_(NULL)
		{
		}
		Iterator(Record *rec): record_(rec)
		{
		}
		Iterator(ValueT *val): record_( FROM_MEMBER(val,value,Record) )
		{
		}
		void operator=(Iterator it)
		{
			this->record_ = it.record_;
		}
		const KeyT* key(void) const
		{
			return &this->record_->key;
		}
		ValueT* value(void)
		{
			return &this->record_->value;
		}
		bool isNull(void) const
		{
			return this->record_==NULL;
		}
		friend class sp_ext_hash_table<KeyT,ValueT,LockT>;
	};

	/// ɢ�б�Ĵ�С��Խ���ͻ����ԽС
	bool create(unsigned int tablesize)
	{
		//assert( table_==NULL );
		this->table_size_ = tablesize|1;

		this->table_ = new ListX[this->table_size_+1];
		if( this->table_==NULL )
			return false;

		this->list_.clear();
		return true;
	}
	void destroy(void)
	{
		this->lock_.lock();
		if( this->table_ )
		{
			delete[] this->table_;
			this->table_=NULL;
		}
		this->list_.clear();
		this->allocator_.clear();
		this->lock_.unlock();
	}
	ValueT* insert(KeyT& key)
	{
		//assert( this->table_ );
		this->lock_.lock();
		Record* record = this->allocator_.alloc();
		if( record )
		{
			record->owner = this;
			record->key = key;
			this->table_[ key %this->table_size_ ].push_back(record);	//����ɢ�б�
			this->list_.push_back( &record->order_entry );				//����˳���
			this->lock_.unlock();
			return &record->value;
		}
		this->lock_.unlock();
		return NULL;
	}
	Iterator find(KeyT& key)
	{
		//assert( this->table_ );
		this->lock_.lock();
		Record* record = (Record*) this->table_[ key %this->table_size_].head();
		while( record )
		{
			if( record->key==key )
			{
				this->lock_.unlock();
				//assert( record->owner==this );
				return Iterator(record);
			}
			record=(Record*)record->next;
		}
		this->lock_.unlock();
		return Iterator();
	}
	bool erase(KeyT& key)
	{
		assert( this->table_ );
		this->lock_.lock();
		ListX& slot = this->table_[ key %this->table_size_];
		Record *record = (Record*) slot.head();
		while( record )
		{
			if( record->key==key )
			{
				//assert( record->owner==this );
				record->owner = NULL;
				slot.remove(record);
				this->list_.remove( &record->order_entry );
				this->allocator_.recycle( record );
				this->lock_.unlock();
				return true;
			}
			record = (Record*)record->next;
		}
		this->lock_.unlock();
		return false;
	}
	void erase(Iterator& it)
	{
		Record* record = it.record_;
		if( record && record->owner==this )
		{
			this->lock_.lock();
			record->owner = NULL;
			this->table_[ record->key %this->table_size_].remove(record);
			this->list_.remove( &record->order_entry );
			this->allocator_.recycle( record );
			this->lock_.unlock();
			it.record_ = NULL;
		}
	}
	Iterator first(void) const
	{
		ListX::entry* entry = this->list_.head();
		if( entry )
		{
			return Iterator( FROM_MEMBER(entry,order_entry,Record) );
		}
		return Iterator();
	}
	Iterator next(Iterator& it) const
	{
		assert( it.record_ );
		ListX::entry* entry = it.record_->order_entry.next;
		if( entry )
		{
			return Iterator( FROM_MEMBER(entry,order_entry,Record) );
		}
		return Iterator();
	}
	uint32_t count(void) const
	{
		return this->list_.count();
	}
};

/// �����������ظ�����, vec�������, O(n*n/2)
template<class T>
std::pair<int,int> duplicate(T* vec, int vec_size)
{
	T* cur;
	for( int i=0; i<vec_size; i++ )
	{
		cur = &vec[i];
		for( int j=i+1; j<vec_size; j++ )
		{
			if( *cur==vec[j] )
			{
				return std::pair<int,int>(i,j);
			}
		}
	}
	return std::pair<int,int>(-1,-1);
}


}// sp_ext
#endif // __SP_EXT_HASH_TABLE_H__

