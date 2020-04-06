// SvnDiffParser.h : Svn Diff Parser
// Copyright (C) 2020 ~ 2020 drangon <drangon.zhou (at) gmail.com>
//
// This program is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.

#ifndef INCLUDED_SVNDIFFPARSER_H
#define INCLUDED_SVNDIFFPARSER_H

#include "SvnFtk.h"

#include <dgn/CStr.h>
#include <dgn/File.h>

BEGIN_NS_SVNFTK
////////////////

class DiffWindow;
class DiffOper;

// 负责解析一个 SVN Diff 数据 buffer ，只引用这个 buffer，不持有内存
class DiffParser
{
public:
	static int ReadIntV( const char * buf, int64_t * val );
	
	DiffParser();
	~DiffParser();

public:
	// 解析一个 buffer ，一直引用，直到 Reset()
	int Parse(  const char * buf, int64_t len );
	int Reset();

	int GetVer() const { return m_ver; }

	int64_t GetCurrPos() const { return m_win_pos; }
	int64_t GetWinLen() const { return m_win_len; }
	int64_t GetNextPos() const { return m_win_pos + m_win_len; }
	DiffWindow * Read();
	
protected:
	const char * m_buf;
	int64_t m_len;
	int m_ver;
	int64_t m_win_pos;
	int64_t m_win_len;
	DiffWindow * m_win;
};

class DiffWindow
{
public:
	DiffWindow();
	~DiffWindow();

public:
	// 只解析Oper，不解析NewData
	int Parse(  const char * buf, int64_t len );
	int Reset();

	int64_t GetWinLen() const { return m_win_len; }
	int64_t GetSrcOff() const { return m_src_off; }
	int64_t GetSrcLen() const { return m_src_len; }
	int64_t GetDstLen() const { return m_dst_len; }
	int64_t GetOperLen() const { return m_oper_len; }
	int64_t GetNewdataLen() const { return m_newdata_len; }

	int64_t GetOperOff() const { return m_oper_off; }
	int64_t GetNewdataOff() const { return m_newdata_off; }

	const DiffOper * ReadOper();
	
protected:
	const char * m_buf;
	int64_t m_win_len;
	
	int64_t m_src_off;
	int64_t m_src_len;
	int64_t m_dst_len;
	int64_t m_oper_len;
	int64_t m_newdata_len;
	
	int64_t m_oper_off;
	int64_t m_newdata_off;

	int64_t m_curr_oper_off;
	
	DiffOper * m_oper;
};

enum DiffOperType_e
{
	DIFF_OPER_TYPE_COPY_SRC = 0,
	DIFF_OPER_TYPE_COPY_DST = 1,
	DIFF_OPER_TYPE_COPY_NEWDATA = 2,
	DIFF_OPER_TYPE_RESERVED = 3,
};

class DiffOper
{
public:
	static const char * GetDiffOperTypeStr( DiffOperType_e type );
	
	DiffOper( DiffOperType_e type = DIFF_OPER_TYPE_RESERVED, int64_t len = 0, int64_t off = 0, int oper_len = 0 ) 
		: m_oper_type( type ), m_len( len ), m_off( off ), m_oper_len( oper_len ) { }
	~DiffOper() { }

	DiffOperType_e m_oper_type;
	int64_t m_len;
	int64_t m_off;  // 对于 DIFF_OPER_TYPE_COPY_NEWDATA ， offset 是隐含的，置 0
	int m_oper_len; // oper 本身的大小
};

////////////////
END_NS_SVNFTK

#endif // INCLUDED_SVNDIFFPARSER_H

