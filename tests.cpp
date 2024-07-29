#include <format>
#include <iostream>

#include "lecs.h"

#define msg(m,...) printf(m##,__VA_ARGS__)
#define pass printf("function %s passed\n", __PRETTY_FUNCTION__);
#define fail __PRETTY_FUNCTION__
using namespace ls::lecs;

struct tt {};
struct tc {int x;};
struct c2 {int x, y;};

void cc_swap()
{
  struct test { int x, y; };
  cc alpha {3, 0, sizeof(test), malloc(sizeof(test) * 3)};
  alpha.aplace<test>(0, 1, 1);
  alpha.aplace<test>(1, 2, 3);
  alpha.rswap(0);
  auto t_ref = (test*)alpha.at(0);
  assert(t_ref->x == 2 && t_ref->y == 3 && fail);
  pass
}
void cc_resize()
{
  struct test { int x, y; };
  cc alpha {1, 0, sizeof(test), malloc(sizeof(test) * 1)};
  alpha.aplace<test>(0, 1, 1);
  alpha.aplace<test>(1, 2, 2);
  alpha.aplace<test>(2, 3, 3);
  auto t_ref = (test*) alpha.at(2);
  auto t_ref2 = (test*) alpha.at(1);
  auto t_ref3 = (test*) alpha.at(0);
  assert(t_ref->x == 3 && t_ref->y == 3 &&
    t_ref2->x == 2 && t_ref2->y == 2 && t_ref3->x == 1 && t_ref3->y == 1 && fail);
  pass
}
void cc_copy()
{
  struct test { int x, y; };
  cc alpha {1, 0, sizeof(test), malloc(sizeof(test) * 1)};
  cc beta {1, 0, sizeof(test), malloc(sizeof(test) * 1)};
  alpha.aplace<test>(0, 1, 1);
  beta.aplace<test>(0, 2, 2);
  alpha.aplace<test>(1, 3, 3);
  assert(alpha.count == 2 && fail);
  alpha.copy_element(&beta, 1, 1);
  alpha.rswap(1);
  assert(alpha.count == 1 && beta.count == 2 && fail);
  auto tref = (test*)beta.at(1);
  assert(tref->x == 3 && tref->y == 3 && fail);
  pass
}
void atype_sparse_set()
{
  atype at {0};
  at.include(1, 0);
  assert(at.sparse_components[0] == SIZE_MAX && fail);
  assert(at.sparse_components[1] == 0 && fail);
  assert(at.dense_components[0] == 1);
  assert(at.size == 0);
  at.include(0, 2);
  assert(at.sparse_components[1] == 1 && fail);
  assert(at.sparse_components[0] == 0 && fail);
  assert(at.dense_components[0] == 0 && fail);
  assert(at.dense_components[1] == 1 && fail);
  assert(at.size == 1);
  pass
}
void atype_sparse_set_component_first_include()
{
  struct t { int x, y; };
  atype at {0};
  at.include(0, sizeof(t));
  assert(at.sparse_components[0] == 0 && fail);
  assert(at.dense_components[0] == 0 && fail);
  assert(at.size == 1 && fail);
  at.include(1, 0);
  assert(at.sparse_components[1] == 1 && fail);
  assert(at.dense_components[1] == 1 && fail);
  assert(at.size == 1 && fail);
  pass
}
void atype_sparse_set_only_component_include()
{
  atype at {0};
  at.include(0, 8);
  at.include(1, 12);
  assert(at.sparse_components[0] == 0 && at.sparse_components[1] == 1 && fail);
  assert(at.dense_components[0] == 0 && at.dense_components[1] == 1 && fail);
  assert(at.size == 2 && fail);
  pass

}
void atype_evict()
{
  struct t { int x, y; };
  atype at {0};
  at.entities.push_back(0);
  at.entities.push_back(1);
  at.clcs.push_back(new cc {2, 0, sizeof(t), malloc(sizeof(t) * 2)});
  at.clcs[0]->aplace<t>(0, 1, 1);
  at.clcs[0]->aplace<t>(1, 2, 2);
  ecsid swapped_entity = at.evict_entity(0);
  assert(at.entities.size() == 1 && fail);
  assert(swapped_entity == 1 && fail);
  assert(at.entities[0] == 1 && fail);
  assert(at.clcs[0]->count == 1 && fail);
  auto tref = (t*)at.clcs[0]->at(0);
  assert(tref->x == 2 && tref->y == 2 && fail);
  pass
}
void atype_ereloc_empty()
{
  struct t { int x, y; };
  atype at {0};
  atype at2 {1};
  at.include(0, sizeof(t));
  at2.include(0, sizeof(t));
  at.entities.push_back(0);
  at.entities.push_back(1);
  at.clcs.push_back(new cc {2, 0, sizeof(t), malloc(sizeof(t) * 2)});
  at.clcs[0]->aplace<t>(0, 1, 1);
  at.clcs[0]->aplace<t>(1, 2, 2);
  at2.clcs.push_back(new cc {2, 0, sizeof(t), malloc(sizeof(t) * 2)});
  auto pair = at.ereloc(&at2, 0);
  assert(at.entities.size() == 1 && fail);
  assert(at2.entities.size() == 1 && fail);
  assert(pair.first == 0 && fail);
  assert(pair.second == 1 && fail);
  assert(at.entities[0] == 1 && fail);
  assert(at2.entities[0] == 0 && fail);
  assert(at.clcs[0]->count == 1 && fail);
  assert(at2.clcs[0]->count == 1 && fail);
  auto tref = (t*)at.clcs[0]->at(0);
  auto tref2 = (t*)at2.clcs[0]->at(0);
  assert(tref->x == 2 && tref->y == 2 && fail);
  assert(tref2->x == 1 && tref2->y == 1 && fail);
  pass
}
void atype_ereloc_in_use()
{
  struct t { int x, y; };
  atype at {0};
  atype at2 {1};
  at.include(0, sizeof(t));
  at2.include(0, sizeof(t));
  at.entities.push_back(0);
  at.entities.push_back(1);
  at2.entities.push_back(2);
  at.clcs[0]->aplace<t>(0, 1, 1);
  at.clcs[0]->aplace<t>(1, 2, 2);
  at2.clcs[0]->aplace<t>(0, 3, 3);
  auto pair = at.ereloc(&at2, 0);
  assert(at.entities.size() == 1 && fail);
  assert(at2.entities.size() == 2 && fail);
  assert(pair.first == 1 && fail);
  assert(pair.second == 1 && fail);
  assert(at.entities[0] == 1 && fail);
  assert(at2.entities[0] == 2 && fail);
  assert(at2.entities[1] == 0 && fail);
  assert(at.clcs[0]->count == 1 && fail);
  assert(at2.clcs[0]->count == 2 && fail);
  auto tref = (t*)at.clcs[0]->at(0);
  auto tref2 = (t*)at2.clcs[0]->at(0);
  auto tref3 = (t*)at2.clcs[0]->at(1);
  assert(tref->x == 2 && tref->y == 2 && fail);
  assert(tref2->x == 3 && tref2->y == 3 && fail);
  assert(tref3->x == 1 && tref3->y == 1 && fail);
  pass
}
void sim_next_archetype_created()
{
  sim s;
  assert(s.atypes.size() == 1 && fail);
  assert(s.atypes[0]->hash == 0 && fail);
  atype* at = s.get_next_atype(s.atypes[0], 1, 10, false, 0);
  assert(at->hash == 1 && fail);
  assert(at->size == 0 && fail);
  assert(at->clcs.empty() && fail);
  assert(at->dense_components.size() == 1 && fail);
  assert(at->dense_components[0] == 10 && fail);
  assert(at->sparse_components[10] == 0 && fail);
  pass
}
void sim_next_archetype_fetched()
{
  sim s;
  s.atypes[1] = new atype(1);
  assert(s.atypes.size() == 2 && fail);
  atype* at = s.get_next_atype(s.atypes[0], 1, 10, false, 0);
  assert(at->hash == 1 && fail);
  assert(s.atypes.size() == 2 && fail);
  pass
}
void sim_next_archetype_addto_component()
{
  sim s;
  atype* at1 = new atype(1);
  at1->include(0, 8);
  atype* at2 = s.get_next_atype(at1, 2, 1, false, 12);
  assert(at2->hash == 2 && fail);
  assert(at2->dense_components.size() == 2 && at2->dense_components[0] == 0 &&
    at2->dense_components[1] == 1 && fail);
  assert(at2->sparse_components[0] == 0 && at2->sparse_components[1] == 1 && fail);
  assert(at2->clcs.size() == 2 && fail);
  assert(at2->clcs[0]->element_size == 8 && fail);
  assert(at2->clcs[1]->element_size == 12 && fail);
  pass
}
void sim_next_archetype_addto_tag()
{
  sim s;
  atype* at1 = new atype(1);
  at1->include(0, 8);
  atype* at2 = s.get_next_atype(at1, 2, 1, false, 0);
  assert(at2->size == 1 && fail);
  assert(at2->dense_components.size() == 2 && fail);
  assert(at2->dense_components[0] == 0 && fail);
  assert(at2->dense_components[1] == 1 && fail);
  assert(at2->clcs[0]->element_size == 8 && fail);
  pass
}
void sim_next_archetype_remove_tovoid()
{
  sim s;
  atype* at = s.get_next_atype(s.atypes[0], 1, 0, false, 0);
  atype* at2 = s.get_next_atype(at, 0, 0, true, 0);
  assert(at2->hash == 0 && fail);
  assert(at2->dense_components.empty() && fail);
  pass
}
void sim_next_archetype_remove_component()
{
  // don't make sense right now
}
void  sim_next_archetype_remove_tag()
{
  // don't make sense right now
}
void sim_correct_recycle_construct()
{
  sim s;
  assert(s.entities.empty() && fail);
  ecsid entity = s.entity();
  assert(s.entities.size() == 1 && fail);
  assert(ident(entity) == s.entities.size() - 1 && fail);
  assert(version(entity) == 0 && fail);
  assert(s.head == NULL_ID && fail);
  assert(s.avail_count == 0 && fail);
  s.erase(entity);
  assert(s.head == ident(entity) && fail);
  assert(s.avail_count == 1 && fail);
  ecsid id = s.entities[0];
  assert(ident(id) == NULL_ID && fail);
  assert(version(id) == (version(entity)+1) && fail);
  pass
}
void sim_correct_recycle_construct_multiple()
{
  sim s;
  ecsid entity = s.entity();
  ecsid entity2 = s.entity();
  s.erase(entity);
  s.erase(entity2);
  ecsid entity3 = s.entity();
  assert(s.avail_count == 1 && fail);
  assert(s.head == ident(entity) && fail);
  assert(entity3 == s.entities.back() && fail);
  assert(ident(entity3) == ident(entity2) && fail);
  assert(version(entity3) == (version(entity2)+1) && fail);
  pass
}
void sim_recycle()
{
  sim s;
  ecsid entity = s.entity();
  s.erase(entity);
  ecsid entity2 = s.entity();
  assert(s.head == NULL_ID && fail);
  assert(s.avail_count == 0 && fail);
  assert(s.entities[0] == entity2 && fail);
  assert(ident(entity2) == ident(entity) && fail);
  assert(version(entity2) == version(entity)+1 && fail);
  pass
}
void sim_has_true()
{
  sim s;
  ecsid entity = s.entity();
  s.add<tt>(entity);
  assert(s.atypes.contains(tyfo::id<tt>) && fail);
  assert(!s.atypes[s.c_hashes[tyfo::id<tt>]]->entities.empty() && fail);
  assert(s.atypes[s.c_hashes[tyfo::id<tt>]]->entities[0] == entity && fail);
  assert(!s.atypes[s.c_hashes[tyfo::id<tt>]]->dense_components.empty() && fail);
  assert(s.atypes[s.c_hashes[tyfo::id<tt>]]->dense_components[0] == tyfo::id<tt> && fail);
  assert(s.has<tt>(entity) && fail);
  pass
}
void sim_has_false()
{
  sim s;
  ecsid entity = s.entity();
  assert(s.atypes[0]->dense_components.empty() && fail);
  assert(!s.has<tt>(entity) && fail);
  pass
}
void sim_add()
{
  sim s;
  //s.c_hashes.push_back(1);
  ecsid entity = s.entity();
  s.add<tt>(entity);
  assert(s.has<tt>(entity) && fail);
  s.add<tc>(entity, 1);
  assert(s.has<tt>(entity) && fail);
  assert(s.has<tc>(entity) && fail);
  pass
}
void sim_replace()
{
  sim s;
  ecsid entity = s.entity();
  s.add_i<tc>(entity, 0, 1);
  auto tcref = (tc*)s.get(entity, 0);
  assert(tcref->x == 1 && fail);
  s.add_i<tc>(entity, 0, 20);
  auto tcrefrep = (tc*)s.get(entity, 0);
  assert(tcrefrep->x == 20 && fail);
  pass
}
void sim_remove_tag()
{
  sim s;
  ecsid entity = s.entity();
  s.c_hashes.push_back(0b1);
  s.c_hashes.push_back(0b10);
  s.c_hashes.push_back(0b100);
  s.add_i<tt>(entity, 0b0);
  s.add_i<tc>(entity, 0b1, 1);
  s.add_i<c2>(entity, 0b10, 10, 20);
  s.remove<tt>(entity, 0b0);
  assert(s.atypes.size() == 5 && fail);
  assert(s.atypes[0]->entities.empty() && fail);
  assert(s.atypes[0b1]->entities.empty() && s.atypes[0b1]->dense_components.size() == 1 &&
    s.atypes[0b1]->clcs.empty() && fail);
  assert(s.atypes[0b11]->entities.empty() && s.atypes[0b11]->dense_components.size() == 2 &&
    s.atypes[0b11]->clcs.size() == 1 && fail);
  assert(s.atypes[0b111]->entities.empty() && s.atypes[0b111]->dense_components.size() == 3 &&
    s.atypes[0b111]->clcs.size() == 2 && fail);
  assert(s.atypes[0b110]->entities.size() == 1 && s.atypes[0b110]->dense_components.size() == 2 &&
    s.atypes[0b110]->clcs.size() == 2 && fail);
  pass
}
void sim_remove_component()
{
  sim s;
  ecsid entity = s.entity();
  s.c_hashes.push_back(0b1);
  s.c_hashes.push_back(0b10);
  s.c_hashes.push_back(0b100);
  s.add_i<tt>(entity, 0b0);
  s.add_i<tc>(entity, 0b1, 1);
  s.add_i<c2>(entity, 0b10, 10, 20);
  s.remove<tc>(entity, 0b1);
  assert(s.atypes.size() == 5 && fail);
  assert(s.atypes[0]->entities.empty() && fail);
  assert(s.atypes[0]->entities.empty() && fail);
  assert(s.atypes[0b1]->entities.empty() && s.atypes[0b1]->dense_components.size() == 1 &&
    s.atypes[0b1]->clcs.empty() && fail);
  assert(s.atypes[0b11]->entities.empty() && s.atypes[0b11]->dense_components.size() == 2 &&
    s.atypes[0b11]->clcs.size() == 1 && fail);
  assert(s.atypes[0b111]->entities.empty() && s.atypes[0b111]->dense_components.size() == 3 &&
    s.atypes[0b111]->clcs.size() == 2 && fail);
  assert(s.atypes[0b101]->entities.size() == 1 && s.atypes[0b101]->dense_components.size() == 2 &&
    s.atypes[0b101]->clcs.size() == 1 && fail);
  pass
}
void sim_invalid_entity_kappa()
{
  sim s;
  s.entity();
  char i = s.erase(10);
  assert(i == -1 && fail);
  pass
}
void sim_obsolete_entity_lambda()
{
  sim s;
  ecsid entity = s.entity();
  char b = s.erase(entity);
  assert(!b && fail);
  ecsid entityv2 = s.entity();
  char i = s.erase(entity);
  assert(i == -2 && fail);
  pass
}

void find_alpha(sim& s)
{
  auto find = s.query();
  find->all_of<tc>();
  find->update();
  assert(find->caches.size() == 1 && fail);
  assert(find->allof.size() == 1 && fail);
  assert(find->caches[0]->entities.size() == 3 && fail);
  assert(find->caches[0]->dense_components.size() == 1 && fail);
  size_t iterations = 0;
  auto tuple = std::make_tuple(iterations);
  auto lambda = +[](ecsid entity, std::tuple<size_t>& tup, tc* c)
  {
    size_t& it = std::get<0>(tup);
    assert(entity == it && fail);
    assert(c->x == it && fail);
    it++;
  };
  find->each(lambda, tuple);
  assert(std::get<0>(tuple) == 3 && fail);
  pass
}
void find_mu(sim& s)
{
  auto find = s.query();
  find->all_of<tc>();
  find->update();
  assert(find->caches.size() == 1 && fail);
  assert(find->allof.size() == 1 && fail);
  assert(find->caches[0]->entities.size() == 3 && fail);
  assert(find->caches[0]->dense_components.size() == 1 && fail);
  size_t iterations = 0;
  auto tuple = std::make_tuple(iterations);
  auto lambda = +[](const ecsid* entity, size_t size, std::tuple<size_t>& tup, tc* c)
  {
    size_t& it = std::get<0>(tup);
    it+=size;
  };
  find->batch(lambda, tuple);
  assert(std::get<0>(tuple) == 3 && fail);
  pass
}
void find_beta(sim& s)
{
  s.queries.clear();
  auto find = s.query();
  find->all_of<tc>();
  find->update();
  assert(find->caches.size() == 2 && fail);
  size_t iterations = 0;
  auto tuple = std::make_tuple(iterations);
  auto lambda = +[](ecsid entity, std::tuple<size_t>& tup, tc* c)
  {
    size_t& it = std::get<0>(tup);
    assert(c->x == entity && fail);
    it++;
  };
  find->each(lambda, tuple);
  assert(std::get<0>(tuple) == 3 && fail);
  pass
}
void find_gamma(sim& s)
{
  s.queries.clear();
  auto find = s.query();
  find->all_of<tc>();
  find->update();
  assert(find->caches.size() == 3 && fail);
  size_t iterations = 0;
  auto tuple = std::make_tuple(iterations);
  auto lambda = +[](ecsid entity, std::tuple<size_t>& tup, tc* c)
  {
    size_t& it = std::get<0>(tup);
    assert(c->x == entity && fail);
    it++;
  };
  find->each(lambda, tuple);
  assert(std::get<0>(tuple) == 3 && fail);
  pass
}
void find_delta(sim& s)
{
  s.queries.clear();
  auto find = s.query();
  find->all_of<tc, tt>();
  find->update();
  assert(find->caches.size() == 1 && fail);
  auto lambda = +[](ecsid entity, tc* c)
  {
    assert(entity == 1 && fail);
    assert(c->x == 1 && fail);
  };
  find->each(lambda);
  pass
}
void find_epsilon(sim& s)
{
  s.queries.clear();
  auto find = s.query();
  find->all_of<tc, c2>();
  find->update();
  assert(find->caches.size() == 1 && fail);
  auto lambda = +[](ecsid entity, tc* c, c2* c2)
  {
    assert(entity == 2 && fail);
    assert(c->x == 2 && fail);
    assert(c2->x == 20 && c2->y == 20 && fail);
  };
  find->each(lambda);
  pass
}
void find_zeta(sim& s)
{
  s.queries.clear();
  auto find = s.query();
  find->all_of<tc, c2, tt>();
  find->update();
  assert(find->caches.size() == 1 && fail);
  auto lambda = +[](ecsid entity, tc* c, c2* c2)
  {
    assert(entity == 2 && fail);
    assert(c->x == 2 && fail);
    assert(c2->x == 20 && c2->y == 20 && fail);
  };
  find->each(lambda);
  pass
}
void find_eta(sim& s)
{
  s.queries.clear();
  auto find = s.query();
  find->all_of<tc, c2>();
  find->update();
  assert(find->caches.size() == 2 && fail);
  auto lambda = +[](ecsid entity, tc* c, c2* c2)
  {
    assert(entity == 2 && fail);
    assert(c->x == 2 && fail);
    assert(c2->x == 20 && c2->y == 20 && fail);
  };
  find->each(lambda);
  pass
}
void find_theta_failure(sim& s)
{
  s.queries.clear();
  auto find = s.query();
  find->all_of<tc, tt>();
  find->update();
  assert(find->caches.size() == 2 && fail);
  auto lambda = +[](ecsid entity, tc* c, tt* c2)
  {
    assert(false && fail);
  };
  find->each(lambda);
  pass
}
void find_iota_failure(sim& s)
{
  struct failr {};
  s.queries.clear();
  auto find = s.query();
  find->all_of<tc, tt>();
  find->update();
  assert(find->caches.size() == 2 && fail);
  auto lambda = +[](ecsid entity, tc* c, failr* c2)
  {
    assert(false && fail);
  };
  find->each(lambda);
  pass
}

void test_suite()
{
  cc_swap();
  cc_resize();
  cc_copy();
  atype_sparse_set();
  atype_sparse_set_component_first_include();
  atype_sparse_set_only_component_include();
  atype_evict();
  atype_ereloc_empty();
  atype_ereloc_in_use();
  sim_next_archetype_created();
  sim_next_archetype_fetched();
  sim_next_archetype_addto_component();
  sim_next_archetype_addto_tag();
  sim_next_archetype_remove_tovoid();
  sim_correct_recycle_construct();
  sim_correct_recycle_construct_multiple();
  sim_recycle();
  sim_has_true();
  sim_has_false();
  sim_add();
  sim_replace();
  sim_remove_tag();
  sim_remove_component();
  sim_invalid_entity_kappa();
  sim_obsolete_entity_lambda();

  sim s;
  // add misc from prev. tests
  s.c_hashes.push_back(sim::create_hash());
  s.c_hashes.push_back(sim::create_hash());
  s.c_hashes.push_back(sim::create_hash());
  ecsid entity0 = s.entity();
  ecsid entity1 = s.entity();
  ecsid entity2 = s.entity();
  s.add<tc>(entity0, 0);
  s.add<tc>(entity1, 1);
  s.add<tc>(entity2, 2);
  find_alpha(s);
  find_mu(s);
  s.add<tt>(entity1);
  find_beta(s);
  s.add<c2>(entity2, 20, 20);
  find_gamma(s);
  find_delta(s);
  find_epsilon(s);
  s.add<tt>(entity2);
  find_zeta(s);
  s.remove<tt>(entity2);
  find_eta(s);
  find_theta_failure(s);
  find_iota_failure(s);
}
