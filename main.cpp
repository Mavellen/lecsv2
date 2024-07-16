#include <iostream>
#include <random>

#include "tecs.h"
#include "thimpl.h"

using namespace ls::lecs;

#define loge(e) std::cout << "entity " << e << std::endl
#define logp(p) std::cout << p->x << " " << p->y << std::endl
#define logv(v) std::cout << v->dx << " " << v->dy << " " << v->dw << " " << v->dz << std::endl;
#define logs(s) std::cout << s->width << " " << s->height << " " << s->depth << std::endl
#define delimit() std::cout << " --- " << std::endl

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
struct t{};
struct f{};

// new_component(position, 1);
// new_component(velocity, 2);
// new_component(size, 3);
//
// new_tag(t, 4);

// TODO Relationships
// TODO INVALIDATION MECHANISMS
// TODO EVENTS LIKE ADDED COMPONENT

int main()
{
  constexpr int WIDTH = 500;
  constexpr int HEIGHT = 500;
  constexpr size_t ENTITY_CAP = 1000;
  constexpr int per_row = 2;

  constexpr int recwidth = 50;
  constexpr int recheight = 50;

  std::random_device dev;
  std::mt19937 rng(dev());
  std::uniform_int_distribution<std::mt19937::result_type> dist(0,255);
  std::uniform_int_distribution<std::mt19937::result_type> dist2(0,5);

  world w;
  std::vector<ecsid> entities {};
  {
    size_t i = 1;
    do{
      const ecsid entity = w.entity();
      entities.push_back(entity);
      // w.add<size>(entity, posx, posy, 7);
      // w.add<position>(entity, posx, 6);
      // w.add<velocity>(entity, posx, posy, 8, 9);
      if(i <= 100)
      {
        w.add<position>(entity, i, 6);
      }
      else if(i > 100 && i <= 200)
      {
        w.add<velocity>(entity, i, i, 8, 9);
      }
      else if(i > 200 && i <= 300)
      {
        w.add<size>(entity, i, i, 7);
      }
      else
      {
        w.add<size>(entity, i, i, 7);
        w.add<position>(entity, i, 6);
        w.add<velocity>(entity, i, i, 8, 9);
      }

      ++i;
      }while(i-1 < ENTITY_CAP);
  }

  query* q = new query;
  q->has<size, velocity, position>();
  q->fetch<size, velocity, position>();

  query* q2 = new query;
  q2->has<position>();
  q2->fetch<position>();

  w.register_query(q);
  w.register_query(q2);

  auto l1 = +[](ecsid entity, size* s, velocity* v, position* p)
  {
    loge(entity);
    logs(s);
    logv(v);
    logp(p);
    delimit();
  };

  auto l2 = +[](const ecsid* entities, size_t size, ::size* s, velocity* v, position* p)
  {
    for(size_t k = 0; k < size; k++)
    {
      loge(entities[k]);
      logs((s + k));
      logv((v + k));
      logp((p + k));
      delimit();
    }
  };

  auto la2 = +[](const ecsid* entities, size_t size, position* p)
  {
    for(size_t k = 0; k < size; k++)
    {
      loge(entities[k]);
      logp((p + k));
      delimit();
    }
  };

  auto l3 = +[](::size* s, velocity* v, position* p)
  {
    logs(s);
    logv(v);
    logp(p);
    delimit();
  };

  auto l4 = +[](size_t size, ::size* s, velocity* v, position* p)
  {
    for(size_t k = 0; k < size; k++)
    {
      logs((s + k));
      logv((v + k));
      logp((p + k));
      delimit();
    }
  };



  //q->batch_each(l2);
  q2->batch_each(la2);



  // world w {4};
  //
  // std::vector<eid> entities {};
  // entities.reserve(ENTITY_CAP);
  // {
  //   int posx = 0, posy = 0;
  //   size_t i = 0;
  //   do
  //   {
  //     const eid entity = w.new_entity();
  //     entities.push_back(entity);
  //     // w.add<size>(entity, posx, posy, 7);
  //     // w.add<position>(entity, posx, 6);
  //     // w.add<velocity>(entity, posx, posy, 8, 9);
  //     w.add<size>(entity, 0, 0, 7);
  //     w.add<position>(entity, 0, 6);
  //     w.add<velocity>(entity, 0, 0, 8, 9);
  //     ++i;
  //     posx++;
  //     posy++;
  //     // if(posx == 5)
  //     // {
  //     //   posx = 0;
  //     //   posy = 0;
  //     // }
  //   }while(i < ENTITY_CAP);
  // }
  //
  // auto query1 = new query;
  // query1->has<size, velocity, position>();
  // query1->fetch<position, velocity, size>();
  //
  // auto query2 = new query;
  // query2->has<t, position>();
  // query2->fetch<position>();
  //
  // w.register_query(query1);
  // w.register_query(query2);
  //
  // w.tag<t>(entities[0]);
  // w.tag<t>(entities[100]);
  // w.tag<t>(entities[69]);
  //
  // auto l1 = +[](eid entity, position* p, velocity* v, size* s)
  // {
  //   std::cout << "e " << entity-1 << std::endl;
  //   std::cout << "size " << s->width << " " << s->height << " " << s->depth << std::endl;
  //   std::cout << "velo " << v->dx << " " << v->dy << " " << v->dw << " " << v->dz << std::endl;
  //   std::cout << "pos " << p->x << " " << p->y << std::endl;
  // };
  //
  // auto l2 = +[](const eid* entity, size_t amount, position* p, velocity* v, size* s)
  // {
  //   for(int i = 0 ;i < amount; i++)
  //   {
  //     std::cout << "e " << entity[i]-1 << std::endl;
  //     std::cout << "size " << s[i].width << " " << s[i].height << " " << s[i].depth << std::endl;
  //     std::cout << "velo " << v[i].dx << " " << v[i].dy << " " << v[i].dw << " " << v[i].dz << std::endl;
  //     std::cout << "pos " << p[i].x << " " << p[i].y << std::endl;
  //   }
  // };
  //
  // auto l3 = +[](position* p, velocity* v, size* s)
  // {
  //   std::cout << "size " << s->width << " " << s->height << " " << s->depth << std::endl;
  //   std::cout << "velo " << v->dx << " " << v->dy << " " << v->dw << " " << v->dz << std::endl;
  //   std::cout << "pos " << p->x << " " << p->y << std::endl;
  // };
  //
  // auto l4 = +[](size_t amount, position* p, velocity* v, size* s)
  // {
  //   for(int i = 0 ;i < amount; i++)
  //   {
  //     std::cout << "size " << s[i].width << " " << s[i].height << " " << s[i].depth << std::endl;
  //     std::cout << "velo " << v[i].dx << " " << v[i].dy << " " << v[i].dw << " " << v[i].dz << std::endl;
  //     std::cout << "pos " << p[i].x << " " << p[i].y << std::endl;
  //   }
  // };
  //
  // auto l5 = +[](eid entity, position* p)
  // {
  //   std::cout << entity << std::endl;
  //   std::cout << p->x << " " << p->y << std::endl;
  // };
  //
  // //query2->each(l5);
  // std::cout << " --- " << std::endl;
  // query1->each(l1);
  // std::cout << " --- " << std::endl;
  // //query1->batch_each(l2);
  // std::cout << " --- " << std::endl;
  // //query1->anon_each(l3);
  // std::cout << " --- " << std::endl;
  // //query1->batch_anon(l4);

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