#include <stdio.h>
#include <unistd.h>
extern "C" {
ssize_t tls_write(int fd, const void *buf, size_t count);
}

namespace dmtcp {
  class TLSUtils {
    public:
      typedef void *(*pthread_getspecific_t)(pthread_key_t);
      typedef int (*pthread_setspecific_t)(pthread_key_t, const void *);
      typedef int (*pthread_key_create_t)(pthread_key_t *,
                                          void (*dtor)(void *));
      typedef int (*pthread_key_delete_t)(pthread_key_t);

      static void *pthread_getspecific(pthread_key_t key);
      static int pthread_setspecific(pthread_key_t key, const void *value);
      static int pthread_key_create(pthread_key_t *key,
                                    void (*dtor)(void *));
      static int pthread_key_delete(pthread_key_t key);
      static void debug(const char *str);
    private:
      static TLSUtils &instance();
      TLSUtils();

      pthread_getspecific_t _pthread_getspecific;
      pthread_setspecific_t _pthread_setspecific;
      pthread_key_create_t _pthread_key_create;
      pthread_key_delete_t _pthread_key_delete;
  };
}

namespace dmtcp {
  template<class T>
  TLS<T>::TLS()
    : _tlsKey(0)
    , _default()
  {
    TLSUtils::pthread_key_create(&_tlsKey, deallocate);
  }

  template<class T>
  TLS<T>::TLS(const T &val)
    : _tlsKey(0)
    , _default(val)
  {
    TLSUtils::pthread_key_create(&_tlsKey, deallocate);
  }

  template<class T>
  TLS<T>::~TLS() {
    T *data = get();
    TLSUtils::pthread_key_delete(_tlsKey);
  }

  template<class T>
  T &TLS<T>::operator=(const T &val) {
    T *data = get();
    *data = val;
    return *data;
  }

  template<class T>
  TLS<T>::operator T&() {
    T *data = get();
    return *data;
  }

  template<class T>
  T *TLS<T>::get() const {
    typename TLSAllocator::Item *data =
      (typename TLSAllocator::Item *)TLSUtils::pthread_getspecific(_tlsKey);
    if (data == NULL) {
      data = allocator().alloc();
      data->_obj = _default;
      TLSUtils::pthread_setspecific(_tlsKey, (void*)data);
    }
    return &data->_obj;
  }

  template<class T>
  const T &TLS<T>::operator=(const T &val) const {
    const T *data = get();
    *data = val;
    return *data;
  }

  template<class T>
  TLS<T>::operator const T&() const {
    const T *data = get();
    return *data;
  }

  template<class T>
  typename TLS<T>::TLSAllocator &TLS<T>::allocator() {
    static TLSAllocator _allocator;
    return _allocator;
  }
  template<class T>
  void TLS<T>::deallocate(void *data) {
    typename TLSAllocator::Item *freeItem
      = (typename TLSAllocator::Item *)data;
    TLS<T>::allocator().free(freeItem);
  }
}
