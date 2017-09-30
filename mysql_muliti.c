#include "header.h"
#include <my_global.h>
#include <mysql.h>

extern dictionary  * ini_config;

Mconfig * get_db_config ()
{
	Mconfig * mc;
	mc	= malloc (sizeof(Mconfig));
	mc->host	= iniparser_getstring(ini_config, "mysql:host", NULL);
	mc->user	= iniparser_getstring(ini_config, "mysql:user", NULL);
	mc->password= iniparser_getstring(ini_config, "mysql:password", NULL);
	mc->db		= iniparser_getstring(ini_config, "mysql:db", NULL);
	mc->port	= iniparser_getint(ini_config, "mysql:port", -1);

	return mc;
}

MYSQL * m_conn()
{
	MYSQL *con;
	Mconfig *dbConfig;
	char errorlog[100];
	con = mysql_init(NULL);
	if (con == NULL) return NULL;
	
	dbConfig	= get_db_config();	
	//printf("%s\t%s\t%s\t%s\t%d\n", dbConfig->host, dbConfig->user, dbConfig->password, dbConfig->db, dbConfig->port);
	if (mysql_real_connect(con, dbConfig->host, dbConfig->user, dbConfig->password, dbConfig->db, dbConfig->port, NULL, 0) == NULL) {
		mysql_close(con);
		con	= NULL;
		free(dbConfig);
		dbConfig	= NULL;
		mysql_thread_end();
		return NULL;
	}  

	mysql_query(con, "set names utf8");
	return con;
}

m_query(char * sql, MYSQL * con)
{
	if (mysql_query(con, sql)) {
		fprintf(stderr, "%s\n", mysql_error(con));
	}
}

int m_get_num(char * sql, MYSQL * con)
{
	long nums;
	MYSQL_RES *res_ptr;

	if (! mysql_query(con, sql)) {
		res_ptr	= mysql_store_result(con);
		nums	= mysql_num_rows(res_ptr);
		mysql_free_result(res_ptr);
	} else {
		nums	= 0;
	}
	
	return nums;
}

MYSQL_RES * m_get_res(char * sql, MYSQL * con)
{
	if (mysql_query(con, sql)) {
		fprintf(stderr, "%s\n", mysql_error(con));
	}

	return	mysql_store_result(con);
}

m_close(MYSQL * con)
{
	mysql_close(con);
	mysql_thread_end();
}
