#include <random>

#include "tecs.h"

#include <cstring>
#include <random>

namespace ls::lecs
{
  world::world(size_t ct_amount)
  {
    groups[VOID_HASH] = {new group()};
    cid_groups[VOID_HASH] = {};
    _component_hashes.push_back(VOID_HASH);

    std::string str(HASH_SIZE, 0);
    for(size_t i = 0; i < ct_amount; i++)
    {
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
      _component_hashes[0] = hasher(str);
      cid_groups[i] = {};
    }
    _families.emplace_back();
    entities.reserve(INIT_CAP_ENTITIES);
    entities.push_back({});
  }

  hash world::thash(const hash h1, const hash h2) { return h1 ^ h2; }

  eid world::new_entity()
  {
    const auto void_group = groups[VOID_HASH];
    size_t row = void_group->entities.size();
    eid gen_eid;
    if(open_indices.empty())
    {
      gen_eid = e_counter++;
      entities.push_back({void_group, row});
    }
    else
    {
      gen_eid = open_indices.front();
      open_indices.pop();
      entities[gen_eid] = {void_group, row};
    }
    void_group->entities.push_back(gen_eid);
    return gen_eid;
  }
  void world::destroy_entity(const eid entity)
  {
    auto& [group, row] = entities[entity];
    const eid swapped_entity = group->evict_entity(row);
    entities[swapped_entity].row = row;
    open_indices.push(entity);
  }

  family world::new_family()
  {
    _families.emplace_back();
    return f_counter++;
  }

  void world::register_query(query* q)
  {
    queries.push_back(q);
    for(const auto& [hash, group] : groups)
    {
      q->inspect_group(group);
    }
  }
  void world::register_group(group* g, const hash gh)
  {
    for(auto q : queries)
    {
      q->inspect_group(g);
    }
  }

  [[nodiscard]]
  group* world::create_group(const group* og, const hash gh, const cid excludee)
  {
    group* ng;
    if(excludee)
      ng = new group(*og, gh, excludee);
    else ng = new group(*og, gh);

    for(int i = 0; i < ng->size; i++)
    {
      cid_groups[ng->components[i]][gh] = i;
    }
    // size_t index = 0;
    // for(const auto component : og->components)
    // {
    //   if(component == excludee)
    //     continue;
    //   cid_groups[component][gh] = index++;
    // }
    groups[gh] = ng;
    return ng;
  }
}
