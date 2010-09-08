/** Copyright (c) 2008-2009
 * All rights reserved.
 * 
 * 文件名称:	sp_ext_shared_mutex.cpp  
 * 摘	 要:	读写共享锁
 * 
 * 当前版本:	1.0
 * 作	 者:	陈学术
 * 操	 作:	新建
 * 完成日期:	2010年09月07日
 */

#include "sp_ext_shared_mutex.h"
#include <cassert>

namespace sp_ext
{

shared_mutex::shared_mutex(): m_sharedNum(0), m_exclusiveNum(0), m_lockType(LOCK_NONE)
{
    // 创建用于保护内部数据的互斥量
    m_mutex = ::CreateMutex(NULL, FALSE, NULL);
    // 创建用于同步共享访问线程的事件（手动事件）
    m_sharedEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
    // 创建用于同步独占访问线程的事件（自动事件）
    m_exclusiveEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
}

shared_mutex::~shared_mutex()
{
    ::CloseHandle(m_mutex);
    ::CloseHandle(m_sharedEvent);
    ::CloseHandle(m_exclusiveEvent);
}

// 获取共享访问权
bool shared_mutex::lock_share(DWORD waitTime)
{
    ::WaitForSingleObject(m_mutex, INFINITE);
    ++m_sharedNum;
    if (m_lockType == LOCK_EXCLUSIVE) {
        DWORD retCode = SignalObjectAndWait(m_mutex, m_sharedEvent, waitTime, FALSE);
        if (retCode == WAIT_OBJECT_0) {
            return true;
        } else {
            if (retCode == WAIT_TIMEOUT)
                ::SetLastError(WAIT_TIMEOUT);
            return false;
        }
    }
    m_lockType = LOCK_SHARED;
    ::ReleaseMutex(m_mutex);
    return true;
}

// 释放共享访问权
void shared_mutex::unlock_share()
{
    assert(m_lockType == LOCK_SHARED);
    ::WaitForSingleObject(m_mutex, INFINITE);
    --m_sharedNum;
    if (m_sharedNum == 0) {
        if (m_exclusiveNum > 0) {
            // 唤醒一个独占访问线程
            m_lockType = LOCK_EXCLUSIVE;
            ::SetEvent(m_exclusiveEvent);
        } else {
            // 没有等待线程
            m_lockType = LOCK_NONE;
        }
    }
    ::ReleaseMutex(m_mutex);
}

// 获取独占访问权
bool shared_mutex::lock(DWORD waitTime)
{
    ::WaitForSingleObject(m_mutex, INFINITE);
    ++m_exclusiveNum;
    if (m_lockType != LOCK_NONE) {
        DWORD retCode = ::SignalObjectAndWait(m_mutex, m_exclusiveEvent, waitTime, FALSE);
        if (retCode == WAIT_OBJECT_0) {
            return true;
        } else {
            if (retCode == WAIT_TIMEOUT)
                ::SetLastError(WAIT_TIMEOUT);
            return false;
        }
    }
    m_lockType = LOCK_EXCLUSIVE;
    ::ReleaseMutex(m_mutex);
    return true;
}

// 释放独占访问权
void shared_mutex::unlock()
{
    assert(m_lockType == LOCK_EXCLUSIVE);
    ::WaitForSingleObject(m_mutex, INFINITE);
    --m_exclusiveNum;
    // 独占访问线程优先
    if (m_exclusiveNum > 0) {
        // 唤醒一个独占访问线程
        ::SetEvent(m_exclusiveEvent);
    } else if (m_sharedNum > 0) {
        // 唤醒当前所有共享访问线程
        m_lockType = LOCK_SHARED;
        ::PulseEvent(m_sharedEvent);
    } else {
        // 没有等待线程
        m_lockType = LOCK_NONE;
    }
    ::ReleaseMutex(m_mutex);
}

} // sp_ext