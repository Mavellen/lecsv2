#include "tecs.h"

namespace ls::lecs
{
  void query::inspect_group(world* w, group* g, const hash gh, size_t& index)
  {
    int i = 0;
    _has.execute(g, i);
    if(i <= 0) return;
    i = 0;
    _has_not.execute(g, i);
    if(i <= 0) return;
    // TODO exclusive
    _fetch.execute(w, this, g, gh, index);
  }

}