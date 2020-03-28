# SVN dump format

本文档描述 SVN 的 dump 文件格式

参考资料：https://svn.apache.org/repos/asf/subversion/trunk/notes/dump-load-format.txt

# dump 文件格式

dump文件四类记录组成（record），每个 record 是一个 rfc822 的描述块：
  多个 header line，空行，可选的body，每个header line是 ·key: value\n ·的格式。如果有body，后面再加一个空行。

1. version stamp，本dump文件格式版本，目前是3
1. UUID，本dump文件来源的SVN仓库的UUID，版本2以上提供
1. Revision，多行，以 Revision-number 开头，表示一个版本，body一般是版本的property信息
1. Node，多行，以 Node-path 开头，表示版本中的一个修改内容，例如修改一个文件，添加目录，删除文件等

## （1） version record

header 只有一个 SVN-fs-dump-format-version ，没有 body ，用来记录版本，目前一般是版本3，例子：
···
SVN-fs-dump-format-version: 3
···

## （2）UUID record

header 只有一个 UUID ，没有body，例子：
···
UUID: 16f281d4-52cc-445c-b915-d51ca099f0d6
···

## （3）Revision record

Revision 表示一个记录，header 一般有三个，body 一般是 property 信息

header一般包括：
* Revision-number，版本号，放在第一行，
* Prop-content-length，属性的大小，由于都是属性，所以和body的大小应该一样
* Content-length，后面body的大小，方便纯RFC822的解析，

body里面一般是property信息，最后以 PROPS-END 结尾，一般包括：
* svn:author
* svn:date
* svn:log

例子：
···
Revision-number: 104601
Prop-content-length: 502
Content-length: 502

K 10
svn:author
V 12
xxxxxxxxxxxx
K 8
svn:date
V 27
2020-02-11T06:36:09.978603Z
K 7
svn:log
V 13
yyyyyyyyyyyyy
PROPS-END
···

## （4）Node record

Node 表示一个修改内容，即一个文件或者目录的增删改，header 一般包括下列信息：
* Node-path，修改内容路径，在SVN库里面的路径
* Node-kind，节点类型，文件（file）或者目录（dir），可以省略（action是删除时）
* Node-action，修改动作，修改（change），添加（add），删除（delete），覆盖（replace）
* Node-copyfrom-path，如果是svn copy过来的，描述来源的原地址路径
* Node-copyfrom-rev，如果是svn copy过来的，描述来源的版本
* Text-copy-source-md5，如果是svn copy过来的，描述数据源的md5，用于校验数据源一致性
* Text-copy-source-sha1，如果是svn copy过来的，描述数据源的sha1，用于校验数据源的一致性
* Text-content-length，本版本的修改内容的长度，一般就是body的大小，
* Text-content-md5，本版本的修改内容的md5，还原后的内容，用来校验修改内容
* Text-content-sha1，本版本的修改内容的sha1，用来校验修改内容
* Prop-content-length，属性内容的长度，如果这个修改本身是修改property本身
* Content-length，后面body的大小，方便纯RFC822的解析
    
下面这些是format 3新增的，支持delta格式：
* Text-delta: 描述文件内容是否delta格式，默认false，
* Prop-delta: 描述property内容是否delta格式，默认false，
* Text-delta-base-md5: 如果文件内容是delta格式，这里描述差异的源数据的md5
* Text-delta-base-sha1: 如果文件内容是delta格式，这里描述差异的源数据的sha1

body 是修改的内容，要么是文件的完整内容，要么是文件的delta修改差异。

# body 未 property 的格式

body 是 property 时，包含多个 property ，每个property一般是4行，最后一行 PROPS-END 表示结束。4行内容为：
* K <len>，描述下一行的key的内容的长度
* 描述key的内容，长度如上面所述
* V <len>，描述下一行的value的内容的长度
* 描述value的内容，长度如上述所述

版本3支持新的描述 D ，描述删除property，只有两行，D类似K，但没有V，

# 其他说明

add时候，文件或目录不能已经存在，而其他操作，文件或目录不能不存在

copy和text，是可以同时存在的。先copy过来，再在copy上修改。

replace是先delete再add（可以是copy的add），可以同时带修改内容。

根目录不允许change和删除

delta的差异的源，可以是copy过来的版本，默认是同一个路径的上一个版本。

