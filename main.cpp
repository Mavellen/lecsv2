#include <iostream>

#include "tecs.h"
#include "thimpl.h"

using namespace ls::lecs;

struct position{ int x,y; };
constexpr cid pid = 1;

struct velocity{ int x,y,z; };
constexpr cid vid = 2;

register_component(position, pid);
register_component(velocity, vid);

// TODO exclusivity in query

int main()
{
  world w;

  eid e1 = w.new_entity();
  eid e2 = w.new_entity();

  w.add<position>(e1, 1, 2);
  w.add<velocity>(e1, 10, 20, 30);
  w.add<position>(e2, 3, 4);

  auto q1 = new query;
  q1->has<position>();
  q1->has_not<velocity>();
  // TODO exclusivity in query
  q1->fetch<position>();

  auto lambda = +[](eid entity, position* p)
  {
    std::cout << "entity" << entity << std::endl;
    std::cout << "at " << p->x << " " << p->y << std::endl;
    std::cout << " --- " << std::endl;
  };

  w.register_query(q1);

  q1->each(lambda);

}