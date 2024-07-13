#include "tecs.h"

namespace ls::lecs
{
  group::group(group& g, const hash h ,const cid exclude)
  {
    size_t reserve = exclude ? g.size - 1 : g.size + 1;
    size = exclude ? g.size - 1 : g.size + 1;
    columns.reserve(reserve);
    components.reserve(reserve);
    entities.reserve(INIT_CAP_ENTITIES);
    group_hash = h;
    for(int i = 0; i < g.size; i++)
    {
      if(g.components[i] == exclude) continue;
      columns.push_back(new column{
        INIT_CAP, g.components[i], g.columns[i]->element_size,
        malloc(INIT_CAP * g.columns[i]->element_size)
      });
      components.push_back(g.components[i]);
    }
  }
  group::group(const group& g, const hash h, const cid exclude)
  {
    size_t reserve = exclude ? g.size - 1 : g.size + 1;
    size = exclude ? g.size - 1 : g.size + 1;
    columns.reserve(reserve);
    components.reserve(reserve);
    entities.reserve(INIT_CAP_ENTITIES);
    group_hash = h;
    for(int i = 0; i < g.size; i++)
    {
      if(g.components[i] == exclude) continue;
      columns.push_back(new column{
        INIT_CAP, g.components[i], g.columns[i]->element_size,
        malloc(INIT_CAP * g.columns[i]->element_size)
      });
      components.push_back(g.components[i]);
    }
  }

  eid group::evict_entity(const size_t row)
  {
    const eid last_pos = entities.size() - 1;
    const eid swapped_eid = entities[last_pos];
    for (const auto column : columns)
    {
      column->rswap(row);
    }
    entities[row] = swapped_eid;
    entities.pop_back();
    return swapped_eid;
  }

  std::pair<size_t, eid> group::ereloc(group* to, const size_t frow, const cid excludee)
  {
    const size_t new_row = to->entities.size();
    for(int i = 0; i < size; i++)
    {
      if(components[i] == excludee) continue;
      for(int k = 0; k < size; k++)
      {
        if(components[i] == to->components[i])
        {
          columns[i]->copy_element(to->columns[i], frow, new_row);
          break;
        }
      }
    }
    to->entities.push_back(entities[frow]);
    const eid swapped_eid = evict_entity(frow);
    return {new_row, swapped_eid};
    // for(auto* column : columns)
    // {
    //   if(column->component_id == excludee) continue;
    //   for(auto* new_col : to->columns)
    //   {
    //     if(new_col->component_id == column->component_id)
    //     {
    //       column->copy_element(new_col, frow, new_row);
    //       break;
    //     }
    //   }
    // }
  }
}