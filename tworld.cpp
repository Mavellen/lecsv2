#include <random>

#include "tecs.h"
#include <cstring>

#include "thimpl.h"

namespace ls::lecs
{
  world::world()
  {
    groups[VOID_HASH] = {new group()};
    groups[VOID_HASH]->size = 0;
    groups[VOID_HASH]->is_finalized = true;

    _component_locations.reserve(INIT_CAP);
    _component_locations.push_back({VOID_HASH});
    _component_hashes.push_back(VOID_HASH);

    _families.emplace_back();

    entities.reserve(INIT_CAP_ENTITIES);
    entities.push_back({});
  }

  hash world::create_hash()
  {
    std::string str(HASH_SIZE, 0);
    std::ranges::generate_n(str.begin(), HASH_SIZE, []()
    {
      static constexpr auto chars =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
      static constexpr size_t max_index = (strlen(chars) - 1);
      static std::random_device rd;
      static std::mt19937 gen(rd());
      static std::uniform_int_distribution<> distributor(0, max_index);
      return chars[distributor(gen)];
    });
    return hasher(str);
  }

  hash world::thash(const hash h1, const hash h2) { return h1 ^ h2; }

  group* world::get_next_group(const group* current, const hash nhash, const ecsid nid, const bool remove, const size_t size)
  {
    if(groups.contains(nhash)) [[likely]] return groups[nhash];
    auto* ngroup = new group;
    ngroup->group_hash = nhash;
    ngroup->entities.reserve(INIT_CAP_ENTITIES);
    if(remove)
    {
      ngroup->size = is_tag(nid) ? current->size : current->size - 1;
      ngroup->components.reserve(current->components.size()-1);
      ngroup->columns.reserve(is_tag(nid) ? current->size : current->size - 1);
    }
    else
    {
      ngroup->size = is_tag(nid) ? current->size : current->size + 1;
      ngroup->components.reserve(current->components.size() + 1);
      ngroup->columns.reserve(is_tag(nid) ? current->size : current->size + 1);
    }
    for(int i = 0; i < current->components.size(); i++)
    {
      const ecsid id = current->components[i];
      if(remove && id == nid) continue;
      ngroup->components.push_back(id);
      if(i < current->size)
      {
        ngroup->columns.push_back(
          new column{INIT_CAP, 0, current->columns[i]->element_size,
            imalloc(INIT_CAP * current->columns[i]->element_size)}
        );
      }
    }
    if(!remove)
    {
      if(size)
      {
        ngroup->columns.push_back(new column{INIT_CAP, 0, size, imalloc(INIT_CAP * size)});
        ngroup->components.push_back(ngroup->components[ngroup->size-1]);
        ngroup->components[ngroup->size-1] = nid;
      }
      else ngroup->components.push_back(nid);
    }
    return ngroup;
  }

  void world::finalize_group(group* group)
  {
    if(group->is_finalized) return;
    for(const ecsid component : group->components)
    {
      _component_locations[real_id(component)].insert(group->group_hash);
    }
    groups[group->group_hash] = group;
    register_group(group);
    group->is_finalized = true;
  }

  ecsid world::entity()
  {
    const auto void_group = groups[VOID_HASH];
    size_t row = void_group->entities.size();
    ecsid generated;
    if(_open_indices.empty())
    {
      generated = entity_counter++;
      entities.push_back({void_group, row});
    }
    else
    {
      generated = _open_indices.front();
      _open_indices.pop();
      entities[generated] = {void_group, row};
    }
    void_group->entities.push_back(generated);
    return generated;
  }

  void world::erase(const ecsid entity)
  {
    auto& [group, row] = entities[entity];
    const ecsid swapped_entity = group->evict_entity(row);
    entities[swapped_entity] = {group, row};
    _open_indices.push(entity);
    entities[entity].row = -1;
  }



  family world::new_family()
  {
    _families.emplace_back();
    return f_counter++;
  }

  void world::register_query(query* q)
  {
    queries.push_back(q);
    q->create_cache(this);
  }
  void world::register_group(group* g)
  {
    for(auto q : queries)
    {
      q->inspect_group(this, g);
    }
  }

  void world::remove_relation(const ecsid entity, const ecsid other, const ecsid id)
  {
    broker& id_broker = _relations[id];
    broker& inv_broker = _relations[id_broker.inverse];
    std::vector<ecsid>& id_entities = id_broker.entities;
    std::vector<ecsid>& inv_entities = inv_broker.entities;
    std::unordered_map<ecsid, std::vector<ecsid>>& id_map = id_broker.relatives;
    std::unordered_map<ecsid, std::vector<ecsid>>& inv_map = inv_broker.relatives;


    for(size_t i = 0; i < inv_entities.size(); i++)
    {
      if(inv_entities[i] == entity)
      {
        inv_entities[i] = inv_entities[inv_entities.size() - 1];
        inv_entities.pop_back();
        break;
      }
    }

    for(size_t i = 0; i < inv_map[other].size(); i++)
    {
      if(inv_map[other][i] == entity)
      {
        inv_map[other][i] = inv_map[other][inv_map[other][id_map[other].size() - 1]];
        inv_map[other].pop_back();
        break;
      }
    }

    if(id_map[entity].empty())
    {
      id_map.erase(entity);
      _entity_relations[entity].erase(id);
      for(size_t i = 0; i < id_entities.size(); i++)
      {
        if(id_entities[i] == entity)
        {
          id_entities[i] = id_entities[id_entities.size() - 1];
          id_entities.pop_back();
          break;
        }
      }
    }
    if(inv_map[other].empty())
    {
      inv_map.erase(other);
      _entity_relations[other].erase(id_broker.inverse);
      for(size_t i = 0; i < id_map[entity].size(); i++)
      {
        if(id_map[entity][i] == other)
        {
          id_map[entity][i] = id_map[entity][id_map[entity][id_map[entity].size() - 1]];
          id_map[entity].pop_back();
          break;
        }
      }
    }
  }

  void world::clear_relation(const ecsid entity, const ecsid relation)
  {
    broker& id_broker =  _relations[relation];
    broker& inv_broker = _relations[id_broker.inverse];
    for(auto other : id_broker.relatives[entity])
    {
      for(size_t i = 0; i < inv_broker.relatives[other].size(); i++)
      {
        if(inv_broker.relatives[other][i] == entity)
        {
          inv_broker.relatives[other][i] = inv_broker.relatives[other][inv_broker.relatives[other].size() - 1];
          inv_broker.relatives[other].pop_back();
          break;
        }
        if(inv_broker.relatives[other].empty())
        {
          inv_broker.relatives.erase(other);
          for(size_t k = 0; k < inv_broker.entities.size(); k++)
          {
            if(inv_broker.entities[k] == other)
            {
              inv_broker.entities[k] = inv_broker.entities[inv_broker.entities.size() - 1];
              inv_broker.entities.pop_back();
              break;
            }
          }
          _entity_relations[other].erase(id_broker.inverse);
        }
      }
    }
    id_broker.relatives[entity].clear();
    id_broker.relatives.erase(entity);
    for(size_t k = 0; k < id_broker.entities.size(); k++)
    {
      if(id_broker.entities[k] == entity)
      {
        id_broker.entities[k] = id_broker.entities[id_broker.entities.size() - 1];
        id_broker.entities.pop_back();
        break;
      }
    }
    _entity_relations[entity].erase(relation);
  }
}
