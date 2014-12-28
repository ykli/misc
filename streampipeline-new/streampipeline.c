#include <stdint.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#define NR_MAX_OBSERVERS 5
#define MAX_MODULE_NAME_LEN 8

typedef struct Module {
  char name[MAX_MODULE_NAME_LEN];
  Module *subjectModule;
  Module *observerList[NR_MAX_OBSERVERS];
  int num_of_observer;

  void (*AddObserver)(Module *modsub, Module *modob);
  void (*RemoveObserver)(Module *modsub, Module *modob);
  void (*NotifyObservers)(Module *modsub);
  void (*Update)(Module *modob);
} Module;

static int add_observer(Module *modsub, Module *modob)
{
  int i;

  for (i = 0; i < NR_MAX_OBSERVERS; i++) {
    if (modsub->observerList[i] == NULL) {
      modsub->observerList[i] = modob;
      modob->subjectModule = modsub;
      num_of_observer++;
      return 0;
    }
  }

  return -1;
}

static int remove_observer(Module *modsub, Module *modob)
{
  int i;

  for (i = 0; i < NR_MAX_OBSERVERS; i++) {
    if (modsub->observerList[i] == modob) {
      modsub->observerList[i] = NULL;
      modob->subjectModule = NULL;
      num_of_observer--;
      return 0;
    }
  }

  return -1;
}

static int notify_observers(Module *modsub)
{
  int i;
  for (i = 0; i < modsub->num_of_observer; i++) {
    Module *modob = modsub->observerList[i];
    modob->Update(modob);
  }
}

static int update(Module *modob)
{
  printf("%s update\n", modob->name);
}

typedef struct IPMModule {
  Module *module;
  int features;
} IPMModule;

Module *AllocModule(const char *name, int extras)
{
  Module *module = malloc(sizeof(struct Module) + extras);
  int i;

  i = strlen(name);
  if (i > MAX_MODULE_NAME_LEN) {
    printf("The length of name %d is longer that %d\n", i, MAX_MODULE_NAME_LEN);
    return NULL;
  }
  strcpy(module->name, name);

  num_of_observer = 0;
  for (i = 0; i < NR_MAX_OBSERVERS; i++) {
    observerList[i] = NULL;
  }

  module->AddObserver = add_observer;
  module->RemoveObserver = remove_observer;
  module->NotifyObservers = notify_observers;
  module->Update = update;

  return module;
}

void BindObserverToSubject(Module *module_src, Module *module_dst)
{
  if (module_src == NULL) {
    printf("module_src is NULL!\n");
    return;
  }

  if (module_dst == NULL) {
    printf("module_dst is NULL!\n");
    return;
  }

  module_src->AddObserver(module_dst);
}

void UnbindObserverFromSubject(Module *module_src, Module *module_dst)
{
  if (module_src == NULL) {
    printf("module_src is NULL!\n");
    return;
  }

  if (module_dst == NULL) {
    printf("module_dst is NULL!\n");
    return;
  }

  module_src->RemoveObserver(module_dst);
}

int main(int argc, char *argv[])
{
  IPMModule *ipm = AllocModule("IPM", sizeof(struct IPMModule));
  VIModule *vi = AllocModule("VI", sizeof(struct VIModule));

  BindObserverToSubject(vi, ipm);

  return 0;
}
