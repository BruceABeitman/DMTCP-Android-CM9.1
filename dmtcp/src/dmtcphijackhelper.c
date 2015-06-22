#include <dlfcn.h>

void *__helper_dlopen(){
  return (void*)&dlopen;
}

void *__helper_dlerror(){
  return (void*)&dlerror;
}

void *__helper_dlclose(){
  return (void*)&dlclose;
}

void *__helper_dlsym(){
  return (void*)&dlsym;
}
