#include "tecs.h"

namespace ls::lecs
{
  void column::mset(void* from, void* to, const size_t fidx, const size_t tidx, const size_t length)
  {
    memmove(to + tidx, from + fidx, length);
  }

  void column::resize()
  {
    size_t ccap = capacity;
    capacity <<= 1;
    void* new_array = malloc(element_size * capacity);
    mset(elements, new_array, 0, 0, element_size * ccap);
    free(elements);
    elements = new_array;
  }

  void* column::at(const size_t row) const
  {
    return elements + element_size * row;
  }

  void column::rswap(const size_t row)
  {
    const size_t foff = element_size * (count - 1);
    const size_t toff = element_size * row;
    mset(elements, elements, foff, toff, element_size);
    count--;
  }

  void column::copy_element(column* other, const size_t frow, const size_t trow)
  {
    if(trow >= other->capacity)
      other->resize();
    mset(
      elements,
      other->elements,
      frow * element_size,
      trow * element_size,
      element_size
    );
    other->count++;
  }
}
