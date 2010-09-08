/** Copyright (c) 2008-2009
 * All rights reserved.
 * 
 * �ļ�����:	sp_ext_shared_mutex.cpp  
 * ժ	 Ҫ:	��д������
 * 
 * ��ǰ�汾:	1.0
 * ��	 ��:	��ѧ��
 * ��	 ��:	�½�
 * �������:	2010��09��07��
 */

#include "sp_ext_shared_mutex.h"
#include <cassert>

namespace sp_ext
{

shared_mutex::shared_mutex(): m_sharedNum(0), m_exclusiveNum(0), m_lockType(LOCK_NONE)
{
    // �������ڱ����ڲ����ݵĻ�����
    m_mutex = ::CreateMutex(NULL, FALSE, NULL);
    // ��������ͬ����������̵߳��¼����ֶ��¼���
    m_sharedEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
    // ��������ͬ����ռ�����̵߳��¼����Զ��¼���
    m_exclusiveEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
}

shared_mutex::~shared_mutex()
{
    ::CloseHandle(m_mutex);
    ::CloseHandle(m_sharedEvent);
    ::CloseHandle(m_exclusiveEvent);
}

// ��ȡ�������Ȩ
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

// �ͷŹ������Ȩ
void shared_mutex::unlock_share()
{
    assert(m_lockType == LOCK_SHARED);
    ::WaitForSingleObject(m_mutex, INFINITE);
    --m_sharedNum;
    if (m_sharedNum == 0) {
        if (m_exclusiveNum > 0) {
            // ����һ����ռ�����߳�
            m_lockType = LOCK_EXCLUSIVE;
            ::SetEvent(m_exclusiveEvent);
        } else {
            // û�еȴ��߳�
            m_lockType = LOCK_NONE;
        }
    }
    ::ReleaseMutex(m_mutex);
}

// ��ȡ��ռ����Ȩ
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

// �ͷŶ�ռ����Ȩ
void shared_mutex::unlock()
{
    assert(m_lockType == LOCK_EXCLUSIVE);
    ::WaitForSingleObject(m_mutex, INFINITE);
    --m_exclusiveNum;
    // ��ռ�����߳�����
    if (m_exclusiveNum > 0) {
        // ����һ����ռ�����߳�
        ::SetEvent(m_exclusiveEvent);
    } else if (m_sharedNum > 0) {
        // ���ѵ�ǰ���й�������߳�
        m_lockType = LOCK_SHARED;
        ::PulseEvent(m_sharedEvent);
    } else {
        // û�еȴ��߳�
        m_lockType = LOCK_NONE;
    }
    ::ReleaseMutex(m_mutex);
}

} // sp_ext