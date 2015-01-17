#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include <system/module.h>

static int add_observer(Module *modsub, Module *modob, void **data)
{
	int i;

	for (i = 0; i < NR_MAX_OBSERVERS; i++) {
		if (modsub->observerList[i].module == NULL) {
			modsub->observerList[i].module = modob;
			modsub->observerList[i].data = data;
			modsub->num_of_observer++;
			modob->subjectModule = modsub;
			return 0;
		}
	}

	printf("%s %s error: Can't add more observers\n", modsub->name, __func__);

	return -1;
}

static int remove_observer(Module *modsub, Module *modob)
{
	int i;

	for (i = 0; i < NR_MAX_OBSERVERS; i++) {
		if (modsub->observerList[i].module == modob) {
			modsub->observerList[i].module = NULL;
			modsub->observerList[i].data = NULL;
			modsub->num_of_observer--;
			modob->subjectModule = NULL;
			return 0;
		}
	}

	printf("%s %s error: Can't find observer: %s\n", modsub->name, __func__, modob->name);

	return -1;
}

static int notify_observers(Module *modsub)
{
	int i;
	for (i = 0; i < modsub->num_of_observer; i++) {
		int ret;
		Module *modob = modsub->observerList[i].module;

		ret = modob->Update(modob, modsub->observerList[i].data);
		if (ret < 0)
			printf("%s update failed\n", modob->name);
	}

	return 0;
}

static int write_msg(Module *module, void *data)
{
#ifdef MESSAGE_BLOCKING
	if (sem_trywait(&module->sem_msg) < 0) {
		printf("%s: msg queue is full, blocking...", module->name);
		sem_wait(&module->sem_msg);
		printf("OK. Continue\n");
	}
#endif
	pthread_mutex_lock(&module->mutex);

	if (module->num_msgs_in_queue == MAX_MODULE_MSG_DEPTH) {
		printf("write msg error: Full\n");
		pthread_mutex_unlock(&module->mutex);

		return -1;
	}
	module->msg_head->msg = data;

	module->msg_head = module->msg_head->next;
	module->num_msgs_in_queue++;

	pthread_mutex_unlock(&module->mutex);

	return 0;
}

static void *read_msg(Module *module)
{
	void *data;

	pthread_mutex_lock(&module->mutex);

	if (module->num_msgs_in_queue == 0) {
		printf("read msg error: Empty\n");
		pthread_mutex_unlock(&module->mutex);
		return NULL;
	}
	data = module->msg_tail->msg;

	module->msg_tail = module->msg_tail->next;
	module->num_msgs_in_queue--;

	pthread_mutex_unlock(&module->mutex);
#ifdef MESSAGE_BLOCKING
	sem_post(&module->sem_msg);
#endif
	return data;
}

static int update(Module *modob, void **data)
{
	int ret;

	ret = write_msg(modob, *data);
	if (ret < 0)
		return ret;

	sem_post(&modob->sem);

	return 0;
}

static void* module_thread(void *m)
{
	Module *module = (Module *)m;

	while (1) {
		sem_wait(&module->sem);

		void *p = read_msg(module);

		if (module->OnDataUpdate) {
			module->OnDataUpdate(module_pri(module), p);
			module->NotifyObservers(module);
		}
	}

	return 0;
}

Module *AllocModule(const char *name, int extras)
{
	Module *module;
	int i, ret;

	module = malloc(sizeof(struct Module) + extras);
	if (module == NULL) {
		printf("malloc module error\n");
		return NULL;
	}
	memset(module, 0, sizeof(struct Module) + extras);

	i = strlen(name);
	if (i > MAX_MODULE_NAME_LEN) {
		printf("The length of name %d is longer that %d\n", i, MAX_MODULE_NAME_LEN);
		goto free;
	}
	strcpy(module->name, name);

	/* Observer initialize */
	module->num_of_observer = 0;
	for (i = 0; i < NR_MAX_OBSERVERS; i++) {
		module->observerList[i].module = NULL;
		module->observerList[i].data = NULL;
	}
	module->subjectModule = NULL;
	sem_init(&module->sem, 0, 0);
	sem_init(&module->sem_msg, 0, MAX_MODULE_MSG_DEPTH);

	ret = pthread_mutex_init(&module->mutex, NULL);
	if (ret != 0) {
		printf("pthread_mutex_init() error\n");
	}

	ret = pthread_create(&module->tid, NULL, module_thread, module);
	if (ret) {
		printf("module_thread create error\n");
		goto free;
	}

	module->AddObserver = add_observer;
	module->RemoveObserver = remove_observer;
	module->NotifyObservers = notify_observers;
	module->Update = update;

	/* Message queue initialize */
	for (i = 0; i < MAX_MODULE_MSG_DEPTH; i++) {
		struct msg_list *msg_tmp = &module->msg_queue[i];
		msg_tmp->msg = NULL;
		msg_tmp->next = &module->msg_queue[i + 1];
		if (i == (MAX_MODULE_MSG_DEPTH - 1))
			msg_tmp->next = &module->msg_queue[0];
	}
	module->msg_head = &module->msg_queue[0];
	module->msg_tail = &module->msg_queue[0];
	module->num_msgs_in_queue = 0;

	return module;
free:
	free(module);

	return NULL;
}

void FreeModule(Module *module)
{

}

void BindObserverToSubject(Module *module_src, Module *module_dst, void **data)
{
	if (module_src == NULL) {
		printf("module_src is NULL!\n");
		return;
	}

	if (module_dst == NULL) {
		printf("module_dst is NULL!\n");
		return;
	}

	module_src->AddObserver(module_src, module_dst, data);
}

void UnBindObserverFromSubject(Module *module_src, Module *module_dst)
{
	if (module_src == NULL) {
		printf("module_src is NULL!\n");
		return;
	}

	if (module_dst == NULL) {
		printf("module_dst is NULL!\n");
		return;
	}

	module_src->RemoveObserver(module_src, module_dst);
}
