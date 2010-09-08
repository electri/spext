/**
* SharedMutex_example.cpp
* @Author   Tu Yongce <yongce (at) 126 (dot) com>
* @Created  2008-11-17
* @Modified 2008-11-17
* @Version  0.1
*/
#include "stdafx.h"
#include "SharedMutex_example.h"
#include <process.h>
#include <iostream>
#include "sp_ext_shared_mutex.h"
#include "sp_ext_thread.h"

using namespace std;
sp_ext::shared_mutex g_mutex;

const int LOOP_NUM = 2000;
volatile __int64 g_data = 0;
CRITICAL_SECTION g_cs;

unsigned WINAPI ReaderThread(void *pParam)
{
    int id = (int)pParam;

    ::EnterCriticalSection(&g_cs);
    cout << "Reader [" << id << "] start" << endl;
    ::LeaveCriticalSection(&g_cs);

    __int64 max = 0;
    __int64 min = 0;

    for (int i = 0; i < LOOP_NUM; ++i) {
        {       
            sp_ext::shared_mutex::auto_lock_share s_lock(g_mutex);
            __int64 data = g_data;
            if (data > max)
                max = data;
            if (data < min)
                min = data;
        }
        Sleep(1);
    }

    ::EnterCriticalSection(&g_cs);
    cout << "Reader [" << id << "] quit, max = " << max << ", min = " << min << endl;
    ::LeaveCriticalSection(&g_cs);

    return 0;
}

unsigned WINAPI WriterThread1(void *pParam)
{
    int id = (int)pParam;

    ::EnterCriticalSection(&g_cs);
    cout << "Writer1 [" << id << "] start" << endl;
    ::LeaveCriticalSection(&g_cs);

    for (int i = 0; i < LOOP_NUM; ++i) {
        {
            sp_ext::shared_mutex::auto_lock e_lock(g_mutex);
            g_data = g_data + i;
        }
        Sleep(1);
    }

    ::EnterCriticalSection(&g_cs);
    cout << "Writer1 [" << id << "] quit" << endl;
    ::LeaveCriticalSection(&g_cs);

    return 0;
}

unsigned WINAPI WriterThread2(void *pParam)
{
    int id = (int)pParam;

    ::EnterCriticalSection(&g_cs);
    cout << "Writer2 [" << id << "] start" << endl;
    ::LeaveCriticalSection(&g_cs);

    for (int i = 0; i < LOOP_NUM; ++i) 
    {
        {
            sp_ext::shared_mutex::auto_lock e_lock(g_mutex);
            g_data = g_data - i;
        }
        Sleep(1);
    }

    ::EnterCriticalSection(&g_cs);
    cout << "Writer2 [" << id << "] quit" << endl;
    ::LeaveCriticalSection(&g_cs);

    return 0;
}

int test_share_mutex()
{
    sp_ext::time_waiter waiter;
    ::InitializeCriticalSection(&g_cs);

    // 创建读写工作线程（创建时挂起工作线程）
    HANDLE readers[20];
    for (int i = 0; i < _countof(readers); ++i) {
        readers[i] = (HANDLE)_beginthreadex(NULL, 0, ReaderThread, (void*)i, 
            CREATE_SUSPENDED, NULL);
    }

    HANDLE writers1[5];
    for (int i = 0; i < _countof(writers1); ++i) {
        writers1[i] = (HANDLE)_beginthreadex(NULL, 0, WriterThread1, (void*)i, 
            CREATE_SUSPENDED, NULL);
    }

    HANDLE writers2[5];
    for (int i = 0; i < _countof(writers2); ++i) {
        writers2[i] = (HANDLE)_beginthreadex(NULL, 0, WriterThread2, (void*)i, 
            CREATE_SUSPENDED, NULL);
    }

    // 恢复工作线程
    for (int i = 0; i < _countof(readers); ++i) {
        ResumeThread(readers[i]);
    }
    waiter.wait(20000); // 休眠五秒
    
    for (int i = 0; i < _countof(writers1); ++i) {
        ResumeThread(writers1[i]);
    }
    waiter.wait(5000); // 休眠五秒
    for (int i = 0; i < _countof(writers2); ++i) {
        ResumeThread(writers2[i]);
    }
    waiter.wait(5000); // 休眠五秒

    // 等待工作线程结束
    WaitForMultipleObjects(_countof(readers), readers, TRUE, INFINITE);
    WaitForMultipleObjects(_countof(writers1), writers1, TRUE, INFINITE);
    WaitForMultipleObjects(_countof(writers2), writers2, TRUE, INFINITE);

    // 释放内核对象句柄
    for (int i = 0; i < _countof(readers); ++i) {
        CloseHandle(readers[i]);
    }

    for (int i = 0; i < _countof(writers1); ++i) {
        CloseHandle(writers1[i]);
    }

    for (int i = 0; i < _countof(writers2); ++i) {
        CloseHandle(writers2[i]);
    }

    ::DeleteCriticalSection(&g_cs);

    cout << ">> Expected data value is " << 0 << ", and the real value is " << g_data << endl;

    return 0;
}

