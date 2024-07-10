#include <random>

#include "tecs.h"

#include <cstring>
#include <random>

namespace ls::lecs
{

  world::world()
  {
    groups[VOID_HASH] = {new group};
    cid_groups[VOID_HASH] = {};
  }

  char world::rchar()
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
  }
  hash world::thash(const hash h1, const hash h2) { return h1 ^ h2; }

  eid world::new_entity()
  {
    const auto void_group = groups[VOID_HASH].group_ptr;
    size_t row = void_group->entities.size();
    eid gen_eid;
    if(open_indices.empty())
    {
      gen_eid = e_counter++;
      entities.emplace_back(VOID_HASH, row);
    }
    else
    {
      gen_eid = open_indices.front();
      open_indices.pop();
      entities[gen_eid] = {VOID_HASH, row};
    }
    void_group->entities.push_back(gen_eid);
    return gen_eid;
  }
  void world::destroy_entity(const eid entity)
  {
    auto& [group_hash, row] = entities[entity];
    const auto& [group_ptr] = groups.at(group_hash);
    const eid swapped_entity = group_ptr->evict_entity(row);
    entities[swapped_entity].row = row;
    open_indices.push(entity);
  }

  family world::new_family()
  {
    families.emplace_back();
    return f_counter++;
  }

  void world::register_query(query* q)
  {
    queries.push_back(q);
    size_t index = 0;
    for(const auto& group : groups)
    {
      q->inspect_group(this, group.second.group_ptr, group.first, index);
    }
  }
  void world::register_group(group* g, const hash gh)
  {
    for(auto q : queries)
    {
      size_t index = q->_ccoms.size();
      q->inspect_group(this, g, gh, index);
    }
  }
}
