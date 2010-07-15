//#include "stdafx.h"
#include "sp_ext_id_recycle.h"
#include <windows.h>

namespace sp_ext
{

id_recycle::id_recycle()
{
	inc_id_ = 1000;
}

id_recycle::~id_recycle()
{
}

/// ����һ���µ�ID
int id_recycle::create_new_id(void)
{
	int nNewID;
	recycle_lock.lock();
	int nCount = array_recycle_.GetSize();

	if ( nCount <= 0 )
		nNewID = inc_id_++;
	else
	{
		nNewID = (int)array_recycle_.GetAt( nCount - 1);
		array_recycle_.RemoveAt(nCount - 1);
	}
	recycle_lock.unlock();
	return nNewID;
}

/// ����һ��ID
void id_recycle::recycle_id(int nID)
{
	recycle_lock.lock();
	array_recycle_.Add((void*)nID);
	recycle_lock.unlock();
}

/// ��ջ���վ
void id_recycle::reset(int nID)
{
	recycle_lock.lock();
	array_recycle_.RemoveAll();
	inc_id_ = nID;
	recycle_lock.unlock();
}

}
