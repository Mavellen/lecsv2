#include "tecs.h"

namespace ls::lecs
{
  group::group(group& g, const cid exclude)
  {
    for(const auto column : g.columns)
    {
      if(column->component_id == exclude) continue;
      columns.push_back(
        new lecs::column{
          INIT_CAP, 0,
          column->element_size, column->component_id,
          malloc(INIT_CAP * column->element_size)
        }
      );
    }
  }
  group::group(const group& g, const cid exclude)
  {
    for(const auto column : g.columns)
    {
      if(column->component_id == exclude) continue;
      columns.push_back(
        new lecs::column{
          INIT_CAP, 0,
          column->element_size, column->component_id,
          malloc(INIT_CAP * column->element_size)
        }
      );
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

  std::pair<size_t, eid> group::ereloc(group* to, const size_t frow, const bool excludes, const cid excludee)
  {
    const size_t new_row = to->entities.size();
    for(auto* column : columns)
    {
      if(excludes && column->component_id == excludee) continue;
      for(auto* new_col : to->columns)
      {
        if(new_col->component_id == column->component_id)
        {
          column->copy_element(new_col, frow, new_row);
          break;
        }
      }
    }
    to->entities.push_back(entities[frow]);
    const eid swapped_eid = evict_entity(frow);
    return {new_row, swapped_eid};
  }
}