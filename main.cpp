#include <iostream>

#include "tecs.h"
#include "thimpl.h"

using namespace ls::lecs;

struct position{ int x,y; };
constexpr cid pid = 1;
// MAKE_COMPONENT(position, pos_id)
//
struct velocity{ int x,y,z; };
constexpr cid vid = 2;
// MAKE_COMPONENT(velocity, vel_id)
//
//
// constexpr qid q1 = 0;
// ADD_QUERY_HAS(1, q1, pos_id);
// ADD_QUERY_HAS_NOT(0, q1);
// ADD_QUERY_FETCH(1, q1, pos_id);
//
// REGISTER_QUERIES(1, q1);

register_component(position, pid);
register_component(velocity, vid);


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
  q1->fetch<position>();

  auto lambda = +[](eid entity, position* p)
  {
    std::cout << "entity" << entity << std::endl;
    std::cout << "at " << p->x << " " << p->y << std::endl;
    std::cout << " --- " << std::endl;
  };

  w.register_query(q1);

  q1->each(lambda);




  // world w = MAKE_WORLD(2, new query<q1>);
  //
  // const eid e1 = w.new_entity();
  // w.add<position>(e1, 1, 2);
  //
  // w.include_in_family<position>(1);
  //
  // auto lambda = +[](eid entity, position* p)
  // {
  //   std::cout << entity << std::endl;
  //   std::cout << p->x << " " << p->y << std::endl;
  // };
  // w.execute<q1>(lambda);
}