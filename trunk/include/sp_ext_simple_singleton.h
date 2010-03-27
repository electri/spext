/** Copyright (c) 2008-2010
* All rights reserved.
* 
* �ļ�����:	sp_ext_simple_singleton.h     
* ժ	 Ҫ:	�򵥵�����ʵ��
* 
* ��ǰ�汾��	1.0
* ��	 ��:	��ѧ��
* ��	 ��:	�½�
* �������:	2010��03��24��
*/

#ifndef __SP_EXT_SIMPLE_SINGLETON_H__
#define	__SP_EXT_SIMPLE_SINGLETON_H__

namespace sp_ext
{
template<typename T>
class sp_ext_singleton 
{
public:
	static T* get_instance() 
	{
		return instance_;
	}
	static void new_instance() 
	{
		delete_instance();
		instance_ = new T();
	}

	static void delete_instance() 
	{
		if( instance_ )
		{
			T* pTmp = instance_;
			instance_ = NULL;
			delete pTmp;
		}
	}

	static void set_instance_null()
	{
		instance_ = NULL; 
	}

protected:
	sp_ext_singleton() 
	{ 
	}
	virtual ~sp_ext_singleton() 
	{ 
		/*if(instance_)
		{
			delete instance_;
			instance_ = NULL;
		}*/
	}
	static T* instance_;
private:
	sp_ext_singleton(const sp_ext_singleton&);
	sp_ext_singleton& operator=(const sp_ext_singleton&);
};

template<typename T> T* sp_ext_singleton<T>::instance_ = NULL;

}// sp_ext
#endif	/* __SP_EXT_SIMPLE_SINGLETON_H__ */
