# SVN diff format

本文档描述 SVN 的 diff 数据格式，在 dump 和 repo 数据文件中，会用到这个 diff 格式

参考资料：http://svn.apache.org/repos/asf/subversion/trunk/notes/svndiff

# diff 数据格式

## 整体格式

diff 数据由 4 字节数据头，以及 1 到多个 window 组成

内部数据中的一些整型数据，采用可变长整型（后面简称 int-v 格式），具体编码方式如下：
采用 big-endian 方式，高字节在前，每个字节的低 7 bit是有效数据，而最高位如果为1，表示后面还有数据。

## 4字节数据头

4字节数据头，前三字节是 'SVN' ，第四字节是版本号：
* 版本 0 ，表示 window 里面的 instructions 和 new data 未压缩，
* 版本 1 ，表示 window 里面的 instructions 和 new data 使用 zlib 进行压缩，
* 版本 2 ，表示 window 里面的 instructions 和 new data 使用 lz4 进行压缩，

## window 数据格式

每个 window 依次是下列数据：
1. source view offset，int-v 类型，源数据的文件偏移
1. source view length，int-v 类型，源数据的长度
1. target view length，int-v 类型，目标数据的长度，（默认偏移是接着上一个window）
1. instructions length，int-v 类型，后面的 instructions 数据的长度
1. new data length，int-v 类型，后面的 new data 的长度
1. instructions 数据，二进制串，描述具体的差异操作，具体格式见后面。<p>
   如果是 V1/V2 压缩格式，则先是 int-v 类型的原始数据长度，然后是压缩后的二进制数据
1. new data 数据，二进制串，差异操作中 op 为 10 时使用到的新数据。<p>
   如果是 V1/V2 压缩格式，则先是 int-v 类型的原始数据长度，然后是压缩后的二进制数据

instructions 格式，根据第一个字节的高两位来决定操作的类型，不同操作类型后面跟不同的参数
* 00 ，从source view复制，低6bit是长度，如果为0，则增加一个int-v表示长度，然后是一个int-v表示offset
* 01 ，从target view复制，低6bit是长度，如果为0，则增加一个int-v表示长度，然后是一个int-v表示offset
* 10 ，从new data复制，低6bit是长度，如果为0，则增加一个int-v表示长度，offset是隐含的，从上次复制结束地方开始
* 11 ，保留未用
