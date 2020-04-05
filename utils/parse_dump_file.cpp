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

static const char * s_NodeKind_Str[4] = { "invalid", "unset", "file", "dir" };
static const char * s_NodeAction_Str[6] = { "invalid", "unset", "add", "change", "delete", "replace" };
static const char * s_NodeBool_Str[4] = { "invalid", "unset", "true", "false" };

int ParseDump( DumpFile * fp )
{
	const DRecord * drd = NULL;
	while( (drd = fp->Read()) != NULL ) {
		printf( "[DRD] type %d ( %s )\n", drd->GetType(), drd->GetTypeStr() );
		printf( "  body len %lld\n", (long long)drd->GetBodyLen() );
		if( drd->GetType() == DRecordType_Version ) {
			const DRecordVersion * drd_v = (const DRecordVersion *)drd;
			printf( "  version %d\n", drd_v->m_version );
		}
		else if( drd->GetType() == DRecordType_UUID ) {
			const DRecordUUID * drd_u = (const DRecordUUID *)drd;
			printf( "  uuid [%s]\n", drd_u->m_uuid.Str() );
		}
		else if( drd->GetType() == DRecordType_Revision ) {
			const DRecordRevision * drd_r = (const DRecordRevision *)drd;
			printf( "  Revision-number [%lld]\n", (long long)drd_r->m_revnum );
			printf( "  Prop-content-length [%lld]\n", (long long)drd_r->m_prop_len );
			printf( "  Content-length [%lld]\n", (long long)drd_r->GetBodyLen() );
		}
		else if( drd->GetType() == DRecordType_Node ) {
			const DRecordNode * drd_n = (const DRecordNode *)drd;
			printf( "  Node-path [%s]\n", drd_n->m_path.Str() );
			printf( "  Node-kind [%s]\n", s_NodeKind_Str[drd_n->m_kind] );
			printf( "  Node-action [%s]\n", s_NodeAction_Str[drd_n->m_action] );
			printf( "  Node-copyfrom-path [%s]\n", drd_n->m_copyfrom_path.Str() );
			printf( "  Node-copyfrom-rev [%lld]\n", (long long)drd_n->m_copyfrom_rev );
			printf( "  Text-copy-source-md5 [%s]\n", drd_n->m_copy_source_md5.Str() );
			printf( "  Text-copy-source-sha1 [%s]\n", drd_n->m_copy_source_sha1.Str() );
			printf( "  Text-content-length [%lld]\n", (long long)drd_n->m_text_len );
			printf( "  Text-content-md5 [%s]\n", drd_n->m_text_md5.Str() );
			printf( "  Text-content-sha1 [%s]\n", drd_n->m_text_sha1.Str() );
			printf( "  Prop-content-length [%lld]\n", (long long)drd_n->m_prop_len );
			printf( "  Text-delta [%s]\n", s_NodeBool_Str[drd_n->m_text_delta] );
			printf( "  Prop-delta [%s]\n", s_NodeBool_Str[drd_n->m_prop_delta] );
			printf( "  Text-delta-base-md5 [%s]\n", drd_n->m_delta_base_md5.Str() );
			printf( "  Text-delta-base-sha1 [%s]\n", drd_n->m_delta_base_sha1.Str() );
			printf( "  Content-length [%lld]\n", (long long)drd_n->GetBodyLen() );
		}
		else if( drd->GetType() == DRecordType_Unknown ) {
		}
		else {
		}
	}
	return 0;
}

