// SvnDumpFile.cpp : SvnDumpFile
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

#include "SvnDumpFile.h"

BEGIN_NS_SVNFTK
////////////////

DumpFile::DumpFile() : m_nextpos(0), m_rd(NULL)
{
}

DumpFile::~DumpFile()
{
	Close();
}

int DumpFile::Open( const char * fname )
{
	m_nextpos = 0;
	return m_fp.Open( fname );
}

int DumpFile::Close()
{
	m_fp.Close();
	m_nextpos = 0;
	if( m_rd != NULL )
		delete m_rd, m_rd = NULL;
	return 0;
}

int DumpFile::Seek( int64_t pos )
{
	if( m_fp.Seek( pos ) < 0 )
		return -1;
	m_nextpos = pos;
	return 0;
}

const DRecord * DumpFile::Read()
{
	// TODO : 暂不支持 header 超过 4k
	char buf[4096];
	int len = 0;
	if( m_fp.Seek( m_nextpos ) < 0 )
		return NULL;
	len = m_fp.Read( buf, 4096 - 1 );
	if( len < 0 )
		return NULL;
	buf[len] = '\0';

	// skip empty line
	int headlen = 0;
	dgn::CStr key, val;
	while( buf[headlen] == '\n' )
		headlen++;
	len = DRecord::ParseLine(buf + headlen, &key, &val );
	if( len < 0 ) {
		return NULL;
	}
	headlen += len;

	DRecoreType_e type = DRecord::FindType( key );
	if( type == DRecordType_Unknown )
		return NULL;
	if( type != m_rd->GetType() ) {
		delete m_rd;
		m_rd = DRecord::Create( type );
	}
	if( m_rd->SetFirstVal( val ) < 0 ) 
		return NULL;
	
	len = m_rd->ParseBuf( buf + headlen );
	if( len < 0 )
		return NULL;
	headlen += len;
	m_nextpos += headlen + m_rd->GetBodyLen();
	return m_rd;
}

int DumpFile::ReadBody( int64_t pos, char * buf, int len )
{
	if( m_fp.Seek( pos ) < 0 )
		return -1;

	return m_fp.Read( buf, len );
}

int DumpFile::Write( const DRecord * rd )
{
	return rd->Write( &m_fp );
}

int DumpFile::WriteBody( const char * buf, int len )
{
	return m_fp.Write( buf, len );
}

int DRecord::ParseLine( const char * buf, dgn::CStr * key, dgn::CStr * val )
{
	const char * p = NULL;
	const char * p2 = NULL;
	size_t n = 0;
	
	p = buf;
	while( *p == ' ' || *p == '\t' )
		++p;
	n = strcspn( p, ":\n" );
	if( n == 0 || *(p + n) != ':' ) // key is empty or ':' not found
		return -1;
	// strip key
	p2 = p + n - 1;
	while( *p2 == ' ' || *p2 == '\t' )
		--p2;
	key->Assign( p, p2 - p + 1 );

	p += n + 1;  // skip ':'
	while( *p == ' ' || *p == '\t' )
		++p;
	p2 = strchr( p, '\n' );
	if( p2 == NULL )
		return -1;
	n = p2 - buf + 1; // include last '\n'
	if( p2 == p ) {
		val->Assign( "" );
	}
	else {
		while( *p2 == ' ' || *p2 == '\t' )
			--p2;
		val->Assign( p, p2 - p + 1 );
	}
	
	return n;
}

DRecoreType_e DRecord::FindType( const dgn::CStr & key )
{
	if( key == "SVN-fs-dump-format-version" ) {
		return DRecordType_Version;
	}
	else if( key == "UUID" ) {
		return DRecordType_UUID;
	}
	else if( key == "Revision-number" ) {
		return DRecordType_UUID;
	}
	else if( key == "Node-path" ) {
		return DRecordType_UUID;
	}
	return DRecordType_Unknown;
}

DRecord * DRecord::Create( DRecoreType_e type )
{
	switch( type ) {
	case DRecordType_Version :
		return new DRecordVersion();
		break;
	case DRecordType_UUID :
		return new DRecordUUID();
		break;
	case DRecordType_Revision :
		return new DRecordRevision();
		break;
	case DRecordType_Node :
		return new DRecordNode();
		break;
	default :
		return NULL;
		break;
	}
	return NULL;
}

DRecord::DRecord( DRecoreType_e type ) : m_type( type ), m_body_len( 0 )
{
}

DRecord::~DRecord()
{
}

DRecordVersion::DRecordVersion() : DRecord( DRecordType_Version ), m_version( 0 )
{
}

DRecordVersion::~DRecordVersion()
{
}

int DRecordVersion::SetFirstVal( const dgn::CStr & val )
{
	m_version = val.ToInt();
	return 0;
}

int DRecordVersion::ParseBuf( const char * buf )
{
	if( *buf != '\n' )
		return -1;
	return 1;
}

int DRecordVersion::Write( dgn::File * fp ) const
{
	char buf[256];
	dgn::CStr str;
	str.AttachBuffer( buf, 256 );
	str.AssignFmt( "SVN-fs-dump-format-version: %d\n\n", m_version );
	fp->Write( str.Str(), str.Len() );
	return 0;
}

DRecordUUID::DRecordUUID() : DRecord( DRecordType_UUID )
{
}

DRecordUUID::~DRecordUUID()
{
}

int DRecordUUID::SetFirstVal( const dgn::CStr & val )
{
	m_uuid = val;
	return 0;
}

int DRecordUUID::ParseBuf( const char * buf )
{
	if( *buf != '\n' )
		return -1;
	return 1;
}

int DRecordUUID::Write( dgn::File * fp ) const
{
	char buf[256];
	dgn::CStr str;
	str.AttachBuffer( buf, 256 );
	str.AssignFmt( "UUID: %s\n\n", m_uuid.Str() );
	fp->Write( str.Str(), str.Len() );
	return 0;
}

DRecordRevision::DRecordRevision() : DRecord( DRecordType_Revision ), m_revnum( -1 ), m_prop_len( -1 )
{
}

DRecordRevision::~DRecordRevision()
{
}

int DRecordRevision::SetFirstVal( const dgn::CStr & val )
{
	m_revnum = val.ToInt64();
	return 0;
}

int DRecordRevision::ParseBuf( const char * buf )
{
	int len = 0;
	dgn::CStr key, val;
	while( buf[len] != '\n' ) {
		if( buf[len] == '\0' )
			return -1;
		int len2 = DRecord::ParseLine( buf + len, &key, &val );
		if( len2 < 0 )
			return -1;
		if( key == "Prop-content-length" ) {
			m_prop_len = val.ToInt64();
		}
		else if( key == "Content-length" ) {
			SetBodyLen( val.ToInt64() );
		}
		else {
			// unknown key
			return -1;
		}
		len += len2;
	}
	return len + 1;
}

int DRecordRevision::Write( dgn::File * fp ) const
{
	char buf[512];
	dgn::CStr str;
	str.AttachBuffer( buf, 512 );
	str.AssignFmt( "Revision-number: %lld\n", (long long)m_revnum );
	if( m_prop_len != -1 ) {
		str.AppendFmt( "Prop-content-length: %lld\n", (long long)m_prop_len );
	}
	str.AppendFmt( "Content-length: %lld\n\n", (long long)GetBodyLen() );
	fp->Write( str.Str(), str.Len() );
	
	return 0;
}

DRecordNode::DRecordNode() 
	: DRecord( DRecordType_Node )
	, m_kind( DRD_NODE_KIND_UNSET )
	, m_action( DRD_NODE_ACTION_UNSET )
	, m_copyfrom_rev( -1 )
	, m_text_len( -1 )
	, m_prop_len( -1 )
	, m_text_delta( DRD_NODE_BOOL_UNSET )
	, m_prop_delta( DRD_NODE_BOOL_UNSET )
{
}

DRecordNode::~DRecordNode()
{
}

int DRecordNode::SetFirstVal( const dgn::CStr & val )
{
	m_path = val;
	return 0;
}

int DRecordNode::ParseBuf( const char * buf )
{
	int len = 0;
	dgn::CStr key, val;
	while( buf[len] != '\n' ) {
		if( buf[len] == '\0' )
			return -1;
		int len2 = DRecord::ParseLine( buf + len, &key, &val );
		if( len2 < 0 )
			return -1;
		if( key == "Node-kind" ) {
			if( val == "file" ) {
				m_kind = DRD_NODE_KIND_FILE;
			}
			else if( val == "dir" ) {
				m_kind = DRD_NODE_KIND_DIR;
			}
			else {
				return -1;
			}
		}
		else if( key == "Node-action" ) {
			if( val == "add" ) {
				m_action = DRD_NODE_ACTION_ADD;
			}
			if( val == "change" ) {
				m_action = DRD_NODE_ACTION_CHANGE;
			}
			if( val == "delete" ) {
				m_action = DRD_NODE_ACTION_DELETE;
			}
			if( val == "replace" ) {
				m_action = DRD_NODE_ACTION_REPLACE;
			}
			else {
				return -1;
			}
		}
		else if( key == "Node-copyfrom-path" ) {
			m_copyfrom_path = val;
		}
		else if( key == "Node-copyfrom-rev" ) {
			m_copyfrom_rev = val.ToInt64();
		}
		else if( key == "Text-copy-source-md5" ) {
			m_copy_source_md5 = val;
		}
		else if( key == "Text-copy-source-sha1" ) {
			m_copy_source_sha1 = val;
		}
		else if( key == "Text-content-length" ) {
			m_text_len = val.ToInt64();
		}
		else if( key == "Text-content-md5" ) {
			m_text_md5 = val;
		}
		else if( key == "Text-content-sha1" ) {
			m_text_sha1 = val;
		}
		else if( key == "Prop-content-length" ) {
			m_prop_len = val.ToInt64();
		}
		else if( key == "Content-length" ) {
			SetBodyLen( val.ToInt64() );
		}
		else if( key == "Text-delta" ) {
			if( val == "true" ) {
				m_text_delta = DRD_NODE_BOOL_TRUE;
			}
			else if( val == "false" ) {
				m_text_delta = DRD_NODE_BOOL_FALSE;
			}
			else {
				return -1;
			}
		}
		else if( key == "Prop-delta" ) {
			if( val == "true" ) {
				m_prop_delta = DRD_NODE_BOOL_TRUE;
			}
			else if( val == "false" ) {
				m_prop_delta = DRD_NODE_BOOL_FALSE;
			}
			else {
				return -1;
			}
		}
		else if( key == "Text-delta-base-md5" ) {
			m_delta_base_md5 = val;
		}
		else if( key == "Text-delta-base-sha1" ) {
			m_delta_base_sha1 = val;
		}
		else {
			// unknown key
			return -1;
		}
		len += len2;
	}
	return len + 1;
}

static const char * s_NodeKind_Str[4] = { "invalid", "unset", "file", "dir" };
static const char * s_NodeAction_Str[6] = { "invalid", "unset", "add", "change", "delete", "replace" };
static const char * s_NodeBool_Str[4] = { "invalid", "unset", "true", "false" };

int DRecordNode::Write( dgn::File * fp ) const
{
	dgn::CStr str;
	str.Reserve( 4096 );
	str.AssignFmt( "Node-path: %s\n", m_path.Str() );
	if( m_kind == DRD_NODE_KIND_FILE || m_kind == DRD_NODE_KIND_DIR ) {
		str.AppendFmt( "Node-kind: %s\n", s_NodeKind_Str[m_kind] );
	}
	if( m_action == DRD_NODE_ACTION_ADD || m_action == DRD_NODE_ACTION_CHANGE 
		|| m_action == DRD_NODE_ACTION_DELETE || m_action == DRD_NODE_ACTION_REPLACE ) {
		str.AppendFmt( "Node-action: %s\n", s_NodeAction_Str[m_action] );
	}
	if( m_copyfrom_path.Len() > 0 ) {
		str.AppendFmt( "Node-copyfrom-path: %s\nNode-copyfrom-rev: %lld\n", 
			m_copyfrom_path.Str(), (long long)m_copyfrom_rev );
	}
	if( m_copy_source_md5.Len() > 0 ) {
		str.AppendFmt( "Text-copy-source-md5: %s\n", m_copy_source_md5.Str() );
	}
	if( m_copy_source_sha1.Len() > 0 ) {
		str.AppendFmt( "Text-copy-source-sha1: %s\n", m_copy_source_sha1.Str() );
	}
	if( m_text_len != -1 ) {
		str.AppendFmt( "Text-content-length: %lld\n", (long long)m_text_len );
	}
	if( m_text_md5.Len() > 0 ) {
		str.AppendFmt( "Text-content-md5: %s\n", m_text_md5.Str() );
	}
	if( m_text_sha1.Len() > 0 ) {
		str.AppendFmt( "Text-content-sha1: %s\n", m_text_sha1.Str() );
	}
	if( m_prop_len != -1 ) {
		str.AppendFmt( "Prop-content-length: %lld\n", (long long)m_prop_len );
	}
	if( m_text_delta == DRD_NODE_BOOL_TRUE || m_text_delta == DRD_NODE_BOOL_FALSE ) {
		str.AppendFmt( "Text-delta: %s\n", s_NodeBool_Str[m_text_delta] );
	}
	if( m_prop_delta == DRD_NODE_BOOL_TRUE || m_prop_delta == DRD_NODE_BOOL_FALSE ) {
		str.AppendFmt( "Prop-delta: %s\n", s_NodeBool_Str[m_prop_delta] );
	}
	if( m_delta_base_md5.Len() > 0 ) {
		str.AppendFmt( "Text-delta-base-md5: %s\n", m_delta_base_md5.Str() );
	}
	if( m_delta_base_sha1.Len() > 0 ) {
		str.AppendFmt( "Text-delta-base-sha1: %s\n", m_delta_base_sha1.Str() );
	}
	str.AppendFmt( "Content-length: %lld\n\n", (long long)GetBodyLen() );
	fp->Write( str.Str(), str.Len() );

	return 0;
}

////////////////
END_NS_SVNFTK

