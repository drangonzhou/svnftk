# Makefile for utils
# Copyright (C) 2020 ~ 2020 drangon zhou <drangon.zhou (at) gmail.com>
#
# This program is free software: you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program; if not, see <http://www.gnu.org/licenses/>.

HOST_TYPE ?= linux
OPT_LIB_DIR ?= /home/drangon/opt

CXX=g++

SRCS=$(wildcard *.cpp)

TARGETS:=$(foreach src, $(SRCS), $(basename $(src)).exe)

CFLAGS = -g -Wall -pipe -fno-rtti -fno-exceptions 
CFLAGS += -I../libdgn -I../libsvnftk -I$(OPT_LIB_DIR)/include

LDFLAGS = -Wl,-rpath,./ -L../libsvnftk -L../libdgn -lsvnftk -ldgnbase

mingw32_LDFLAGS = -lwinmm -lws2_32
mingw64_LDFLAGS = $(mingw32_LDFLAGS)
linux_LDFLAGS = -lpthread

LDFLAGS += $($(HOST_TYPE)_LDFLAGS)

.PHONY : all test clean
	
all : $(TARGETS) 

$(TARGETS) : %.exe : %.cpp
	$(CXX) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean :
	rm -f $(TARGETS) *.o

$(TARGETS) : ./libsvnftk.so ./libdgnbase.so 

./libsvnftk.so : ../libsvnftk/libsvnftk.so
	cp ../libsvnftk/libsvnftk.so ./

./libdgnbase.so : ../libdgn/libdgnbase.so
	cp ../libdgn/libdgnbase.so ./

