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

#include "SvnDiffParser.h"

BEGIN_NS_SVNFTK
////////////////

int DiffParser::ReadIntV( const char * buf, int64_t * val )
{
	*val = 0;
	const uint8_t * p = (const uint8_t *)buf;
	while( 42 )
	{
		*val = (*val << 7) + ((*p) & 0x7F);
		if( ((*p) & 0x80) == 0 ) {
			return (int)((const char *)p - buf) + 1;
		}
		++p;
	}
	// never here
	return 0;
}

DiffParser::DiffParser()
	: m_buf( NULL )
	, m_len( 0 )
	, m_ver( 0 )
	, m_win_pos( 0 )
	, m_win_len( 0 )
	, m_win( NULL )
{
}

DiffParser::~DiffParser()
{
	Reset();
}

int DiffParser::Parse(  const char * buf, int64_t len )
{
	Reset();
	// TODO : overflow test, buf should end with '\0'
	if( buf == NULL || len < 4 )
		return -1;
	if( buf[0] != 'S' || buf[1] != 'V' || buf[2] != 'N' || ((uint8_t)buf[3]) > 2 )
		return -1;
	
	m_buf = buf;
	m_len = len;
	m_ver = (uint8_t)buf[3];
	m_win_pos = 4;
	m_win_len = 0;
	return 0;
}

int DiffParser::Reset()
{
	m_buf = NULL;
	m_len = 0;
	m_win_pos = 0;
	m_win_len = 0;
	if( m_win!= NULL )
		delete m_win, m_win = NULL;
	return 0;
}

DiffWindow * DiffParser::Read()
{
	if( m_win_pos + m_win_len >= m_len )
		return NULL;
	if( m_win == NULL )
		m_win = new DiffWindow();
	int64_t len = m_win->Parse( m_buf + m_win_pos + m_win_len, m_len - m_win_pos - m_win_len );
	if( len < 0 )
		return NULL;
	if( m_win_pos + m_win_len + len > m_len )
		return NULL;
	m_win_pos += m_win_len;
	m_win_len = len;
	return m_win;
}

DiffWindow::DiffWindow()
	: m_buf( NULL )
	, m_win_len( 0 )
	, m_src_off( 0 )
	, m_src_len( 0 )
	, m_dst_len( 0 )
	, m_oper_len( 0 )
	, m_newdata_len( 0 )
	, m_oper_off( 0 )
	, m_newdata_off( 0 )
	, m_curr_oper_off( 0 )
	, m_oper( NULL )
{
}

DiffWindow::~DiffWindow()
{
	Reset();
}

int DiffWindow::Parse(  const char * buf, int64_t len )
{
	// TODO : overflow test, buf should end with '\0'
	if( buf == NULL || len < 7 )
		return -1;
	Reset();

	int clen = 0;
	int len2 = 0;
	len2 = DiffParser::ReadIntV( buf + clen, &m_src_off );
	clen += len2;
	len2 = DiffParser::ReadIntV( buf + clen, &m_src_len );
	clen += len2;
	len2 = DiffParser::ReadIntV( buf + clen, &m_dst_len );
	clen += len2;
	len2 = DiffParser::ReadIntV( buf + clen, &m_oper_len );
	clen += len2;
	len2 = DiffParser::ReadIntV( buf + clen, &m_newdata_len );
	clen += len2;

	m_oper_off = clen;
	m_newdata_off = clen + m_oper_len;
	m_win_len = m_newdata_off + m_newdata_len;

	m_curr_oper_off = m_oper_off;

	m_buf = buf;
	m_win_len = len;

	return m_win_len;	
}

int DiffWindow::Reset()
{
	m_buf = NULL;
	m_win_len = 0;
	m_src_off = 0;
	m_src_len = 0;
	m_dst_len = 0;
	m_oper_len = 0;
	m_newdata_len = 0;
	m_oper_off = 0;
	m_newdata_off = 0;
	m_curr_oper_off = 0;
	if( m_oper != NULL )
		delete m_oper, m_oper = NULL;
	return 0;
}

const DiffOper * DiffWindow::ReadOper()
{
	int64_t m_next_oper_off = m_curr_oper_off + ( m_oper == NULL ? 0 : m_oper->m_oper_len );
	if( m_next_oper_off >= m_oper_off + m_oper_len )
		return NULL;
	if( m_oper == NULL )
		m_oper = new DiffOper();
	const uint8_t * p = (const uint8_t *)m_buf + m_next_oper_off;
	m_oper->m_oper_type = (DiffOperType_e)((*p >> 6) & 0x03);
	if( m_oper->m_oper_type == DIFF_OPER_TYPE_RESERVED )
		return NULL;
	m_curr_oper_off = m_next_oper_off;
	m_oper->m_len = ((*p) & 0x3F);
	m_oper->m_oper_len = 1;
	p += 1;
	if( m_oper->m_len == 0 ) {
		int len2 = DiffParser::ReadIntV( (const char *)p, &m_oper->m_len );
		m_oper->m_oper_len += len2;
		p += len2;
	}
	if( m_oper->m_oper_type == DIFF_OPER_TYPE_COPY_SRC || m_oper->m_oper_type == DIFF_OPER_TYPE_COPY_DST ) {
		int len2 = DiffParser::ReadIntV( (const char *)p, &m_oper->m_off );
		m_oper->m_oper_len += len2;
		p += len2;
	}
	else {
		m_oper->m_off = 0;
	}
	return m_oper;
}

static const char * s_DiffOperType_Str[4] = { "CopySrc", "CopyDst", "CopyNewdata", "(Reserved)" };

const char * DiffOper::GetDiffOperTypeStr( DiffOperType_e type )
{
	return s_DiffOperType_Str[type];
}

////////////////
END_NS_SVNFTK

