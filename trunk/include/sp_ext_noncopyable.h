/** Copyright (c) 2008-2009
 * All rights reserved.
 * 
 * 文件名称:	sp_ext_noncopyable.h  
 * 摘	 要:	使子类禁止拷贝构造和赋值拷贝
 * 
 * 当前版本:	1.0
 * 作	 者:	陈学术
 * 操	 作:	新建
 * 完成日期:	2010年09月07日
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
