// parse_dump_file.cpp : parse dump file
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
#include "SvnDiffParser.h"

#include <dgn/dgn.h>
#include <dgn/Logger.h>

#include <stdio.h>

using namespace svnftk;

int ParseDump( DumpFile * fp );

int main( int argc, char * argv[] )
{
	const char * fname = "a.dump";
	if( argc >= 2 )
		fname = argv[1];

	dgn::DgnLib::Init( argc, argv );
	
	DumpFile dumpfp;
	if( dumpfp.Open( fname ) >= 0 ) {
		ParseDump( &dumpfp );
		dumpfp.Close();
	}
	else {
		printf( "Open [%s] failed\n", fname );
	}

	dgn::DgnLib::Fini();
	
	return 0;
}

int ParseDump( DumpFile * fp )
{
	const DRecord * drd = NULL;
	while( (drd = fp->Read()) != NULL ) {
		printf( "[DRD] type %d ( %s )\n", drd->GetType(), drd->GetTypeStr() );
		printf( "  curr pos %lld, head len %d, body len %lld\n", 
			(long long)fp->GetCurrPos(), fp->GetHeadLen(), (long long)drd->GetBodyLen() );
		if( drd->GetType() == DRD_TYPE_VERSION ) {
			const DRecordVersion * drd_v = (const DRecordVersion *)drd;
			printf( "  version %d\n", drd_v->m_version );
		}
		else if( drd->GetType() == DRD_TYPE_UUID ) {
			const DRecordUUID * drd_u = (const DRecordUUID *)drd;
			printf( "  uuid [%s]\n", drd_u->m_uuid.Str() );
		}
		else if( drd->GetType() == DRD_TYPE_REVISION ) {
			const DRecordRevision * drd_r = (const DRecordRevision *)drd;
			printf( "  Revision-number [%lld]\n", (long long)drd_r->m_revnum );
			printf( "  Prop-content-length [%lld]\n", (long long)drd_r->m_prop_len );
			printf( "  Content-length [%lld]\n", (long long)drd_r->GetBodyLen() );
		}
		else if( drd->GetType() == DRD_TYPE_NODE ) {
			const DRecordNode * drd_n = (const DRecordNode *)drd;
			printf( "  Node-path [%s]\n", drd_n->m_path.Str() );
			printf( "  Node-kind [%s]\n", DRecord::GetNodeKindStr(drd_n->m_kind) );
			printf( "  Node-action [%s]\n", DRecord::GetNodeActionStr(drd_n->m_action) );
			printf( "  Node-copyfrom-path [%s]\n", drd_n->m_copyfrom_path.Str() );
			printf( "  Node-copyfrom-rev [%lld]\n", (long long)drd_n->m_copyfrom_rev );
			printf( "  Text-copy-source-md5 [%s]\n", drd_n->m_copy_source_md5.Str() );
			printf( "  Text-copy-source-sha1 [%s]\n", drd_n->m_copy_source_sha1.Str() );
			printf( "  Text-content-length [%lld]\n", (long long)drd_n->m_text_len );
			printf( "  Text-content-md5 [%s]\n", drd_n->m_text_md5.Str() );
			printf( "  Text-content-sha1 [%s]\n", drd_n->m_text_sha1.Str() );
			printf( "  Prop-content-length [%lld]\n", (long long)drd_n->m_prop_len );
			printf( "  Text-delta [%s]\n", DRecord::GetNodeBoolStr(drd_n->m_text_delta) );
			printf( "  Prop-delta [%s]\n", DRecord::GetNodeBoolStr(drd_n->m_prop_delta) );
			printf( "  Text-delta-base-md5 [%s]\n", drd_n->m_delta_base_md5.Str() );
			printf( "  Text-delta-base-sha1 [%s]\n", drd_n->m_delta_base_sha1.Str() );
			printf( "  Content-length [%lld]\n", (long long)drd_n->GetBodyLen() );

			if( drd_n->m_text_delta == DRD_NODE_BOOL_TRUE ) {
				char buf[4096];
				int64_t diffoff = fp->GetCurrPos() + fp->GetHeadLen() 
					+ (drd_n->m_prop_len > 0 ? drd_n->m_prop_len : 0);
				int len2 = fp->ReadBody( diffoff, buf, 4095 );
				if( len2 < 0 ) {
					printf( "  [ERR] read body at %lld failed", (long long)diffoff );
					continue;
				}
				// FIXME : 超过4k的text会出错，每个window都应该单独read buffer，最好是做成一个 buffer view
				if( len2 > (int)drd_n->m_text_len )
					len2 = (int)drd_n->m_text_len;
				buf[len2] = '\0';
				DiffParser diffp;
				if( diffp.Parse( buf, len2 ) < 0 ) {
					printf( "  [ERR] parse failed" );
					continue;
				}
				printf( "  SVN diff ver %d\n", diffp.GetVer() );
				DiffWindow * win = NULL;
				while( (win = diffp.Read()) != NULL ) {
					printf( "  [win] len %lld, src off/len %lld / %lld, dst len %lld\n",
						(long long)win->GetWinLen(), (long long)win->GetSrcOff(), (long long)win->GetSrcLen(),
						(long long)win->GetDstLen() );
					printf( "    oper len %lld, new data len %lld, oper off %lld, new data off %lld\n",
						(long long)win->GetOperLen(), (long long)win->GetNewdataLen(),
						(long long)win->GetOperOff(), (long long)win->GetNewdataOff() );
					const DiffOper * oper = NULL;
					while( (oper = win->ReadOper()) != NULL ) {
						printf( "    oper type [%s], len %lld, off %lld, oper len %d\n",
							DiffOper::GetDiffOperTypeStr( oper->m_oper_type ), (long long)oper->m_len,
							(long long)oper->m_off, oper->m_oper_len );
					}
				}
			}
		}
		else if( drd->GetType() == DRD_TYPE_UNKNOWN ) {
		}
		else {
		}
	}
	return 0;
}

