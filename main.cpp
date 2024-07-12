#include <iostream>
#include <random>

#include "tecs.h"
#include "thimpl.h"

using namespace ls::lecs;


struct position{ int x,y; };
struct velocity{ int x,y,z,w; };

register_component(position, pid, 1);
register_component(velocity, vid, 2);


int main()
{
  constexpr int WIDTH = 1000;
  constexpr int HEIGHT = 1000;
  constexpr size_t ENTITY_CAP = 1000;
  constexpr int per_row = 2;
  constexpr int recwidth = WIDTH/per_row;
  constexpr int recheight = HEIGHT/(ENTITY_CAP/2);
  std::random_device dev;
  std::mt19937 rng(dev());
  std::uniform_int_distribution<std::mt19937::result_type> dist(0,255);

  world w {};

  std::vector<eid> entities {};
  entities.reserve(ENTITY_CAP);
  {
    int posx = 1, posy = 1;
    size_t i = 0;
    do
    {
      const eid entity = w.new_entity();
      entities[i] = entity;
      w.add<position>(entity, posx, posy);
      w.add<velocity>(entity, posx, posy, posx, posy);
      ++i;
      posx++;
      posy++;
      // if(posx==9)
      // {
      //   posx = 1;
      //   posy = 1;
      // }
    }while(i < ENTITY_CAP);
  }

  const eid entity = w.new_entity();
  w.add<position>(entity, 0, 0);
  w.add<velocity>(entity, 0, 0, 0, 0);

  query* q = new query;
  q->has<position, velocity>();
  q->fetch<position, velocity>();

  w.register_query(q);

  auto lambda = +[](eid entity, position* p, velocity* v)
  {
    std::cout << "entity " << entity << " at " << p->x << " " << p->y << std::endl;
    std::cout << "with " << v->x << " " << v->y << " " << v->z << " " << v->w << std::endl;
    std::cout << "---" << std::endl;
  };
  auto lambda2 = +[](const eid* entities, size_t size, position* p, velocity* v)
  {
    for(size_t i = 0; i < size; i++)
    {
      std::cout << "entity " << entities[i] << " at " << p[i].x << " " << p[i].y << std::endl;
      std::cout << "with " << v[i].x << " " << v[i].y << " " << v[i].z << " " << v[i].w << std::endl;
      std::cout << "---" << std::endl;
    }
  };

  //q->each(lambda);
  q->batch_each(lambda2);
}