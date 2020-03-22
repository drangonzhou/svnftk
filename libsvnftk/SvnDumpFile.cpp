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

SvnDumpFile::SvnDumpFile()
{
}

SvnDumpFile::~SvnDumpFile()
{
}

int SvnDumpFile::Open( const char * fname )
{
	return -1;
}

int SvnDumpFile::Close()
{
	return -1;
}

int SvnDumpFile::Seek( int64_t pos )
{
	return -1;
}


SDRecord * SvnDumpFile::Read()
{
	return NULL;
}


int SvnDumpFile::Write( SDRecord * rd )
{
	return -1;
}

SDRecord::SDRecord()
{
}

SDRecord::~SDRecord()
{
}

