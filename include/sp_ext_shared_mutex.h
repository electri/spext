/** Copyright (c) 2008-2009
 * All rights reserved.
 * 
 * �ļ�����:	sp_ext_shared_mutex.h  
 * ժ	 Ҫ:	��д������
 * 
 * ��ǰ�汾:	1.0
 * ��	 ��:	��ѧ��
 * ��	 ��:	�½�
 * �������:	2010��09��07��
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

// �ɹ���Ļ�����
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

    // ��ȡ�������Ȩ
    bool lock_share(DWORD waitTime = INFINITE);
    // �ͷŹ������Ȩ
    void unlock_share();

    // ��ȡ��ռ����Ȩ
    bool lock(DWORD waitTime = INFINITE);
    // �ͷŶ�ռ����Ȩ
    void unlock();
public:
    // ��ռ�Զ���
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
    // �����Զ���
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
