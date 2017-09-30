/**
 * [爬虫小示例]
 * 
 * @abstract 每隔1小时抓取book.douban.com 推荐的新书列表入库
 * @author <github.com/cunwang>
 * @date 2017/09
 * @version 3.0
 * @usage:
 * > make && ./getbook
 */

#include "header.h" 
#include "pool.c"
#include "list.c"
#include "mysql_muliti.c"
#include "tools.c"

TPOOL * pool;
pthread_t mt;
dictionary  * ini_config;

extern void th_management(TPOOL * pool);
extern m_query(char * sql, MYSQL * con);  
extern m_close(MYSQL * con);
extern int m_get_num(char * sql, MYSQL * con);
extern char *sub_string(char *str, int start, int end);
extern void init_string(struct string *s);
extern size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s);
extern char * getHtml(char * url);
extern int is_run(const char *filename);
extern int lockfile(int fd);


/**
 * 最终执行的任务
 */
void * douban_process(Books * book)
{
	if (NULL == book) return NULL;

	MYSQL *conn;
	char sql[350], logstr[150];
	time_t t;
	t=time(0);

	pthread_mutex_lock(&book->lock);
	conn	= m_conn();

	if (NULL != book) {
		sprintf(sql, "select id from books where dou_book_id=%d", book->id);
		int nums= m_get_num(sql, conn);
		sprintf(logstr, "%s\t%d", sql, nums);
		write2log(logstr);
		if (nums < 1) {
			sprintf(logstr, "find new book: %s\n", book->name);
			write2log(logstr);
			sprintf(sql, "insert into books(name,dou_book_id,detail_url,comments_url,abstract,created_at, created_by, updated_at) Values('%s',%d,'%s','%s','%s',%d,%d,%d)", book->name, book->id, book->url, book->comments_url, book->abstract, t, 1, t);
			write2log(sql);
			m_query(sql, conn);
		}
	}

	pthread_mutex_unlock(&book->lock);
	m_close(conn);
   	return NULL;
}


/**
 * 正则
 */
int getAllHref(char * html)
{
	Books * book;
	char * tmp;
	char *pattern = "<a\\shref=([\'\"])((https?://book.douban.com/subject/([0-9]{1,11})/)[^\'\"]*?)\\1\\s+title=\\1([^\'\"]*?)\\1"; 
	char *file = html;
	char *st, *url;	

	regmatch_t pm[10];
	regex_t preg;
	if (regcomp(&preg, pattern, REG_EXTENDED | REG_NEWLINE) != 0) {
		return -1;
	}

	st = file;
	while (st && regexec(&preg, st, 6, pm, REG_NOTEOL) != REG_NOMATCH) {
		book				= malloc(sizeof(Books));
		book->id			= atoi(sub_string(st, pm[4].rm_so, pm[4].rm_eo));
		book->abstract		= NULL;
		book->url			= (char *) malloc(70);
		book->name			= (char *) malloc(70);
		book->comments_url	= (char *) malloc(70);

		tmp	= (char *) malloc(70);
		strcpy(tmp, sub_string(st, pm[3].rm_so, pm[3].rm_eo));
		strcat(tmp, "comments/");
		strcpy(book->name, sub_string(st, pm[5].rm_so, pm[5].rm_eo));
		strcpy(book->url, sub_string(st, pm[2].rm_so, pm[2].rm_eo));
		strcpy(book->comments_url, tmp);

		free(tmp);
		tmp= NULL;
		st = &st[pm[4].rm_eo];

		/**
 		 * push book jobs to queue
 		 */ 
		tp_add_jobs(douban_process, book, pool);
	}

	return 0;
}



/**
 * 配置信息
 */
struct settings * init_setting()
{
	struct settings * tmp;
	tmp	= malloc (sizeof(struct settings));
	tmp->thread_min_size	= iniparser_getint(ini_config, "thread:thread_min_size", -1);
	tmp->thread_max_size	= iniparser_getint(ini_config, "thread:thread_max_size", -1);
	tmp->queue_max_list		= iniparser_getint(ini_config, "thread:queue_max_list", -1);
	tmp->thread_add_cond	= iniparser_getint(ini_config, "thread:thread_add_cond", -1);
	//printf("%d\t%d\t%d\t%d\n", tmp->thread_min_size, tmp->thread_max_size, tmp->queue_max_list, tmp->thread_add_cond);
	return tmp;
}

char * load_base(dictionary * config)
{
	char *url;
	const char * bu;
	char bu1[100];
	bu = iniparser_getstring(ini_config, "common:url", NULL);
	sprintf(bu1, "%s", bu);
	url	= bu1;

	return url;
}

/**
 * event callback function
 */
void onTime(int sock, short event, void *arg)
{
	const char * tmp_str;
	char tmp[100];
	char *main_url;
	int interval;
	tmp_str	= iniparser_getstring(ini_config, "common:url", NULL);
	interval= iniparser_getint(ini_config, "common:interval", -1);
	strcpy(tmp, tmp_str);
	main_url	= tmp;

	write2log("di da");
	getAllHref(getHtml(main_url));

	/**
 	 * trigger event again
 	 */
	struct timeval tv;
    tv.tv_sec = interval;
    tv.tv_usec = 0;
	event_add((struct event*)arg, &tv);
}

void init()
{

	/**
 	 * load config.ini
 	 */ 
	int interval;
	ini_config	= iniparser_load("config.ini");
	if (ini_config == NULL)  EXIT_TRAP();
	interval= iniparser_getint(ini_config, "common:interval", -1);

	/**
	 * 启动线程池 
	 */
	pool	= tp_init(init_setting());	
	pthread_create(&mt, NULL, (void *) th_management, pool);
	mysql_library_init(0, 0, NULL);

	/**
 	 * libevent 循环事件 
 	 */
	event_init();
	struct event ev_time;
	evtimer_set(&ev_time, onTime, &ev_time);
	struct timeval tv;
    tv.tv_sec = interval;
    tv.tv_usec = 0;
    event_add(&ev_time, &tv);
    event_dispatch();

	mysql_library_end();
}

int main(void)
{
	init_deamon();
	if (is_run(LOCKFILE_PATH)) {
		return 0;
	} else {
		write2log("int main will init!\n");
		init();
	}

  	return 0;
}
