/****************************************************************************
 *   Copyright (C) 2006-2012 by Kito Cheng                                  *
 *   kito@0xlab.org                                                         *
 *                                                                          *
 *   This file is part of the dmtcp/src module of DMTCP (DMTCP:dmtcp/src).  *
 *                                                                          *
 *  DMTCP:dmtcp/src is free software: you can redistribute it and/or        *
 *  modify it under the terms of the GNU Lesser General Public License as   *
 *  published by the Free Software Foundation, either version 3 of the      *
 *  License, or (at your option) any later version.                         *
 *                                                                          *
 *  DMTCP:dmtcp/src is distributed in the hope that it will be useful,      *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *  GNU Lesser General Public License for more details.                     *
 *                                                                          *
 *  You should have received a copy of the GNU Lesser General Public        *
 *  License along with DMTCP:dmtcp/src.  If not, see                        *
 *  <http://www.gnu.org/licenses/>.                                         *
 ****************************************************************************/

#include <tls.h>
#include <constants.h>
#include <string.h>
#include <unistd.h>


extern "C" {
  void *__helper_dlopen();
  void *__helper_dlerror();
  void *__helper_dlsym();
  void *__helper_dlclose();
}

namespace dmtcp {

TLSUtils &TLSUtils::instance() {
  static TLSUtils _instance;
  return _instance;
}

TLSUtils::TLSUtils() {
  typedef void *(*dlsym_t)(void *, const char *);
  typedef void *(*dlopen_t)(const char *, int); 
  dlsym_t _dlsym = (dlsym_t)__helper_dlsym();
  dlopen_t _dlopen = (dlopen_t)__helper_dlopen();
  void *handle = _dlopen(LIBPTHREAD_FILENAME, 0);
  
  void *func = _dlsym(handle, "pthread_getspecific");
  _pthread_getspecific
    = (pthread_getspecific_t)func;
  func = _dlsym(handle, "pthread_setspecific");
  _pthread_setspecific
    = (pthread_setspecific_t)func;
  func = _dlsym(handle, "pthread_key_create");
  _pthread_key_create
    = (pthread_key_create_t)func;
  func = _dlsym(handle, "pthread_key_delete");
  _pthread_key_delete
    = (pthread_key_delete_t)func;
}

void *TLSUtils::pthread_getspecific(pthread_key_t key) {
  return instance()._pthread_getspecific(key);
}

int TLSUtils::pthread_setspecific(pthread_key_t key, const void *value) {
  return instance()._pthread_setspecific(key, value);
}

int TLSUtils::pthread_key_create(pthread_key_t *key,
                                 void (*dtor)(void *)) {
  return instance()._pthread_key_create(key, dtor);
}

int TLSUtils::pthread_key_delete(pthread_key_t key) {
  return instance()._pthread_key_delete(key);
}

void TLSUtils::debug(const char *str) {
}

}
