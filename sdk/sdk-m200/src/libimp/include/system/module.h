/*
 * StreamPipeline Module header file.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
 */

#ifndef __MODULE_H__
#define __MODULE_H__

#include <pthread.h>
#include <semaphore.h>

#define NR_MAX_OBSERVERS 5
#define MAX_MODULE_NAME_LEN 16
#define MAX_MODULE_MSG_DEPTH 16

struct Module;
typedef struct Module Module;

struct msg_list {
	void *msg;
	struct msg_list *next;
};

struct module_observer {
	Module *module;
	void **data;
};

struct Module {
	char name[MAX_MODULE_NAME_LEN];

	/* Observer */
	Module *subjectModule;
	struct module_observer observerList[NR_MAX_OBSERVERS];
	int num_of_observer;
	int (*AddObserver)(Module *modsub, Module *modob, void **data);
	int (*RemoveObserver)(Module *modsub, Module *modob);
	int (*NotifyObservers)(Module *modsub);
	int (*Update)(Module *modob, void **data);
	int (*OnDataUpdate)(void *modulex, void *data);

	/* Message queue */
	struct msg_list msg_queue[MAX_MODULE_MSG_DEPTH];
	struct msg_list *msg_head;
	struct msg_list *msg_tail;
	int num_msgs_in_queue;
	sem_t sem;
	sem_t sem_msg;
	pthread_t tid;
	pthread_mutex_t mutex;

	/* For extend */
	unsigned int		private[0];
};

static inline void *module_pri(Module *module)
{
	return (void *)module->private;
}

static inline Module *module_subject(Module *module)
{
	return module->subjectModule;
}

static inline Module *module_observer_by_num(Module *modsub, int modob_num)
{
	if (modob_num < modsub->num_of_observer)
		return modsub->observerList[modob_num].module;

	return NULL;
}

static inline int nr_module_observer(Module *module)
{
	return module->num_of_observer;
}

static inline int module_ob_id(Module *module_sub, Module *module_ob)
{
	int i;

	for (i = 0; i < module_sub->num_of_observer; i++) {
		Module *mod = module_sub->observerList[i].module;
		if (mod == module_ob)
			return i;
	}

	return -1;
}

static inline void SetUpdateCallback(Module *module, int (*update_cb)(void *, void *))
{
	module->OnDataUpdate = update_cb;
}

Module *AllocModule(const char *name, int extras);
void FreeModule(Module *module);
void BindObserverToSubject(Module *module_src, Module *module_dst, void **data);
void UnBindObserverFromSubject(Module *module_src, Module *module_dst);

#define AllocModuleHelper(_MODEX, _NAME, _TYPE)		\
	do {											\
		Module *module;								\
		module = AllocModule(_NAME, sizeof(_TYPE));	\
		_MODEX = module_pri(module);				\
		_MODEX->module = module;					\
	} while (0)

#endif /* __MODULE_H__ */
