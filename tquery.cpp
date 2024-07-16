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
  void query::inspect_group(world* w, group* g)
  {
    if(!has_traverse(g)) return;
    if(!hasnt_traverse(g)) return;
    if(!exclusive_traverse(w, g)) return;
    fetch_traverse(g);
  }

  bool query::has_traverse(group* g)
  {
    for(ecsid must_have : _has)
    {
      bool had = false;
      for(ecsid component : g->components)
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
    for(ecsid must_not_have : _hasnt)
    {
      for(ecsid component : g->components)
      {
        if(component == must_not_have) return false;
      }
    }
    return true;
  }

  bool query::exclusive_traverse(world* w, group* g)
  {
    for(auto exclusive : _exclusive_in_family)
    {
      family family = 0;
      for(int i = 0; i < w->_families.size(); i++)
      {
        if(w->_families[family].contains(exclusive))
        {
          family = i;
          break;
        }
      }
      if(family == 0) continue;
      for(auto c : g->components)
      {
        if(w->_families[family].contains(c)) return false;
      }
    }
    return true;
  }

  void query::fetch_traverse(group* g)
  {
    size_t current_size = _caches.size();
    for(int i = 0; i < _fetch.size(); i++)
    {
      ecsid query_for = _fetch[i];
      if(is_tag(query_for) || is_tuple(query_for)) continue;
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
          break;
        }
      }
    }
  }

  std::unordered_set<hash> query::intersect(std::unordered_set<hash>& one, std::unordered_set<hash>& two)
  {
    std::unordered_set<hash> three {};
    for(auto hash : one)
    {
      if(two.contains(hash)) three.insert(hash);
    }
    return three;
  }

  void query::create_cache(world* w)
  {
    ecsid of_cid = 0;
    std::unordered_set<hash> one {}, two {}, three {};
    int last_mod = 1;

    if(!_has.empty())
    {
      of_cid = real_id(_has[0]);
      one = { w->_component_locations[of_cid].begin(), w->_component_locations[of_cid].end() };

      for(int k = 1; k < _has.size(); k++)
      {
        of_cid = real_id(_has[k]);
        two = { w->_component_locations[of_cid].begin(), w->_component_locations[of_cid].end() };

        if( k % 2 == 0 )
        {
          one = intersect(three, two);
          last_mod = 1;
        }
        else
        {
          three = intersect(one, two);
          last_mod = 0;
        }
      }

      if(last_mod == 0)
        one = three;
      if(one.empty()) return;
    }
    last_mod = 1;

    if(!_hasnt.empty())
    {
      if(!of_cid)
      {
        of_cid = real_id(_hasnt[0]);
        one = {w->_component_locations[of_cid].begin(), w->_component_locations[of_cid].end()};
      }
      for(int k = 0; k < _hasnt.size(); k++)
      {
        of_cid = real_id(_hasnt[k]);
        two = {w->_component_locations[of_cid].begin(), w->_component_locations[of_cid].end()};

        if( k % 2 == 1 )
        {
          one = intersect(three, two);
          last_mod = 1;
        }
        else
        {
          three = intersect(one, two);
          last_mod = 0;
        }
      }

      if(last_mod == 0)
        one = three;
      if(one.empty()) return;
    }
    last_mod = 1;

    three.clear();
    if(!_exclusive_in_family.empty())
    {
      for(auto hash : one)
      {
        bool exclusive = true;
        for(ecsid check_for : _exclusive_in_family)
        {
          // this is fine, since families are way way sparcer than components
          // it wouldn't make sense to create a container component -> family
          // especially cause registering queries happens infrequently
          family family = 0;
          for(int i = 0; i < w->_families.size(); i++)
          {
            if(w->_families[i].contains(check_for))
            {
              family = i;
              break;
            }
          }
          if(family == 0) continue;
          for(auto c : w->groups[hash]->components)
          {
            if(w->_families[family].contains(c))
            {
              exclusive = false;
              break;
            }
          }
        }
        if(exclusive)
          three.insert(hash);
      }
      one = three;
    }

    if(_fetch.empty()) return;

    for(int k = 0; k < _fetch.size(); k++)
    {
      of_cid = real_id(_fetch[k]);
      two = {w->_component_locations[of_cid].begin(), w->_component_locations[of_cid].end()};

      if( k % 2 == 1)
      {
        one = intersect(three, two);
        last_mod = 1;
      }
      else
      {
        three = intersect(one, two);
        last_mod = 0;
      }
    }

    if(last_mod == 0)
      one = three;
    if(one.empty()) return;

    size_t current_size = 0;
    for(auto hash : one)
    {
      for(ecsid query_for : _fetch)
      {
        if(is_tag(query_for) | is_tuple(query_for)) continue;
        group* g = w->groups[hash];
        for(int k = 0; k < g->size; k++)
        {
          if(g->components[k] == query_for)
          {
            if(_caches.size() <= current_size)
            {
              _caches.push_back({
                  {
                    g->columns[k]
                  },
                g
              });
            }
            else
            {
              _caches[_caches.size()-1].columns.push_back(g->columns[k]);
            }
            break;
          }
        }
      }
      ++current_size;
    }
  }
#endif


}