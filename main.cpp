#include <iostream>
#include <random>

#include "tecs.h"
#include "thimpl.h"

using namespace ls::lecs;

#ifdef V2
new_component(test, 1);


struct position { int x,y; };
struct velocity { int dx,dy; };
struct size { int width,height; };

register_component(position, pid, 1);
register_component(velocity, vid, 2);
register_component(size, sid, 3);
#endif

struct position { int x,y; };
struct velocity { int dx,dy,dw,dz; };
struct size { int width,height,depth; };

new_component(position, 1);
new_component(velocity, 2);
new_component(size, 3);


int main()
{
  constexpr int WIDTH = 500;
  constexpr int HEIGHT = 500;
  constexpr size_t ENTITY_CAP = 100000;
  constexpr int per_row = 2;

  constexpr int recwidth = 50;
  constexpr int recheight = 50;

  std::random_device dev;
  std::mt19937 rng(dev());
  std::uniform_int_distribution<std::mt19937::result_type> dist(0,255);
  std::uniform_int_distribution<std::mt19937::result_type> dist2(0,5);

  world w {3};

  std::vector<eid> entities {};
  entities.reserve(ENTITY_CAP);
  {
    int posx = 0, posy = 0;
    size_t i = 0;
    do
    {
      const eid entity = w.new_entity();
      entities.push_back(entity);
      w.add<position>(entity, posx, 6);
      w.add<size>(entity, posx, posy, 7);
      w.add<velocity>(entity, posx, posy, 8, 9);
      ++i;
      posx++;
      posy++;
      if(posx == 5)
      {
        posx = 0;
        posy = 0;
      }
    }while(i < ENTITY_CAP);
  }

  auto query1 = new query;
  query1->has<size, velocity, position>();
  query1->fetch<size, velocity, position>();

  w.register_query(query1);

  auto l1 = +[](eid entity, size* s, velocity* v, position* p)
  {
    std::cout << "e " << entity-1 << std::endl;
    std::cout << "size " << s->width << " " << s->height << " " << s->depth << std::endl;
    std::cout << "velo " << v->dx << " " << v->dy << " " << v->dw << " " << v->dz << std::endl;
    std::cout << "pos " << p->x << " " << p->y << std::endl;
  };

  query1->each(l1);

#ifdef V2
  world w {};


  std::vector<eid> entities {};
  entities.reserve(ENTITY_CAP);
  {
    int posx = WIDTH/ENTITY_CAP - recwidth/2, posy = HEIGHT/ENTITY_CAP - recheight/2;
    size_t i = 0;
    do
    {
      const eid entity = w.new_entity();
      entities.push_back(entity);
      w.add<position>(entity, posx, posy);
      w.add<size>(entity, recwidth, recheight);
      size_t dx = dist2(rng);
      size_t dy = dist2(rng);
      w.add<velocity>(entity, dx, dy);
      ++i;
      posx+=WIDTH/2;
      if(i % per_row == 0)
      {
        posx = WIDTH/ENTITY_CAP - recwidth/2;
        posy += HEIGHT/2;
      }
    }while(i < ENTITY_CAP);
  }

  auto q2 = new query;
  q2->has<position, velocity>();
  q2->fetch<position, velocity>();

  w.register_query(q2);

  int dt = 0;

  auto move_system = +[](const eid entity, std::tuple<int*>& t, position* p, velocity* v)
  {
    size_t e = entity;
    int* dt = std::get<0>(t);
    (*dt)++;
    std::cout << e << std::endl;
  };
  auto move_system_simple = +[](const eid entity, position* p, const velocity* v)
  {
    size_t e = entity;
  };

  auto tuple = std::make_tuple(&dt);

  //q2->each(move_system);
  q2->each(move_system, tuple);
  //q2->each_args(move_system_simple);
  //q2->each(move_system_simple);
  //q->batch_each(lambda2);
#endif
}