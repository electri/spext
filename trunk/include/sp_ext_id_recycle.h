/** Copyright (c) 2008-2009
* All rights reserved.
* 
* �ļ�����:	sp_ext_id_recycle.h 
* ժ	 Ҫ:    id������
* 
* ��ǰ�汾��	1.0
* ��	 ��:	��ѧ��
* ��	 ��:	�½�
* �������:		2009��3��28��
*/
#ifndef __SP_EXT_ID_RECYCLE_H__
#define __SP_EXT_ID_RECYCLE_H__

#include "sp_ext_array.h"
#include "sp_ext_thread.h"

namespace sp_ext
{

class id_recycle 
{
public:
	id_recycle();
	virtual ~id_recycle();

public:
	/// ����һ���µ�ID
	int create_new_id(void);

	/// ����һ��ID
	void recycle_id(int nID);

	/// ��ջ���վ��nIDΪ�µĿ�ʼID
	void reset(int nID);

protected:
	DWORD	inc_id_; // Ψһ��ʶ

	ptr_array array_recycle_; // ����վ
	sp_ext_mutex recycle_lock; // ���հ�ȫ

};

} // sp_ext

#endif // __SP_EXT_ID_RECYCLE_H__
