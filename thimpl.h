#pragma once

#include "tecs.h"
#include <algorithm>

namespace ls::lecs
{
  template <typename T, typename... Args>
  void column::aplace(size_t row, Args&&... args)
  {
    if (row >= capacity) resize();
    if (row >= count)
    {
      count++;
    }
    const size_t offset = row * element_size;
    ::new(elements + offset) T(std::forward<Args>(args)...);
  }

#ifdef V2
  template <typename... T>
  void _has_::construct()
  {
    lambda = [this](group* g, int& i)
    {
      int temp = 0;
      size_t amount = 0;
      (traverse<T>(g, temp, amount),...);
      if(temp != amount)
      {
        --i;
        return;
      }
      ++i;
    };
  }

  template <typename... T>
  void _has_not_::construct()
  {
    lambda = [this](group* g, int& i)
    {
      for(auto c : g->components)
      {
        int temp = 1;
        ((traverse<T>(c, temp)),...);
        if(temp <= 0)
        {
          --i;
          return;
        }
      }
      ++i;
    };
  }

  template <typename... T>
  void _exclusive_has_::construct()
  {
    lambda = [this](world* w, group* g, int& i)
    {
      int temp = 0;
      (traverse<T>(w, g, temp),...);
      if(temp < 0)
      {
        --i;
        return;
      }
      ++i;
    };
  }

  template <typename T>
  void _exclusive_has_::traverse(world* w, group* g, int& i)
  {
    const family fam = _family_<component<T>::_id>::_family;
    const cid id = component<T>::_id;
    for(const auto component_in_group : g->components)
    {
      if(component_in_group == id) continue;

      for(const auto component_in_family : w->families[fam])
      {
        if(component_in_family == component_in_group)
        {
          --i;
          return;
        }
      }
    }
  }

  template <typename... T>
  void _fetch_::construct()
  {
    ++amount;
    lambda = [this](world* w, query* q, group* g, const hash gh, size_t& index)
    {
      size_t _index = 0;
      (t_traverse<T...>(w,q,g,gh,index,_index));
      if(_index) ++index;
    };
  }

  template <typename T, typename K, typename... N>
  void _fetch_::t_traverse(world* w, query* q, group* g, const hash gh, size_t& index, size_t& n)
  {
    const cid query_for = component<T>::_id;
    for(size_t k = 0; k < g->components.size(); k++)
    {
      if(g->components[k] == query_for)
      {
        if(q->_ccoms.size() <= index)
        {
          q->_ccoms.push_back({{}, nullptr});
          q->_ccoms[index].columns.push_back(g->columns[w->cid_groups.at(query_for).at(gh)]);
          q->_ccoms[index].g = g;
        }
        else
        {
          q->_ccoms[index].columns.push_back(g->columns[w->cid_groups.at(query_for).at(gh)]);
        }
        ++n;;
      }
    }
    t_traverse<K, N...>(w, q, g, gh, index, n);
  }

  template<typename T>
  void _fetch_::t_traverse(world* w, query* q, group* g, hash gh, size_t& index, size_t& n)
  {
    const cid query_for = component<T>::_id;
    for(size_t k = 0; k < g->components.size(); k++)
    {
      if(g->components[k] == query_for)
      {
        if(q->_ccoms.size() <= index)
        {
          q->_ccoms.push_back({{}, nullptr});
          q->_ccoms[index].columns.push_back(g->columns[w->cid_groups.at(query_for).at(gh)]);
          q->_ccoms[index].g = g;
        }
        else
        {
          q->_ccoms[index].columns.push_back(g->columns[w->cid_groups.at(query_for).at(gh)]);
        }
        ++n;
        return;
      }
    }
  }

  template <typename... T>
  void query::each(void (*lambda)(eid, T*...))
  {
    for(auto& ccom : _ccoms)
    {
      for(size_t i = 0; i < ccom.g->entities.size(); i++)
      {
        size_t k = ccom.columns.size() - 1;
        while(k != SIZE_MAX)
        {
          lambda(
            ccom.g->entities[i],
            exec<T>(ccom, k--, i)...
          );
        }
      }
    }
  }

  template<typename... T, typename... K>
  void query::each(void(*lambda)(eid,std::tuple<K...>&,T*...),std::tuple<K...>& tup)
  {
    for(auto& ccom : _ccoms)
    {
      for(size_t i = 0; i < ccom.g->entities.size(); i++)
      {
        size_t k = ccom.columns.size() - 1;
        while(k != SIZE_MAX)
        {
          lambda(
            ccom.g->entities[i],
            tup,
            exec<T>(ccom, k--, i)...
          );
        }
      }
    }
  }

  template <typename... T>
  void query::batch_each(void (*lambda)(const eid*, size_t, T*...))
  {
    for(auto& ccom : _ccoms)
    {
      size_t k = ccom.columns.size() - 1;
      lambda(
        ccom.g->entities.data(),
        ccom.g->entities.size(),
        all_exec<T>(ccom, k--)...
      );
    }
  }

  template<typename... T, typename... K>
  void query::batch_each(void(*lambda)(const eid*,size_t,std::tuple<K...>&,T*...),std::tuple<K...>& tup)
  {
    for(auto& ccom : _ccoms)
    {
      size_t k = ccom.columns.size() - 1;
      lambda(
        ccom.g->entities.data(),
        ccom.g->entities.size(),
        tup,
        all_exec<T>(ccom, k--)...
      );
    }
  }

  template <typename... T>
  void query::anon_each(void (*lambda)(T*...))
  {
    for(auto& ccom : _ccoms)
    {
      for(size_t i = 0; i < ccom.g->entities.size(); i++)
      {
        size_t k = ccom.columns.size() - 1;
        while(k != SIZE_MAX)
        {
          lambda(exec<T>(ccom, k--, i)...);
        }
      }
    }
  }

  template<typename... T, typename... K>
  void query::anon_each(void(*lambda)(std::tuple<K...>&, T*...),std::tuple<K...>& tup)
  {
    for(auto& ccom : _ccoms)
    {
      for(size_t i = 0; i < ccom.g->entities.size(); i++)
      {
        size_t k = ccom.columns.size() - 1;
        while(k != SIZE_MAX)
        {
          lambda(tup, exec<T>(ccom, k--, i)...);
        }
      }
    }
  }

  template <typename... T>
  void query::batch_anon(void (*lambda)(size_t, T*...))
  {
    for(auto& ccom : _ccoms)
    {
      size_t k = ccom.columns.size() - 1;
      lambda(
        ccom.g->entities.size(),
        all_exec<T>(ccom, k--)...
        );
    }
  }

  template<typename... T, typename... K>
  void query::batch_anon(void(*lambda)(size_t,std::tuple<K...>&,T*...),std::tuple<K...>& tup)
  {
    for(auto& ccom : _ccoms)
    {
      size_t k = ccom.columns.size() - 1;
      lambda(
        ccom.g->entities.size(),
        tup,
        all_exec<T>(ccom, k--)...
        );
    }
  }

  template <typename T>
  T* query::exec(const ccom& com, const size_t column_index, const size_t position)
  {
    return (T*)com.columns[column_index]->at(position);
  }

  template <typename T>
  T* query::all_exec(const ccom& com, const size_t column_index)
  {
    return (T*)com.columns[column_index]->elements;
  }
#endif

#ifdef V3

  template <typename... T>
  void query::each(void (*lambda)(ecsid, T*...))
  {
    for(auto& cache : _caches)
    {
      for(size_t i = 0; i < cache.g->entities.size(); i++)
      {
        size_t k = cache.columns.size() - 1;
        while(k != SIZE_MAX)
        {
          lambda(
            cache.g->entities[i],
            unpack_each<T>(cache, k--, i)...
          );
        }
      }
    }
  }

  template<typename... T, typename... K>
  void query::each(void(*lambda)(ecsid,std::tuple<K...>&,T*...),std::tuple<K...>& tup)
  {
    for(auto& cache : _caches)
    {
      for(size_t i = 0; i < cache.g->entities.size(); i++)
      {
        size_t k = cache.columns.size() - 1;
        while(k != SIZE_MAX)
        {
          lambda(
            cache.g->entities[i],
            tup,
            unpack_each<T>(cache, k--, i)...
          );
        }
      }
    }
  }

  template <typename... T>
  void query::batch_each(void (*lambda)(const ecsid*, size_t, T*...))
  {
    for(auto& cache : _caches)
    {
      size_t k = cache.columns.size() - 1;
      lambda(
        cache.g->entities.data(),
        cache.g->entities.size(),
        unpack_all<T>(cache, k--)...
      );
    }
  }

  template<typename... T, typename... K>
  void query::batch_each(void(*lambda)(const ecsid*,size_t,std::tuple<K...>&,T*...),std::tuple<K...>& tup)
  {
    for(auto& cache : _caches)
    {
      size_t k = cache.columns.size() - 1;
      lambda(
        cache.g->entities.data(),
        cache.g->entities.size(),
        tup,
        unpack_all<T>(cache, k--)...
      );
    }
  }

  template <typename... T>
  void query::anon_each(void (*lambda)(T*...))
  {
    for(auto& cache : _caches)
    {
      for(size_t i = 0; i < cache.g->entities.size(); i++)
      {
        size_t k = cache.columns.size() - 1;
        while(k != SIZE_MAX)
        {
          lambda(unpack_each<T>(cache, k--, i)...);
        }
      }
    }
  }

  template<typename... T, typename... K>
  void query::anon_each(void(*lambda)(std::tuple<K...>&, T*...),std::tuple<K...>& tup)
  {
    for(auto& cache : _caches)
    {
      for(size_t i = 0; i < cache.g->entities.size(); i++)
      {
        size_t k = cache.columns.size() - 1;
        while(k != SIZE_MAX)
        {
          lambda(tup, unpack_each<T>(cache, k--, i)...);
        }
      }
    }
  }

  template <typename... T>
  void query::batch_anon(void (*lambda)(size_t, T*...))
  {
    for(auto& cache : _caches)
    {
      size_t k = cache.columns.size() - 1;
      lambda(
        cache.g->entities.size(),
        unpack_all<T>(cache, k--)...
        );
    }
  }

  template<typename... T, typename... K>
  void query::batch_anon(void(*lambda)(size_t,std::tuple<K...>&,T*...),std::tuple<K...>& tup)
  {
    for(auto& cache : _caches)
    {
      size_t k = cache.columns.size() - 1;
      lambda(
        cache.g->entities.size(),
        tup,
        unpack_all<T>(cache, k--)...
        );
    }
  }

  template <typename T>
  T* query::unpack_each(const cache& com, const size_t column_index, const size_t position)
  {
    return (T*)com.columns[column_index]->at(position);
  }

  template <typename T>
  T* query::unpack_all(const cache& com, const size_t column_index)
  {
    return (T*)com.columns[column_index]->elements;
  }
#endif

  template <typename T>
  void world::include_in_family(const family family)
  {
    if(_families.size() <= family)
      return;
    const ecsid id = _id_<T>::_id;
    if(_families[family].contains(id))
      return;
    _families[family].insert(id);
    // TODO ALL QUERIES THAT HAVE THAT FAMILY AS EXCLUSIVE NEED TO BE UPDATED
  }


  template <typename T>
  bool world::has_relation(const ecsid entity)
  {
    return _entity_relations[entity].contains(_relation_<T>::_id);
  }


  template<typename T>
  const std::vector<ecsid>& world::get_relatives(const ecsid entity)
  {
    return _relations[_relation_<T>::_id].relatives[entity];
  }


  template <typename T>
  void world::add_relation(const ecsid entity, const ecsid other)
  {
    const ecsid id = _relation_<T>::_id;
    const ecsid inv = _relation_<T>::_inverse;
    _entity_relations[entity].insert(id);
    _entity_relations[other].insert(inv);

    _relations[id].relatives[entity].push_back(other);
    _relations[id].entities.push_back(entity);

    _relations[inv].relatives[other].push_back(entity);
    _relations[inv].entities.push_back(other);
  }


  template <typename T>
  void world::remove_relation(const ecsid entity, const ecsid other)
  {
    remove_relation(entity, other, _relation_<T>::_id);
  }





  template<typename T>
  ecsid world::register_id_as_tag()
  {
    const ecsid id = _component_hashes.size();
    const ecsid t_id = id | _tag;
    _component_hashes.push_back(create_hash());
    _component_locations.emplace_back();
    _set(T, t_id);
    return t_id;
  }

  template <typename T, typename K>
  ecsid world::register_as_tuple()
  {
    const ecsid id = _component_hashes.size();
    const ecsid t_id = id | _tuple;
    _component_hashes.push_back(create_hash());
    _component_locations.emplace_back();
    _set_tuple(T, K, t_id);
    return t_id;
  }

  template <typename T>
  ecsid world::register_as_component()
  {
    const ecsid id = _component_hashes.size();
    _component_hashes.push_back(create_hash());
    _component_locations.emplace_back();
    _set(T, id);
    return id;
  }


  template <typename T>
  void world::add(const ecsid entity)
  {
    const ecsid id = _get(T) ? _get(T) : register_id_as_tag<T>();
     // TODO if actual component, add it and set it to zero
    auto& [cgroup, crow] = entities[entity];

    if(_component_locations[id ^ _tag].contains(cgroup->group_hash))
      return;

    const hash chash = _component_hashes[id ^ _tag];
    const hash nhash = thash(cgroup->group_hash, chash);

    group* ngroup = get_next_group(cgroup, nhash, id, false, 0);
    finalize_group(ngroup);

    auto [nrow, swent] = cgroup->ereloc(ngroup, crow);

    entities[swent].row = crow;
    cgroup = ngroup;
    crow = nrow;
  }

  template <typename T, typename K>
  void world::add(const ecsid entity)
  {
    const ecsid id = _get_tuple_id(T, K) ? _get_tuple_id(T, K) : register_as_tuple<T, K>();
    const ecsid realid = id ^ _tag;
    auto& [cgroup, crow] = entities[entity];

    if(_component_locations[realid].contains(cgroup->group_hash))
      return;

    const hash chash = _component_hashes[realid];
    const hash nhash = thash(cgroup->group_hash, chash);

    group* ngroup = get_next_group(cgroup, nhash, id, false, _get_tuple_size(T, K));
    finalize_group(ngroup);

    auto [nrow, swent] = cgroup->ereloc(ngroup, crow);

    entities[swent].row = crow;
    cgroup = ngroup;
    crow = nrow;
  }

  template <typename T, typename K, typename... Args>
  void world::add(const ecsid entity, Args&&... args)
  {
    const ecsid id = _get_tuple_id(T, K) ? _get_tuple_id(T, K) : register_as_tuple<T, K>();
    const ecsid realid = id ^ _tuple;

    auto& [cgroup, crow] = entities[entity];

    if(_component_locations[realid].contains(cgroup->group_hash))
    {
      for(size_t k = 0; k < cgroup->size; k++)
      {
        if(cgroup->components[k] == id)
        {
          cgroup->columns[k]->aplace<T>(crow, std::forward<Args>(args)...);
          return;
        }
      }
    }

    const hash chash = _component_hashes[realid];
    const hash nhash = thash(cgroup->group_hash, chash);

    group* ngroup = get_next_group(cgroup, nhash, id, false, _get_tuple_size(T, K));
    finalize_group(ngroup);

    auto [nrow, swent] = cgroup->ereloc(ngroup, crow);
    ngroup->columns[ngroup->columns.size()-1]->aplace<T>(nrow, std::forward<Args>(args)...);

    entities[swent].row = crow;
    cgroup = ngroup;
    crow = nrow;
  }

  template <typename T, typename... Args>
  void world::add(const ecsid entity, Args&&... args)
  {
    const ecsid id = _get(T) ? _get(T) : register_as_component<T>();
    auto& [cgroup, crow] = entities[entity];

    if(_component_locations[id].contains(cgroup->group_hash))
    {
      for(size_t k = 0; k < cgroup->size; k++)
      {
        if(cgroup->components[k] == id)
        {
          cgroup->columns[k]->aplace<T>(crow, std::forward<Args>(args)...);
          return;
        }
      }
    }

    const hash chash = _component_hashes[id];
    const hash nhash = thash(cgroup->group_hash, chash);

    group* ngroup = get_next_group(cgroup, nhash, id, false, sizeof(T));
    finalize_group(ngroup);

    auto [nrow, swent] = cgroup->ereloc(ngroup, crow);
    ngroup->columns[ngroup->columns.size()-1]->aplace<T>(nrow, std::forward<Args>(args)...);

    entities[swent].row = crow;
    cgroup = ngroup;
    crow = nrow;
  }

  template <typename T>
  void world::add(ecsid entity, ecsid other)
  {
    // TODO
  }

  template <typename T, typename K>
  void world::remove(const ecsid entity)
  {
    const ecsid id = _get_tuple_id(T, K);
    if(!id) return;
    const ecsid realid = id ^ _tag;
    auto& [cgroup, crow] = entities[entity];
    if(!_component_locations[realid].contains(cgroup->group_hash)) return;

    const hash chash = _component_hashes[realid];
    const hash nhash = thash(cgroup->group_hash, chash);

    group* ngroup = get_next_group(cgroup, nhash, id, true, 0);
    finalize_group(ngroup);

    auto [nrow, swent] = cgroup->ereloc(ngroup, crow, id);

    entities[swent].row = crow;
    cgroup = ngroup;
    crow = nrow;
  }

  template <typename T>
  void world::remove(const ecsid entity)
  {
    const ecsid id = _get(T);
    if(!id) return;
    auto& [cgroup, crow] = entities[entity];
    if(!_component_locations[id].contains(cgroup->group_hash)) return;

    const hash chash = _component_hashes[id];
    const hash nhash = thash(cgroup->group_hash, chash);

    group* ngroup = get_next_group(cgroup, nhash, id, true, 0);
    finalize_group(ngroup);

    auto [nrow, swent] = cgroup->ereloc(ngroup, crow, id);

    entities[swent].row = crow;
    cgroup = ngroup;
    crow = nrow;
  }

  template <typename T>
  void world::remove(ecsid entity, ecsid other)
  {
    // TODO
  }

  template <typename T, typename K>
  bool world::has(const ecsid entity)
  {
    const ecsid id = _get_tuple_id(T, K);
    if(!id) return false;
    const auto& [group, row] = entities[entity];
    return _component_locations[id ^ _tag].contains(group->group_hash);
  }

  template<typename T>
  bool world::has(const ecsid entity)
  {
    const ecsid id = _get(T);
    if(!id) return false;
    const auto& [group, row] = entities[entity];
    return _component_locations[id].contains(group->group_hash);
  }

  template <typename T, typename K>
  T* world::get(const ecsid entity)
  {
    const ecsid id = _get_tuple_id(T, K);
    if(!id || !_get_tuple_size(T, K)) return nullptr;
    const auto& [group, row] = entities[entity];
    for(size_t k = 0; k < group->size; k++)
    {
      if(group->components[k] == id)
        return (T*) group->columns[k]->at(row);
    }
    return nullptr;
  }

  template <typename T>
  T* world::get(const ecsid entity)
  {
    const ecsid id = _get(T);
    if(!id) return nullptr;
    const auto& [group, row] = entities[entity];
    for(size_t k = 0; k < group->size; k++)
    {
      if(group->components[k] == id)
        return (T*) group->columns[k]->at(row);
    }
    return nullptr;
  }


}