#include "header.h"

struct string {
	char *ptr;
	size_t len;
};

void write2log(char *s)
{
	openlog("getbook",LOG_PID,LOG_DAEMON);
	syslog(LOG_INFO,"%s\n",s);
	closelog();
}

char *sub_string(char *str, int start, int end)
{
	static char * st = NULL;
	int i = start, j = 0;
	st ? free(st) : 0;
	st = (char *)malloc(sizeof(char) * (end - start + 1));
	while(i < end){
		st[j++] = str[i++];
	}
	st[j] = '\0';
	return st;
}

void init_string(struct string *s) 
{
	s->len = 0;
	s->ptr = malloc(s->len+1);
	if (s->ptr == NULL) {
		EXIT_TRAP();
	}
	s->ptr[0] = '\0';
}

size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s)
{
	size_t new_len = s->len + size*nmemb;
	s->ptr = realloc(s->ptr, new_len+1);
	if (s->ptr == NULL) {
		EXIT_TRAP();
	}
	memcpy(s->ptr+s->len, ptr, size*nmemb);
	s->ptr[new_len] = '\0';
	s->len = new_len;

	return size*nmemb;
}


/**
 * @abstract 返回HTML
 * @usage:
 *	> char *ss = get_html(url);
 */
char * get_html(char * url)
{
	CURL *curl;
	struct string s;
	init_string(&s);
	curl = curl_easy_init();
	if (curl) {
		//printf("pre curl url=%s\n", tmp);
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
		curl_easy_perform(curl);
		//printf("pre curl url=%s\n", tmp);
	}
	curl_easy_cleanup(curl);
	//printf("in %s, url=%s ,conts = %s\n", __func__, tmp, "hello");
	return (char*)s.ptr;
}

int lockfile(int fd)
{
	struct flock fl;

	fl.l_type	= F_WRLCK;
	fl.l_start	= 0;
	fl.l_whence	= SEEK_SET;
	fl.l_len	= 0;

	return (fcntl(fd, F_SETLK, &fl));
}

int is_run(const char *filename)
{
	int fd;
	char buf[16];
	fd	= open(filename, O_WRONLY| O_CREAT, LOCKMOD);
	if (fd < 0) {
		exit(1);
	}

	if (lockfile(fd) == -1) {
		if (errno == EACCES || errno == EAGAIN) {
			close(fd);
			return 1;
		}
		exit(1);
	}

	ftruncate(fd, 0);
	sprintf(buf, "%ld", (long) getpid());
	write(fd, buf, strlen(buf) + 1);

	return 0;
}


void init_deamon()
{
	int i,MAXFILE_S;
	pid_t pid,sid;	
	umask(0);
	pid=fork();
	if (pid < 0) {
		EXIT_TRAP();
	} else if(pid > 0) {
		EXIT_OK();	
	}
	MAXFILE_S = getdtablesize();
	for(i=0;i<MAXFILE_S;i++) {
		close(i);	
	}
}
