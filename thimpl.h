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
  void query::each(void (*lambda)(eid, T*...))
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
  void query::each(void(*lambda)(eid,std::tuple<K...>&,T*...),std::tuple<K...>& tup)
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
  void query::batch_each(void (*lambda)(const eid*, size_t, T*...))
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
  void query::batch_each(void(*lambda)(const eid*,size_t,std::tuple<K...>&,T*...),std::tuple<K...>& tup)
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
    cid id = id_from_type(T);
    if(_families[family].contains(id))
      return;
    _families[family].insert(id);
    // TODO ALL QUERIES THAT HAVE THAT FAMILY AS EXCLUSIVE NEED TO BE UPDATED
  }


  template <typename T>
  bool world::has(const eid entity)
  {
    const auto& [group, row] = entities[entity];

    const cid id = id_from_type(T);

    // UNSURE maybe specilize to has_tag func
    return std::any_of(
      is_tag(id) ? group->components.begin() + group->size
                                     : group->components.begin(),
      group->components.end(),
      [=](const cid c){return c == id;}
      );
  }

  template <typename T>
  T* world::get(const eid entity)
  {
    const auto& [group, row] = entities[entity];
    cid cid_id = id_from_type(T);
    for(int i = 0; i < group->size; i++)
    {
      if(group->components[i] == cid_id)
        return (T*) group->columns[i]->at(row);
    }
    return nullptr;
  }


  template<typename T>
  void world::tag(eid entity)
  {
    const cid cid_id = id_from_type(T);
    if(!(cid_id >> to_tag))   // UNSURE trust them perhaps
      return; // not a tag!

    cid realid = to_realid_tag(cid_id);

    hash cid_hash = _component_hashes[to_realid_tag(cid_id)];
    auto& er = entities[entity];

    for(size_t k = er.group->size; k < er.group->components.size(); k++)
    {
      if(er.group->components[k] == cid_id)
        return;
    }

    const hash next_group_hash = thash(er.group->group_hash, cid_hash);

    group* next_group_ptr;
    if(!groups.contains(next_group_hash)) [[unlikely]]
    {
      next_group_ptr = create_group(er.group, next_group_hash, true);
      _component_groups_vec[to_realid_tag(cid_id)].push_back(next_group_hash);
      next_group_ptr->components.push_back(cid_id);
      register_group(next_group_ptr);
    }
    else [[likely]] next_group_ptr = groups[next_group_hash];

    auto [next_row, swapped_entity]
      = er.group->ereloc(next_group_ptr, er.row);
    entities[swapped_entity].row = er.row;
    er.group = next_group_ptr;
    er.row = next_row;
  }

  template<typename T>
  void world::remove_tag(eid entity)
  {
    const cid cid_id = id_from_type(T);
    if(!(cid_id >> to_tag))   // UNSURE trust them perhaps
      return; // not a tag!

    eid_record& er = entities[entity];

    hash cid_hash = _component_hashes[to_realid_tag(cid_id)];

    const hash next_group_hash = thash(er.group->group_hash, cid_hash);;

    group* next_group_ptr;
    if(!groups.contains(next_group_hash)) [[unlikely]]
    {
      next_group_ptr = create_group(er.group, next_group_hash, true, cid_id);
      _component_groups_vec[to_realid_tag(cid_id)].push_back(next_group_hash);
      register_group(next_group_ptr);
    }
    else [[likely]] next_group_ptr = groups.at(next_group_hash);

    auto [next_row, swapped_entity] = er.group->ereloc(next_group_ptr, er.row, cid_id);

    entities[swapped_entity].row = er.row;
    er.group = next_group_ptr;
    er.row = next_row;
  }

  template <typename T, typename... Args>
  void world::add(const eid entity, Args&&... args)
  {
    const cid cid_id = id_from_type(T);
    if(cid_id >> to_tag) return tag<T>(entity); // dedicated method for tags!

    const hash cid_hash = _component_hashes[cid_id];

    auto& [group_ptr, current_row]= entities[entity];

    for(size_t k = 0; k < group_ptr->size; k++)
    {
      if(group_ptr->components[k] == cid_id)
      {
        group_ptr->columns[k]->aplace<T>(current_row, std::forward<Args>(args)...);
        return;
      }
    }

    const hash next_group_hash = thash(group_ptr->group_hash, cid_hash);

    group* next_group_ptr;
    if(!groups.contains(next_group_hash)) [[unlikely]]
    {
      const size_t cid_size = sizeof(T);
      next_group_ptr = create_group(group_ptr, next_group_hash, false);

      _component_groups_vec[cid_id].push_back(next_group_hash);

      // The group has components up until its size member at which position tags start
      // push the tag at that location to the end of the vector and put the component
      // at that position
      next_group_ptr->components.push_back(next_group_ptr->components[next_group_ptr->size - 1]);
      next_group_ptr->components[next_group_ptr->size - 1] = cid_id;
      // Since columns and components are symmetrical, this is perfect
      next_group_ptr->columns.push_back(
        new column{
          INIT_CAP, 0, cid_size,
          malloc(INIT_CAP * cid_size)
        });

      register_group(next_group_ptr);
    }
    else [[likely]] next_group_ptr = groups.at(next_group_hash);

    auto [next_row, swapped_entity]
      = group_ptr->ereloc(next_group_ptr, current_row);

    for(size_t k = 0; k < next_group_ptr->size; k++)
    {
      if(next_group_ptr->components[k] == cid_id)
      {
        next_group_ptr->columns[k]->aplace<T>(next_row, std::forward<Args>(args)...);
        break;
      }
    }

    entities[swapped_entity].row = current_row;
    group_ptr = next_group_ptr;
    current_row = next_row;
  }

  template <typename T>
  void world::remove(const eid entity)
  {
    const cid cid_id = id_from_type(T);
    if(cid_id >> to_tag) return remove_tag<T>(entity);

    const hash cid_hash = _component_hashes[to_realid_tag(cid_id)];

    auto& [group_ptr, current_row]= entities[entity];

    const hash next_group_hash = thash(group_ptr->group_hash, cid_hash);

    group* next_group_ptr;
    if(!groups.contains(next_group_hash)) [[unlikely]]
    {
      next_group_ptr = create_group(group_ptr, next_group_hash, false, cid_id);
      _component_groups_vec[cid_id].push_back(next_group_hash);
      register_group(next_group_ptr);
    }
    else [[likely]] next_group_ptr = groups.at(next_group_hash);

    auto [next_row, swapped_entity]
      = group_ptr->ereloc(next_group_ptr, current_row, cid_id);

    entities[swapped_entity].row = current_row;
    group_ptr = next_group_ptr;
    current_row = next_row;
  }

  template <typename T, typename K>
  void world::relate(eid entity)
  {
    // Relations:
    // Tag, Tag w/o v .. Tag
    // Entity, Entity .. extra
    cid rel_id = _relation_<T, K>::_id;
    if(!rel_id)
    {
      // this has no value, make it a tag
      // in the other iteration where the relation has values, make it a component instead
      _relation_<T, K>::_id = (_component_hashes.size() | ((cid)1<<to_tag));
      _component_hashes.push_back(create_hash());
    }

    // add this TAG to archetype
  }
}