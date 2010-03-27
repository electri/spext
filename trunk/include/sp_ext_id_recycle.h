/** Copyright (c) 2008-2009
* All rights reserved.
* 
* 文件名称:	sp_ext_id_recycle.h 
* 摘	 要:    id回收器
* 
* 当前版本：	1.0
* 作	 者:	陈学术
* 操	 作:	新建
* 完成日期:		2009年3月28日
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
	/// 创建一个新的ID
	int create_new_id(void);

	/// 回收一个ID
	void recycle_id(int nID);

	/// 清空回收站，nID为新的开始ID
	void reset(int nID);

protected:
	DWORD	inc_id_; // 唯一标识

	ptr_array array_recycle_; // 回收站
	sp_ext_mutex recycle_lock; // 回收安全

};

} // sp_ext

#endif // __SP_EXT_ID_RECYCLE_H__
