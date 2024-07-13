#include "tecs.h"

namespace ls::lecs
{
#ifdef V2
  void query::inspect_group(world* w, group* g, const hash gh, size_t& index)
  {
    int i = 0;
    _has.execute(g, i);
    if(i <= 0) return;

    i = 0;
    _has_not.execute(g, i);
    if(i <= 0) return;

    i = 0;
    _exclusive_has.execute(w, g, i);
    if(i <= 0) return;

    _fetch.execute(w, this, g, gh, index);
  }
#endif
#ifdef V3
  void query::inspect_group(group* g)
  {
    if(!has_traverse(g)) return;
    if(!hasnt_traverse(g)) return;
    if(!exclusive_traverse(g)) return;
    fetch_traverse(g);
  }

  bool query::has_traverse(group* g)
  {
    for(cid must_have : _has)
    {
      bool had = false;
      for(cid component : g->components)
      {
        if(must_have == component)
        {
          had = true;
          break;
        }
      }
      if(!had) return false;
    }
    return true;
  }

  bool query::hasnt_traverse(group* g)
  {
    for(cid must_not_have : _hasnt)
    {
      for(cid component : g->components)
      {
        if(component == must_not_have) return false;
      }
    }
    return true;
  }

  void query::fetch_traverse(group* g)
  {
    size_t current_size = _caches.size();
    for(int i = 0; i < _fetch.size(); i++)
    {
      cid query_for = _fetch[i];
      for(int k = 0; k < g->components.size(); k++)
      {
        if(g->components[k] == query_for)
        {
          if(_caches.size() <= current_size)
          {
            _caches.push_back({
              {
                g->columns[k]
              }, nullptr});
            _caches[_caches.size()-1].g = g;
          }
          else
          {
            _caches[_caches.size()-1].columns.push_back(g->columns[k]);
          }
        }
      }
    }
  }

#endif
}