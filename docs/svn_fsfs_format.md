# SVN FSFS repo format

本文档描述 SVN 的 FSFS 仓库的目录格式

参考资料：https://svn.apache.org/repos/asf/subversion/trunk/subversion/libsvn_fs_fs/structure

# FSFS 目录格式

FSFS 仓库包含下列文件和目录：

1. **format** ，格式版本说明，一行数字，目前是 5
1. README.txt ，说明文件，固定内容 

1. **db** ，数据目录，存放 SVN 管理的数据
1. conf ，配置文件目录，用于 svnserve 服务，对其他服务（例如 apache 的 mod_dav_svn ）没有影响，
  内部文件包括 authz passwd svnserve.conf 等
1. hooks ，钩子脚本目录，存放脚本文件，在某些情况发生时调用，例如提交前（pre-commit）或者提交后（post-commit）
1. locks ，锁的目录，存放用来加锁的文件，在某些操作时，先加锁再执行操作

# db 目录格式

db 目录下包含下列文件和目录：

1. **fs-type** ，格式类型说明，一般是一行，对于 FSFS 仓库来说，内容是：fsfs
2. **format** ，格式版本说明，多行，目前是两行，第一行是版本号 4，第二行是分块大小，缺省是：layout sharded 1000
3. **fsfs.conf** ，FSFS格式的配置参数，包括 memcached、 caches、 sharding 等
4. **uuid** ，仓库的唯一标识 uuid ，一般是一行，例如：edd5f7b9-9120-443b-9ba9-b06e3e89e7dd
1. **current** ，当前最新的版本号
2. **min-unpacked-rev** ，最小的未 packed 的版本的版本号，下一个待pack的版本（可能还不存在）
3. **rep-cache.db** ，hash的数据库，sqlite3格式，记录各个版本的文本内容和他们的hash之间的关系，只用于 representation sharing 启用时。
1. write-lock ，锁文件，在 commit 的最后阶段时加锁，提交完成后解锁。在修改 revprop 时也要加锁。
2. pack-lock ，锁文件，在进行 SVNadmin pack 时要加锁 。（format 7才提供，之前使用 write-lock ，会导致 pack 过程中无法 commit ）
2. txn-current ，当前的事务名称，实际是下一个事务的名称，<base36>-<base-rev>，不断递增
3. txn-current-lock ，锁文件，在某些操作时，先加锁再执行操作

1. **revprops** ，版本的属性数据的目录，存放各个版本的属性数据，根据是否 pack 格式不一样
2. **revs** ，版本的本身数据的目录，存放各个版本的数据，根据是否 pack 格式不一样
1. transactions ，事务数据目录，存放各个事务的相关数据
2. txn-protorevs ，事务的属性数据目录，存放各个事务的属性数据

rep-cache.db 数据库只有一个表：
···
CREATE TABLE rep_cache (   
	hash TEXT NOT NULL PRIMARY KEY,   
	revision INTEGER NOT NULL,   
	offset INTEGER NOT NULL,   
	size INTEGER NOT NULL,   
	expanded_size INTEGER NOT NULL   
);
···

# 基本设计思路

版本记录的是和上一个版本的差异，这样虽然在获取最新版本时效率不高（因为要通过历史版本或还原），
但一般可以通过 cache 的方式来提高效率。这种方式每个版本生成后都不需要更改，存储效率也较高，也比较稳定可靠。



# 未 pack 的属性数据格式


# pack 后的属性数据格式



# 未 pack 的版本数据格式


# pack 后的版本数据格式

