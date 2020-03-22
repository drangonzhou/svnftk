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

// svn dump record
class SDRecord;

// 负责读写一个 svn dump 文件，一个 svn dump 文件由一系列的 SDRecord 组成
// 支持顺序写，顺序读，以及知道准确位置的 seek 读，
class SvnDumpFile
{
public:
	SvnDumpFile();
	~SvnDumpFile();

public:
	int Open( const char * fname );
	int Close();

	// seek 到特定的 SDRecord 位置，用于读取指定 SDRecord，pos 需要在 SDReord 边界，否则会分析出错
	int Seek( int64_t pos );

	// 读取 SDRecord ，失败时返回NULL
	SDRecord * Read();

	// 写入 SDRecord ，
	int Write( SDRecord * rd );
};

class SDRecord
{
public:
	SDRecord();
	virtual ~SDRecord();
};

#endif // INCLUDED_SVNDUMPFILE_H

