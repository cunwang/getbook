/**
 * file list.c
 * @abstract 队列相关操作函数, 参考memcache源码thread.c实现
 * @date 2017/05
 * @author <github.com/cunwang>
 * @since 1.0
 */ 

#include "header.h"

void cq_print(CQ *cq);
CQ * cq_init(CQ *cq)
{
    pthread_mutex_init(&cq->lock, NULL);
	pthread_cond_init(&cq->cq_clears, NULL);
	pthread_cond_init(&cq->cq_destory, NULL);
	
	cq->is_destory=0;
	cq->is_clear = 0;
    cq->head    = NULL;
    cq->tail	= NULL;
    cq->max_size= 65535;
    cq->len		= 0;

	return cq;
}

void cq_push(CQ *cq, CQ_ITEM *item)
{
	if (cq->len > cq->max_size) return;
	item->next	= NULL;
    pthread_mutex_lock(&cq->lock);
    if (NULL == cq->tail) {
        cq->head    = item;
    } else {
        cq->tail->next  = item;
    }

    cq->len ++;
    cq->tail    = item;
    pthread_mutex_unlock(&cq->lock);
}

CQ_ITEM *cq_pop(CQ *cq)
{
	if (cq->len == 0) return;
    CQ_ITEM *item;

    pthread_mutex_lock(&cq->lock);
	item	= cq->head;
	if (NULL != item) {
		cq->head	= item->next;
		if (NULL == cq->head) {
			cq->tail	= NULL;
		}
	} 

	cq->len --;
	if (cq->len < 0) cq->len = 0;
    pthread_mutex_unlock(&cq->lock);

    return item;
}

void cq_print(CQ *cq)
{
    if (NULL == cq->head) {
		EXIT_TRAP();	
    }

    CQ_ITEM *item;
    item    = cq->head;

    while (NULL != item) {
		item    = item->next;
    }
    printf("\n");
}

void cq_clear(CQ *cq)
{
	if (NULL == cq->head) {
		EXIT_TRAP();	
	}	

	CQ_ITEM *item;	
	item	= cq->head->next;
    while (NULL != item) {
		cq->head	= item;
		item		= item->next;	
	}

	free(item);
	item	= NULL;

	cq->tail = cq->head	= NULL;
}
