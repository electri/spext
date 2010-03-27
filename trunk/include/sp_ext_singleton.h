/** Copyright (c) 2008-2010
* All rights reserved.
* 
* 文件名称:	sp_ext_singleton.h     
* 摘	 要:	单例类实现
* 
* 当前版本：	1.0
* 作	 者:	陈学术
* 操	 作:	新建
* 完成日期:	2010年03月22日
*/

/**
 使用举例：
    #include <singleton.h>
    #include <stdio.h>
  
    class MySingleton : private NonCopyable, public Singleton<MySingleton>
	{
		friend class SingletonFactory<MySingleton>; // to access the private constructor
	public:
	virtual ~MySingleton()
		{
			printf("MySingleton destroy.\n");
		}
	public:
		int GetFoo() const { return m_Foo; }
	private:

		MySingleton(int foo) : m_Foo(foo) 
		{
			printf("MySingleton create. foo=%d\n", foo);
		}
	private:
		int m_Foo;
	};

	void main()
	{
		#ifndef SINGLETON_COMPILE_TIME_CHECK
		// create the singleton once
		SingletonFactory<MySingleton>().create(15);
	#endif

		// using it
		MySingleton* pMySingleton = MySingleton::instance();
		//assert(pMySingleton);
	    
		printf("input foo is :%d", MySingleton::instance()->GetFoo());

		 //destroy it
		 SingletonFactory<MySingleton>().destroy();
	}
 */

#ifndef __SP_EXT_SINGLETON_H__
#define	__SP_EXT_SINGLETON_H__

#include <memory>

/* SingletonFactory */
template <typename _TSingleton>
class SingletonFactory
{
public:
virtual ~SingletonFactory() {}

_TSingleton* create()
{
	if (isCreated)
		return _TSingleton::mp_Instance.get();

	isCreated = true;
	_TSingleton* pTmp = new _TSingleton();
	_TSingleton::mp_Instance.reset(pTmp);

	return _TSingleton::mp_Instance.get();
	}

	template <typename _TParam>
	_TSingleton* create(_TParam param)
	{
	if (isCreated)
		return _TSingleton::mp_Instance.get();

	isCreated = true;
	_TSingleton* pTmp = new _TSingleton(param);
	_TSingleton::mp_Instance.reset(pTmp);

	return _TSingleton::mp_Instance.get();
	}

	template <typename _TP1, typename _TP2>
	_TSingleton* create(_TP1 p1, _TP2 p2)
	{
	if (isCreated)
		return _TSingleton::mp_Instance.get();

	isCreated = true;
	_TSingleton* pTmp = new _TSingleton(p1, p2);
	_TSingleton::mp_Instance.reset(pTmp);

	return _TSingleton::mp_Instance.get();
}

template <typename _TP1, typename _TP2, typename _TP3>
_TSingleton* create(_TP1 p1, _TP2 p2, _TP3 p3)
{
	if (isCreated)
		return _TSingleton::mp_Instance.get();

	isCreated = true;
	_TSingleton* pTmp = new _TSingleton(p1, p2, p3);
	_TSingleton::mp_Instance.reset(pTmp);

	return _TSingleton::mp_Instance.get();
}

template <typename _TP1, typename _TP2, typename _TP3, typename _TP4>
_TSingleton* create(_TP1 p1, _TP2 p2, _TP3 p3, _TP4 p4)
{
	if (isCreated)
		return _TSingleton::mp_Instance.get();

	isCreated = true;
	_TSingleton* pTmp = new _TSingleton(p1, p2, p3, p4);
	_TSingleton::mp_Instance.reset(pTmp);

	return _TSingleton::mp_Instance.get();
}

template <typename _TP1, typename _TP2, typename _TP3, typename _TP4, typename _TP5>
_TSingleton* create(_TP1 p1, _TP2 p2, _TP3 p3, _TP4 p4, _TP5 p5)
{
	if (isCreated)
		return _TSingleton::mp_Instance.get();

	isCreated = true;
	_TSingleton* pTmp = new _TSingleton(p1, p2, p3, p4, p5);
	_TSingleton::mp_Instance.reset(pTmp);

	return _TSingleton::mp_Instance.get();
}

template <typename _TP1, typename _TP2, typename _TP3, typename _TP4, typename _TP5, typename _TP6>
_TSingleton* create(_TP1 p1, _TP2 p2, _TP3 p3, _TP4 p4, _TP5 p5, _TP6 p6)
{
	if (isCreated)
		return _TSingleton::mp_Instance.get();

	isCreated = true;
	_TSingleton* pTmp = new _TSingleton(p1, p2, p3, p4, p5, p6);
	_TSingleton::mp_Instance.reset(pTmp);

	return _TSingleton::mp_Instance.get();
}

template <typename _TP1, typename _TP2, typename _TP3, typename _TP4, typename _TP5, typename _TP6, typename _TP7>
_TSingleton* create(_TP1 p1, _TP2 p2, _TP3 p3, _TP4 p4, _TP5 p5, _TP6 p6, _TP7 p7)
{
	if (isCreated)
		return _TSingleton::mp_Instance.get();

	isCreated = true;
	_TSingleton* pTmp = new _TSingleton(p1, p2, p3, p4, p5, p6, p7);
	_TSingleton::mp_Instance.reset(pTmp);

	return _TSingleton::mp_Instance.get();
}

template <typename _TP1, typename _TP2, typename _TP3, typename _TP4, typename _TP5, typename _TP6, typename _TP7, typename _TP8>
_TSingleton* create(_TP1 p1, _TP2 p2, _TP3 p3, _TP4 p4, _TP5 p5, _TP6 p6, _TP7 p7, _TP8 p8)
{
	if (isCreated)
		return _TSingleton::mp_Instance.get();

	isCreated = true;
	_TSingleton* pTmp = new _TSingleton(p1, p2, p3, p4, p5, p6, p7, p8);
	_TSingleton::mp_Instance.reset(pTmp);

	return _TSingleton::mp_Instance.get();
}

void destroy()
{
	_TSingleton::mp_Instance.reset();
	isCreated = false;
}
private:
    static bool isCreated;
};

template <typename _TSingleton> bool SingletonFactory<_TSingleton>::isCreated = false;

/* Singleton */
template <typename _Ty>
class Singleton
{
    friend class SingletonFactory<_Ty>;
protected:
    Singleton()
    {
    }

    Singleton(const Singleton& other)
    {
    }

    Singleton & operator=(const Singleton& other)
    {
		return *this;
    }

public:
    static _Ty* instance()
    {
		return mp_Instance.get();
    }

protected:
    static std::auto_ptr<_Ty> mp_Instance;
};

#ifndef SINGLETON_COMPILE_TIME_CHECK
template <typename _Ty> std::auto_ptr<_Ty> Singleton<_Ty>::mp_Instance;
#endif



#endif	/* __SP_EXT_SINGLETON_H__ */
