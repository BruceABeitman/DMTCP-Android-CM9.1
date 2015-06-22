#include <stdlib.h>
#include <sys/atomics.h>
extern "C" {
#include <libc/private/bionic_atomic_inline.h>
#include <libc/private/bionic_futex.h>
}
namespace dmtcp {
template <class T, size_t Capacity = 1024>
class Allocator {
  public:
    class Item {
      public:
        Item *_next;
        T _obj;
    };
  public:
    Allocator();
    Item *alloc();
    void free(Item *item);
  private:
    Item _item[Capacity];
    Item *_freeHead;
    volatile int _lock;

    void lock();
    void unlock();
};

template <class T, size_t Capacity>
Allocator<T, Capacity>::Allocator()
  : _freeHead(&_item[0])
  , _lock(0)
{
  size_t i;
  for (i = 0;i<Capacity-1;++i) {
    _item[i]._next = &_item[i+1];
  }
  _item[i]._next = NULL;
}

template <class T, size_t Capacity>
typename Allocator<T, Capacity>::Item *Allocator<T, Capacity>::alloc() {
  lock();
  Item *item = _freeHead;
  if (item != NULL) {
    _freeHead = _freeHead->_next;
  }
  unlock();
  return item;
}

template <class T, size_t Capacity>
void Allocator<T, Capacity>::free(Item *item) {
  int *crash = NULL;
  lock();
  /* Not in our _item ? */
  if (item < &_item[0] && item >= &_item[Capacity]) {
    /* Just crash now !! */
    *crash = 0xdeaddead;
  }
  item->_next = _freeHead;
  _freeHead = item;
  unlock();
}

template <class T, size_t Capacity>
void Allocator<T, Capacity>::lock() {
  /* Grab from _normal_lock in Android/bionic */
  if (__atomic_cmpxchg( 0, 1, &_lock ) != 0) {
    while (__atomic_swap(2, &_lock ) != 0)
    __futex_wait_ex(&_lock, 0, 2, 0);
  }
  ANDROID_MEMBAR_FULL();
}

template <class T, size_t Capacity>
void Allocator<T, Capacity>::unlock() {
  /* Grab from _normal_unlock in Android/bionic */
  if (__atomic_dec(&_lock) != 1) {
    _lock = 0;
    __futex_wake_ex(&_lock, 0, 1);
  }
}

}
