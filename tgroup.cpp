#include "tecs.h"

namespace ls::lecs
{
  group::group(group& g, const hash h, const bool is_tag, const cid exclude)
  {
    if(exclude)
    {
      columns.reserve(g.size - 1);
      components.reserve(g.size - 1);
      size = is_tag ? g.size : g.size-1;
    }
    else
    {
      columns.reserve(g.size + 1);
      components.reserve(g.size + 1);
      size = is_tag ? g.size : g.size+1;
    }

    group_hash = h;
    entities.reserve(INIT_CAP_ENTITIES);

    for(int i = 0; i < g.components.size(); i++)
    {
      if(g.components[i] == exclude) continue;
      components.push_back(g.components[i]);
      if(i < g.size)
      {
        columns.push_back(new column{
        INIT_CAP, 0, g.columns[i]->element_size,
        malloc(INIT_CAP * g.columns[i]->element_size)
      });
      }
    }
  }
  group::group(const group& g, const hash h, const bool is_tag, const cid exclude)
  {
    if(exclude)
    {
      columns.reserve(g.size - 1);
      components.reserve(g.size - 1);
      size = is_tag ? g.size : g.size-1;
    }
    else
    {
      columns.reserve(g.size + 1);
      components.reserve(g.size + 1);
      size = is_tag ? g.size : g.size+1;
    }

    group_hash = h;
    entities.reserve(INIT_CAP_ENTITIES);

    for(int i = 0; i < g.components.size(); i++)
    {
      if(g.components[i] == exclude) continue;
      components.push_back(g.components[i]);
      if(i < g.size)
      {
        columns.push_back(new column{
        INIT_CAP, 0, g.columns[i]->element_size,
        malloc(INIT_CAP * g.columns[i]->element_size)
      });
      }
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
      if(world::is_tag(components[i]))       // UNSURE
        continue;
      for(int k = 0; k < size; k++)
      {
        if(components[i] == to->components[k])
        {
          columns[i]->copy_element(to->columns[k], frow, new_row);
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