#include "tecs.h"

namespace ls::lecs
{
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

  std::pair<size_t, ecsid> group::ereloc(group* to, const size_t frow, const ecsid excludee)
  {
    const size_t new_row = to->entities.size();
    for(int i = 0; i < size; i++)
    {
      if(components[i] == excludee || is_tag(components[i])) continue;
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