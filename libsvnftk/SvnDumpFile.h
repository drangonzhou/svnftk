// SvnDumpFile.h : SvnDumpFile
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

#ifndef INCLUDED_SVNDUMPFILE_H
#define INCLUDED_SVNDUMPFILE_H

#include "SvnFtk.h"

#include <dgn/CStr.h>
#include <dgn/File.h>

BEGIN_NS_SVNFTK
////////////////

// svn dump record
class DRecord;

// 负责读写一个 svn dump 文件，一个 svn dump 文件由一系列的 SDRecord 组成
// 支持顺序写，顺序读，以及知道准确位置的 seek 读，
class DumpFile
{
public:
	DumpFile();
	~DumpFile();

public:
	int Open( const char * fname );
	int Close();

	// seek 到特定的 DRecord 位置，用于读取指定 DRecord，pos 需要在 DReord 边界，否则会分析出错
	int Seek( int64_t pos );

	// 读取 DRecord ，失败时返回NULL, 所有权内部管理，再次Read会覆盖
	const DRecord * Read();
	int ReadBody( int64_t pos, char * buf, int len );

	// 写入 DRecord ，
	int Write( const DRecord * rd );
	// 写入 DRecord 的 body 信息，只能按顺序写
	int WriteBody( const char * buf, int len );

protected:
	dgn::File m_fp;
	int64_t m_nextpos;
	DRecord * m_rd;
};

enum DRecoreType_e
{
	DRecordType_Unknown,
	DRecordType_Version,
	DRecordType_UUID,
	DRecordType_Revision,
	DRecordType_Node,
};

class DRecord
{
public:
	// parse one line, return line length ( include last \n ), return < 0 if parse error
	static int ParseLine( const char * buf, dgn::CStr * key, dgn::CStr * val );
	static DRecoreType_e FindType( const dgn::CStr & key );
	static DRecord * Create( DRecoreType_e type );

public:
	DRecord( DRecoreType_e type );
	virtual ~DRecord();

	DRecoreType_e GetType() const { return m_type; }
	int64_t GetBodyLen() const { return m_body_len; }
	void SetBodyLen( int64_t len ) { m_body_len = len; return; }

	virtual int SetFirstVal( const dgn::CStr & val ) = 0;
	virtual int ParseBuf( const char * buf ) = 0;
	virtual int Write( dgn::File * fp ) const = 0;

private:
	DRecoreType_e m_type;
	int64_t m_body_len;
};

class DRecordVersion : public DRecord
{
public:
	DRecordVersion();
	virtual ~DRecordVersion();

	virtual int SetFirstVal( const dgn::CStr & val );
	virtual int ParseBuf( const char * buf );
	virtual int Write( dgn::File * fp ) const;

public:
	int m_version;
};

class DRecordUUID : public DRecord
{
public:
	DRecordUUID();
	virtual ~DRecordUUID();
	
	virtual int SetFirstVal( const dgn::CStr & val );
	virtual int ParseBuf( const char * buf );
	virtual int Write( dgn::File * fp ) const;

public:
	dgn::CStr m_uuid;
};

class DRecordRevision : public DRecord
{
public:
	DRecordRevision();
	virtual ~DRecordRevision();

	virtual int SetFirstVal( const dgn::CStr & val );
	virtual int ParseBuf( const char * buf );
	virtual int Write( dgn::File * fp ) const;

public:
	int64_t m_revnum;
	int64_t m_prop_len; // -1 表示未设置，property 长度，应该和 body len 一样
};

enum DRD_NodeKind_e {
	DRD_NODE_KIND_INVALID = 0,
	DRD_NODE_KIND_UNSET = 1, // action是删除时，kind可以不设置
	DRD_NODE_KIND_FILE = 2,
	DRD_NODE_KIND_DIR = 3,
};

enum DRD_NodeAction_e {
	DRD_NODE_ACTION_INVALID = 0,
	DRD_NODE_ACTION_UNSET = 1, // 不应该出现
	DRD_NODE_ACTION_ADD = 2,
	DRD_NODE_ACTION_CHANGE = 3,
	DRD_NODE_ACTION_DELETE = 4,
	DRD_NODE_ACTION_REPLACE = 5,
};

enum DRD_NodeBool_e {
	DRD_NODE_BOOL_INVALID = 0,
	DRD_NODE_BOOL_UNSET = 1,
	DRD_NODE_BOOL_TRUE = 2,
	DRD_NODE_BOOL_FALSE = 3,
};

class DRecordNode : public DRecord
{
public:
	DRecordNode();
	virtual ~DRecordNode();

	virtual int SetFirstVal( const dgn::CStr & val );
	virtual int ParseBuf( const char * buf );
	virtual int Write( dgn::File * fp ) const;

public:
	dgn::CStr m_path;
	DRD_NodeKind_e m_kind;
	DRD_NodeAction_e m_action;
	dgn::CStr m_copyfrom_path;
	int64_t m_copyfrom_rev; // -1 表示未设置
	dgn::CStr m_copy_source_md5;
	dgn::CStr m_copy_source_sha1;
	int64_t m_text_len; // -1 表示未设置，Text 内容的长度
	dgn::CStr m_text_md5;
	dgn::CStr m_text_sha1;
	int64_t m_prop_len; // -1 表示未设置，property 内容的长度
	DRD_NodeBool_e m_text_delta;
	DRD_NodeBool_e m_prop_delta;
	dgn::CStr m_delta_base_md5;
	dgn::CStr m_delta_base_sha1;
};

////////////////
END_NS_SVNFTK

#endif // INCLUDED_SVNDUMPFILE_H

