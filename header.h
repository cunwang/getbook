/**
 * thread pool header
 * @date 2017/09
 * @author <github.com/cunwang>
 * @since 3.0
 */

#pragma pack(1)
#ifndef __DEF_MYHEADER__
#define __DEF_MYHEADER__

	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <unistd.h>
	#include <pthread.h>
	#include <assert.h>
	#include <errno.h>
	#include <stdbool.h>
	#include <regex.h>
	#include <time.h>
	#include <syslog.h>
	#include <math.h>
	#include <sys/file.h>
	#include <fcntl.h>
	#include <signal.h>
	#include <sys/time.h>
	#include <curl/curl.h> //导入libcurl
	#include <event.h> //导入libevent event
	#include "iniparser.h" //导入iniparser

	#define EXIT_OK() { exit(EXIT_SUCCESS); }
	#define EXIT_TRAP() { exit(EXIT_FAILURE);  } 

	#define LOCKFILE_PATH "/var/run/getbook.pid"
	#define LOCKMOD (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)

	/**
	* 队列需状态 
	*/
	enum cq_status {
		cq_destory,
		cq_no_destory,

		cq_clears,
		cq_no_clears
	};

	typedef struct mysql_config Mconfig;
	struct mysql_config {
		const char * host;
		const char * user;
		const char * db;
		const char * password;
		int port;
	};

	/**
 	 * douban book
 	 */ 
	typedef struct douban_book Books;
	struct douban_book {
		int id;
		char * name;
		char * abstract;
		char * url;
		char * comments_url;
		pthread_mutex_t lock;
	};
	
	/**
	 * worker struct
	 */ 
	typedef struct conn_queue_item CQ_ITEM;
	struct conn_queue_item {
		Books *arg;
		void *(*func) (Books *arg); 
		CQ_ITEM *prev;
		CQ_ITEM *next;
	};

	/**
	 * 队列结构 
	 */
	typedef struct conn_queue CQ;
	struct conn_queue {
		CQ_ITEM *head;
		CQ_ITEM *tail;

		int index;
		int max_size;
		int len;

		enum cq_status is_destory;
		enum cq_status is_clear;

		pthread_mutex_t lock;
		pthread_cond_t cq_clears;
		pthread_cond_t cq_destory;
	};

	enum thread_status {
		STATE_WAITTING, //等待
		STATE_TASK_PROCESS, //处理中
		STATE_TASK_FINISHED //处理完
	};

	typedef struct thread_id_list T_IDS;
	struct thread_id_list {
		pthread_t id;
		enum thread_status state;
		T_IDS * next;	
	};

	/**
	 * 线程池结构
	 */ 
	typedef struct thread_pool TPOOL;
	struct thread_pool {

		pthread_mutex_t queue_lock;
		pthread_cond_t queue_ready;
		T_IDS * tids;
		CQ * cq_list;

		int is_halt;
		int is_init; 
		int core_size;
		int curr_thread_size;
		int max_thread_size;
		int td_cond_size; //来自于setting
	};
	
	struct settings {
		int thread_min_size;
		int thread_max_size;
		int queue_max_list;
		int thread_add_cond; //线程扩展的条件（列表的数量）			
	};

#endif
