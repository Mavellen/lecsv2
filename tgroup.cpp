#include "tecs.h"

namespace ls::lecs
{
  // group::group(group& g, const hash h, const bool is_tag, const ecsid exclude)
  // {
  //   if(exclude)
  //   {
  //     columns.reserve(g.size - 1);
  //     components.reserve(g.size - 1);
  //     size = is_tag ? g.size : g.size-1;
  //   }
  //   else
  //   {
  //     columns.reserve(g.size + 1);
  //     components.reserve(g.size + 1);
  //     size = is_tag ? g.size : g.size+1;
  //   }
  //
  //   group_hash = h;
  //   entities.reserve(INIT_CAP_ENTITIES);
  //
  //   for(int i = 0; i < g.components.size(); i++)
  //   {
  //     if(g.components[i] == exclude) continue;
  //     components.push_back(g.components[i]);
  //     if(i < g.size)
  //     {
  //       columns.push_back(new column{
  //       INIT_CAP, 0, g.columns[i]->element_size,
  //       malloc(INIT_CAP * g.columns[i]->element_size)
  //     });
  //     }
  //   }
  // }
  // group::group(const group& g, const hash h, const bool is_tag, const ecsid exclude)
  // {
  //   if(exclude)
  //   {
  //     columns.reserve(g.size - 1);
  //     components.reserve(g.size - 1);
  //     size = is_tag ? g.size : g.size-1;
  //   }
  //   else
  //   {
  //     columns.reserve(g.size + 1);
  //     components.reserve(g.size + 1);
  //     size = is_tag ? g.size : g.size+1;
  //   }
  //
  //   group_hash = h;
  //   entities.reserve(INIT_CAP_ENTITIES);
  //
  //   for(int i = 0; i < g.components.size(); i++)
  //   {
  //     if(g.components[i] == exclude) continue;
  //     components.push_back(g.components[i]);
  //     if(i < g.size)
  //     {
  //       columns.push_back(new column{
  //       INIT_CAP, 0, g.columns[i]->element_size,
  //       malloc(INIT_CAP * g.columns[i]->element_size)
  //     });
  //     }
  //   }
  // }

  ecsid group::evict_entity(const size_t row)
  {
    const ecsid last_pos = entities.size() - 1;
    const ecsid swapped_eid = entities[last_pos];
    for (const auto column : columns)
    {
      column->rswap(row);
    }
    entities[row] = swapped_eid;
    entities.pop_back();
    return swapped_eid;
  }

  std::pair<size_t, ecsid> group::ereloc(group* to, size_t frow, ecsid excludee)
  {
    const size_t new_row = to->entities.size();
    for(int i = 0; i < size; i++)
    {
      if(components[i] == excludee) continue;
      if(is_tag(components[i]))
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
    const ecsid swapped_eid = evict_entity(frow);
    return {new_row, swapped_eid};
  }
}