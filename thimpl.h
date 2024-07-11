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
      row = count;
      count++;
    }
    const size_t offset = row * element_size;
    ::new(elements + offset) T(std::forward<Args>(args)...);
  }

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
    // for all components in that group
    for(const auto component_in_group : g->components)
    {
      // if its component T, ignore
      if(component_in_group == id) continue;

      // otherwise for all components in the family of component T
      for(const auto component_in_family : w->families[fam])
      {
        // if family contains component of that group
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
      bool n = false;
      (t_traverse<T>(w,q,g,gh,index,n),...);
      if(n) ++index;
    };
  }

  template <typename T>
  void _fetch_::t_traverse(world* w, query* q, group* g, const hash gh, size_t& index, bool& n)
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
        n = true;
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
        size_t k = 0;
        while(k < ccom.columns.size())
        {
          lambda(
            ccom.g->entities[i],
            exec<T>(ccom, (k++), i)...
          );
        }
      }
    }
  }

  template <typename... T>
  void query::batch_each(void (*lambda)(eid*, size_t, T*...))
  {
    for(auto& ccom : _ccoms)
    {
      size_t k = 0;
      lambda(
        ccom.g->entities.data(),
        ccom.g->entities.size(),
        all_exec<T>(ccom, k++)...
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
        size_t k = 0;
        while(k < ccom.columns.size())
        {
          lambda(exec<T>(ccom, (++k) - 1, i)...);
        }
      }
    }
  }

  template <typename... T>
  void query::anon_batch(void (*lambda)(size_t, T*...))
  {
    for(auto& ccom : _ccoms)
    {
      size_t k = 0;
      lambda(
        ccom.g->entities.size(),
        all_exec<T>(ccom, k++)...
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


  template <typename T>
  void world::w_register_component()
  {
    std::string str(HASH_SIZE, 0);
    std::ranges::generate_n(str.begin(), HASH_SIZE, rchar);
    _hash_<component<T>::_id>::_hash = hasher(str);
  }

  template <typename T>
  int world::include_in_family(const cid family)
  {
    if(families.size() <= family)
      return -1;

    lecs::family& f = _family_<component<T>::_id>::_family;
    if(f == family)
      return 1;

    f = family;
    families[f].push_back(component<T>::_id);

    return (f != family) ? 1 : 0;
  }

  template <typename T>
  bool world::has(const eid entity)
  {
    const auto& [group_hash, row] = entities[entity];
    const auto& [group_ptr] = groups[group_hash];
    const cid id = component<T>::_id;

    return std::any_of(
      group_ptr->components.begin(),
      group_ptr->components.end(),
      [=](const cid c){return c == id;}
      );
  }

  template <typename T>
  T* world::get(const eid entity)
  {
    const auto& [group_hash, row] = entities[entity];
    const size_t index = cid_groups[component<T>::_id].at(group_hash);
    return (T*) groups[group_hash].group_ptr->columns[index]->at(row);
  }

  template <typename T, typename... Args>
  void world::add(const eid entity, Args&&... args)
  {
    const cid cid_id = component<T>::_id;
    hash cid_hash = _hash_<cid_id>::_hash;
    if(cid_hash == 0)
    {
      w_register_component<T>();
      cid_hash = _hash_<cid_id>::_hash;
    }
    const size_t cid_size = component<T>::_size;

    auto& [group_hash, current_row]= entities[entity];
    const auto& [group_ptr] = groups[group_hash];

    for(const auto cid : group_ptr->components)
    {
      if(cid == cid_id)
      {
        const size_t index = cid_groups[cid_id].at(group_hash);
        group_ptr->columns[index]->aplace<T>(current_row, std::forward<Args>(args)...);
        return;
      }
    }

    const hash next_group_hash = thash(group_hash, cid_hash);

    group* next_group_ptr;
    if(!groups.contains(next_group_hash)) [[unlikely]]
    {
      next_group_ptr = create_group(group_ptr, next_group_hash);
      cid_groups[cid_id][next_group_hash] = next_group_ptr->components.size();
      next_group_ptr->components.push_back(cid_id);

      next_group_ptr->columns.push_back(
          new column{
            INIT_CAP,
            0,
            cid_size,
            cid_id,
            malloc(cid_size * INIT_CAP)
          });

      register_group(next_group_ptr, next_group_hash);
    }
    else [[likely]] next_group_ptr = groups.at(next_group_hash).group_ptr;

    auto [next_row, swapped_entity] = group_ptr->ereloc(next_group_ptr, current_row);
    next_group_ptr->columns[cid_groups[cid_id].at(next_group_hash)]->aplace<T>(next_row, std::forward<Args>(args)...);

    entities[swapped_entity].row = current_row;
    group_hash = next_group_hash;
    current_row = next_row;
  }

  template <typename T>
  void world::remove(const eid entity)
  {
    const cid cid_id = component<T>::_id;
    const hash cid_hash = _hash_<cid_id>::_hash;
    if(cid_hash == 0)
      return;

    auto& [group_hash, current_row]= entities[entity];
    const auto& [group_ptr] = groups[group_hash];

    const hash next_group_hash = thash(group_hash, cid_hash);

    group* next_group_ptr = nullptr;
    if(!groups.contains(next_group_hash)) [[unlikely]]
    {
      next_group_ptr = create_group(group_ptr, next_group_hash, cid_id);
      register_group(next_group_ptr, next_group_hash);
    }
    else [[likely]] next_group_ptr = groups.at(next_group_hash).group_ptr;

    auto [next_row, swapped_entity] = group_ptr->ereloc(next_group_ptr, current_row, cid_id);

    entities[swapped_entity].row = current_row;
    group_hash = next_group_hash;
    current_row = next_row;
  }

}