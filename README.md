### getbook
每隔1小时抓取https://book.douban.com 热门书籍，保存到本地数据库。


### 需要第三方库文件
- libcurl
- iniparser
- libevent

### 目录结构

```
-rw-r--r--  1 mac  admin   426  9 30 11:52 README.md
-rw-r--r--  1 mac  admin   647  9 30 11:42 config.ini
-rw-r--r--  1 mac  admin  2505  9 30 11:41 header.h
-rwxr--r--  1 mac  admin   171  9 30 11:41 k.sh
-rw-r--r--  1 mac  admin  1586  9 30 11:41 list.c
-rw-r--r--  1 mac  admin  4903  9 30 11:41 main.c
-rw-r--r--  1 mac  admin   335  9 30 11:41 makefile
-rw-r--r--  1 mac  admin  1686  9 30 11:41 mysql_muliti.c
-rw-r--r--  1 mac  admin  4744  9 30 11:41 pool.c
-rw-r--r--  1 mac  admin  2253  9 30 11:41 tools.c
```


### 如何运行
```shell
git clone https://github.com/cunwang/getbook
cd getbook

#编译
make 

#编译后会生成getbook的可执行文件，直接运行或者利用systemctl进行管理
./getbook 
```
- k.sh 是杀死getbook的进程并重新编译的脚本
