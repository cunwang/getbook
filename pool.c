/**
 * file pool.c
 * @abstract 线索池功能实现
 * @author <github.com/cunwang>
 * @date 2017/05/28
 * @since 1.0
 */

#include "header.h"

void th_running(TPOOL * pool);
int tp_add_jobs(void * (*func)(Books *args), Books * args, TPOOL * pool);
bool th_exists(pthread_t tid);
bool th_destory();
double get_t_perct();
int need_thread_nums(TPOOL * pool);
void th_exec(char * type,  int nums, TPOOL * pool);
void th_dynamic_extended(TPOOL * pool);
void th_dynamic_reduce(TPOOL * pool);

extern dictionary  * ini_config;
extern CQ * cq_init(CQ * cq);
extern void cq_push(CQ *cq, CQ_ITEM *item);
extern CQ_ITEM *cq_pop(CQ *cq);

/**
 * 获取线程增长因子
 */ 
double get_t_perct ()
{
	double pt;
	pt	= iniparser_getdouble(ini_config, "thread:thread_add_prec", -0.01);
	return pt;
}

/**
 * 计算要增长的量
 */ 
int need_thread_nums(TPOOL * pool)
{
	return pool->cq_list->len * get_t_perct();
}

/**
 * 线程池初始化
 */
TPOOL * tp_init(struct settings * config)
{
	if (config->thread_max_size <= config->thread_min_size) {
		config->thread_max_size = config->thread_min_size;
	}
	
	TPOOL * pool;
	CQ * cqlist;

	cqlist	= malloc(sizeof(struct conn_queue)); 
	pool 	= malloc (sizeof (TPOOL));
	pthread_mutex_init (&pool->queue_lock, NULL);
	pthread_cond_init (&pool->queue_ready, NULL);
	pool->cq_list	= cq_init(cqlist);
	pool->cq_list->max_size	= config->queue_max_list;
	pool->tids		= NULL;
	pool->td_cond_size= config->thread_add_cond;
	pool->is_init	= 0;
	pool->is_halt	= 0;
	pool->curr_thread_size= 0;
	pool->core_size	= config->thread_min_size;
	pool->max_thread_size	= config->thread_max_size;

	return pool;	
}

/**
 * @abstract 线程管理
 *  - 初次进来创建最少线程
 *  - 线索量：线程量 < 阀值，动态扩展 
 *  - 线索量：线程量 > 阀值，动态减少 
 */
void th_management(TPOOL * pool)
{
	int core_t,t;
	double pt;
	pt		= get_t_perct();
	t		= iniparser_getint(ini_config, "thread:thread_mointer_interval", 1);
	core_t	= iniparser_getint(ini_config, "thread:thread_min_size", -1);
	pt		= 0.01;

	if (pool->cq_list->len == 0 && pool->curr_thread_size == 0 && pool->is_init == 0) {
		th_exec("inc", core_t, pool);
		pool->is_init	= 1;
	}

	while (1) {	
		if (pool->cq_list->len > pool->td_cond_size) {
			if ((pool->curr_thread_size / pool->cq_list->len) < pt) {
				th_dynamic_extended(pool);
			}
		} else {
			if (pool->curr_thread_size > 0 
				&& pool->cq_list->len	> 0 
				&& (double)pool->curr_thread_size / pool->cq_list->len > pt) {
				th_dynamic_reduce(pool);
			}
		}
		
		sleep(t);
	}
}

void th_exec(char * types,  int nums, TPOOL * pool)
{
	if (nums <= 0) return;

	int i;
	T_IDS * tmp;
	if (strcmp(types, "inc") == 0) {
		for (i = 0; i< nums; i++) {
			if (pool->curr_thread_size >= pool->max_thread_size) break;

			tmp	= malloc(sizeof(T_IDS));
			tmp->next	= NULL;

			if (NULL == pool->tids) {
				pool->tids	= tmp;
			} else {
				tmp->next	= pool->tids;		
				pool->tids	= tmp;
			}
			pthread_create(&tmp->id, NULL, (void *)th_running, pool);
			pool->curr_thread_size ++;
		}		
	} else if (strcmp(types, "rem") == 0) {
		i=1;
		tmp	= pool->tids;
		while (NULL != tmp->next) {
			if (i == nums || pool->curr_thread_size <= pool->core_size) break;

			tmp	= tmp->next;
			kill(tmp->id, SIGKILL);

			pool->curr_thread_size --;
			i++;
		}
	} else {}
}

void th_dynamic_extended(TPOOL * pool)
{
	int need_nums, inc_nums;
	need_nums	= need_thread_nums(pool);
	inc_nums	= (need_nums >= pool->max_thread_size) ? (pool->max_thread_size - pool->curr_thread_size) : (need_nums - pool->curr_thread_size);

	th_exec("inc", inc_nums, pool);
}


void th_dynamic_reduce(TPOOL * pool)
{
	int need_nums, reduce_nums;
	need_nums	= need_thread_nums(pool);
	if (need_nums <= pool->core_size) {
		need_nums	= pool->core_size;
	}
	reduce_nums	= pool->curr_thread_size - need_nums;

	th_exec("rem", reduce_nums, pool);
}

void th_running(TPOOL * pool)
{
	while (1) {
		pthread_mutex_lock(&pool->queue_lock);
		while(pool->cq_list->len == 0) {
			pthread_cond_wait(&pool->queue_ready, &pool->queue_lock);
		}
		pthread_mutex_unlock(&pool->queue_lock);

		if (pool->cq_list->len > 0 && pool->cq_list->head != NULL) {
			CQ_ITEM * worker;
			worker	= cq_pop(pool->cq_list);
			if (NULL != worker) {
				(*(worker->func)) (worker->arg);	
			}
			free(worker);
			worker	= NULL;
		}
	}
}


int tp_add_jobs( void * (*func)(Books *args), Books * args, TPOOL * pool)
{
	CQ_ITEM * worker;
	worker			= (CQ_ITEM *) malloc(sizeof(CQ_ITEM));
	worker->func	= func;
	worker->arg		= malloc(sizeof(Books));
	worker->arg		= args;

	pthread_mutex_lock(&pool->queue_lock);
	cq_push(pool->cq_list, worker);
	pthread_mutex_unlock(&pool->queue_lock);
	pthread_cond_signal(&pool->queue_ready);	

	return 0;
}
