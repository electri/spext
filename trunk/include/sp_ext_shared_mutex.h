/** Copyright (c) 2008-2009
 * All rights reserved.
 * 
 * 文件名称:	sp_ext_shared_mutex.h  
 * 摘	 要:	读写共享锁
 * 
 * 当前版本:	1.0
 * 作	 者:	陈学术
 * 操	 作:	新建
 * 完成日期:	2010年09月07日
 */

#ifndef __SP_EXT_SHARED_MUTEX_H__
#define __SP_EXT_SHARED_MUTEX_H__

#ifndef _WIN32
#error "works only on Windows"
#endif

#ifndef _WIN32_WINNT
#define  _WIN32_WINNT  0x0400
#endif // _WIN32_WINNT

#include <windows.h>
#include "sp_ext_noncopyable.h"

namespace sp_ext
{

// 可共享的互斥量
class shared_mutex : private noncopyable
{
private:
    HANDLE m_mutex;
    HANDLE m_sharedEvent;
    HANDLE m_exclusiveEvent;

    volatile int m_sharedNum;
    volatile int m_exclusiveNum;
    volatile int m_lockType;

    static const int LOCK_NONE = 0;
    static const int LOCK_SHARED = 1;
    static const int LOCK_EXCLUSIVE = 2;

public:
    shared_mutex();
    ~shared_mutex();

    // 获取共享访问权
    bool lock_share(DWORD waitTime = INFINITE);
    // 释放共享访问权
    void unlock_share();

    // 获取独占访问权
    bool lock(DWORD waitTime = INFINITE);
    // 释放独占访问权
    void unlock();
public:
    // 独占自动锁
    class auto_lock
    {
    public:
        auto_lock(shared_mutex& t) : lock_(t)
        {
            lock_.lock();
        }
        ~auto_lock()
        {
            lock_.unlock();
        }
    private:
        shared_mutex& lock_;
    };
public:
    // 共享自动锁
    class auto_lock_share
    {
    public:
        auto_lock_share(shared_mutex& t) : lock_(t)
        {
            lock_.lock_share();
        }
        ~auto_lock_share()
        {
            lock_.unlock_share();
        }
    private:
        shared_mutex& lock_;
    };
};

} // sp_ext
#endif // __SP_EXT_SHARED_MUTEX_H__
