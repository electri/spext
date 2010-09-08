/** Copyright (c) 2008-2009
 * All rights reserved.
 * 
 * �ļ�����:	sp_ext_noncopyable.h  
 * ժ	 Ҫ:	ʹ�����ֹ��������͸�ֵ����
 * 
 * ��ǰ�汾:	1.0
 * ��	 ��:	��ѧ��
 * ��	 ��:	�½�
 * �������:	2010��09��07��
 */
#ifndef __SP_EXT_NONCOPYABLE_H__
#define __SP_EXT_NONCOPYABLE_H__

namespace sp_ext
{

class noncopyable
{
protected:
    // disallow instantiation
    noncopyable() {}
    ~noncopyable() {}

private:
    // forbid copy constructor & copy assignment operator
    noncopyable(const noncopyable&);
    noncopyable& operator= (const noncopyable&);
};

} // sp_ext
#endif // __SP_EXT_NONCOPYABLE_H__
