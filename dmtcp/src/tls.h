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

#ifndef TLS_H
#define TLS_H

#include <pthread.h>
#include "tls_allocator.hpp"

namespace dmtcp
{
  template <class T>
  class TLS
  {
    public:
      TLS();
      TLS(const T &val);
      ~TLS();
      operator T&();
      T &operator=(const T &val);
      operator const T&() const;
      const T &operator=(const T &val) const;
    private:
      typedef Allocator<T, 2048> TLSAllocator;
      T *get() const;
      pthread_key_t _tlsKey;
      T _default;
      static TLSAllocator &allocator();
      static void deallocate(void *data);
  };
}

#include "tls_impl.hpp"

#endif
