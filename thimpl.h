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

  template <typename T, typename K>
  void world::include_in_family(const family family)
  {
    if(_families.size() <= family)
      return;
    const ecsid id = real_id(_get_tuple_id(T, K));
    if(_families[family].contains(id))
      return;
    _families[family].insert(id);
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
    auto& [cgroup, crow] = entities[_get_id(entity)];

    if(_component_locations[id ^ _tag].contains(cgroup->group_hash))
      return;

    const hash chash = _component_hashes[id ^ _tag];
    const hash nhash = thash(cgroup->group_hash, chash);

    group* ngroup = get_next_group(cgroup, nhash, id, false, 0);
    finalize_group(ngroup);

    auto [nrow, swent] = cgroup->ereloc(ngroup, crow);

    entities[_get_id(swent)].row = crow;
    cgroup = ngroup;
    crow = nrow;
  }

  template <typename T, typename K>
  void world::add(const ecsid entity)
  {
    const ecsid id = _get_tuple_id(T, K) ? _get_tuple_id(T, K) : register_as_tuple<T, K>();
    const ecsid realid = id ^ _tag;
    auto& [cgroup, crow] = entities[_get_id(entity)];

    if(_component_locations[realid].contains(cgroup->group_hash))
      return;

    const hash chash = _component_hashes[realid];
    const hash nhash = thash(cgroup->group_hash, chash);

    group* ngroup = get_next_group(cgroup, nhash, id, false, _get_tuple_size(T, K));
    finalize_group(ngroup);

    auto [nrow, swent] = cgroup->ereloc(ngroup, crow);

    entities[_get_id(swent)].row = crow;
    cgroup = ngroup;
    crow = nrow;
  }

  template <typename T, typename K, typename... Args>
  void world::add(const ecsid entity, Args&&... args)
  {
    const ecsid id = _get_tuple_id(T, K) ? _get_tuple_id(T, K) : register_as_tuple<T, K>();
    const ecsid realid = id ^ _tuple;

    auto& [cgroup, crow] = entities[_get_id(entity)];

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

    entities[_get_id(swent)].row = crow;
    cgroup = ngroup;
    crow = nrow;
  }

  template <typename T, typename... Args>
  void world::add(const ecsid entity, Args&&... args)
  {
    const ecsid id = _get(T) ? _get(T) : register_as_component<T>();
    auto& [cgroup, crow] = entities[_get_id(entity)];

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
    //ngroup->columns.back()->aplace<T>(nrow, std::forward<Args>(args)...);
    entities[_get_id(swent)].row = crow;
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
    auto& [cgroup, crow] = entities[_get_id(entity)];
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
    auto& [cgroup, crow] = entities[_get_id(entity)];
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
    const auto& [group, row] = entities[_get_id(entity)];
    return _component_locations[id ^ _tag].contains(group->group_hash);
  }

  template<typename T>
  bool world::has(const ecsid entity)
  {
    const ecsid id = _get(T);
    if(!id) return false;
    const auto& [group, row] = entities[_get_id(entity)];
    return _component_locations[id].contains(group->group_hash);
  }

  template <typename T, typename K>
  T* world::get(const ecsid entity)
  {
    const ecsid id = _get_tuple_id(T, K);
    if(!id || !_get_tuple_size(T, K)) return nullptr;
    const auto& [group, row] = entities[_get_id(entity)];
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
    const auto& [group, row] = entities[_get_id(entity)];
    for(size_t k = 0; k < group->size; k++)
    {
      if(group->components[k] == id)
        return (T*) group->columns[k]->at(row);
    }
    return nullptr;
  }


}