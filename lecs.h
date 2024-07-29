#pragma once
#include <cassert>
#include <cstdint>
#include <cstring>
#include <memory>
#include <utility>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <random>

namespace ls::lecs
{
  constexpr size_t INIT_SIZE = 10;
  constexpr size_t E_INIT = 10;

  using ecsid = uint32_t;
  constexpr size_t C_SIZE = sizeof(ecsid) * 8;
  constexpr size_t VERSION_SIZE = 6;
  constexpr ecsid NULL_ID = UINT32_MAX >> VERSION_SIZE;

  using hash = uint64_t;
  constexpr size_t HASH_SIZE = sizeof(hash) * 8;

#define ident(id) (id & ~(0b111111 << (C_SIZE - VERSION_SIZE)))
#define version(id) (id >> (C_SIZE - VERSION_SIZE))
#define set_v(id, v) ((v << (C_SIZE - VERSION_SIZE)) | id)

  struct find;

  struct tyfo
  {
    inline static ecsid counter {};
    template<typename T, typename... K>
    inline static const ecsid id = counter++;
  };

  struct cc
  {
    ~cc(){ free(elements); }
    [[nodiscard]] void* at(const size_t row) const
    {
      return (elements + (element_size * row));
    }
    template<typename T, typename... Args>
    void aplace(const size_t row, Args&&... args)
    {
      if (row >= capacity) resize();
      if (row >= count)
      {
        count++;
      }
      const size_t offset = row * element_size;
      ::new(elements + offset) T(std::forward<Args>(args)...);
    }
    void rswap(const size_t row)
    {
      const size_t foff = element_size * (count - 1);
      const size_t toff = element_size * row;
      mset(elements, elements, foff, toff, element_size);
      count--;
    }
    void copy_element(cc* other, const size_t frow, const size_t trow) const
    {
      if(trow >= other->capacity)
        other->resize();
      mset(
        elements,
        other->elements,
        frow * element_size,
        trow * element_size,
        element_size
      );
      other->count++;
    }
  private:
    static void mset(void* from, void* to, const size_t fidx, const size_t tidx, const size_t length)
    {
      memcpy(to + tidx, from + fidx, length);
    }
    void resize()
    {
      size_t ccap = capacity;
      capacity <<= 1;
      void* new_array = malloc(element_size * capacity);
      mset(elements, new_array, 0, 0, element_size * ccap);
      free(elements);
      elements = new_array;
    }
  public:
    size_t capacity, count;
    size_t element_size;
    void* elements;
  };

  struct atype
  {
    explicit atype(const hash hash) : hash(hash)
    {
      sparse_components.reserve(tyfo::counter);
      entities.reserve(E_INIT);
    }
    ~atype()
    {
      dense_components.clear();
      sparse_components.clear();
      entities.clear();
      for(auto* cc : clcs)
      {
        delete cc;
      }
      clcs.clear();
    }
    [[nodiscard]] size_t cidx(const ecsid component) const
    {
      return component >= sparse_components.size() ? SIZE_MAX : sparse_components[component];
    }
    void include(const ecsid component, const size_t csize)
    {
      while(component >= sparse_components.size())
      {
        sparse_components.push_back(SIZE_MAX);
      }
      if(sparse_components[component] != SIZE_MAX)
        return;
      if(csize > 1)
      {
        if(dense_components.empty() || dense_components.size() == size)
        {
          dense_components.push_back(component);
          sparse_components[component] = size;
        }
        else
        {
          dense_components.push_back(dense_components[size]);
          sparse_components[dense_components[size]] = dense_components.size()-1;
          dense_components[size] = component;
          sparse_components[component] = size;
        }
        size++;
        clcs.push_back(new cc{ INIT_SIZE, 0, csize, malloc(csize * INIT_SIZE)});
      }
      else
      {
        sparse_components[component] = dense_components.size();
        dense_components.push_back(component);
      }
    }
    ecsid evict_entity(const size_t row)
    {
      const ecsid last_pos = entities.size() - 1;
      const ecsid swapped_eid = entities[last_pos];
      for (const auto column : clcs)
      {
        column->rswap(row);
      }
      entities[row] = swapped_eid;
      entities.pop_back();
      return swapped_eid;
    }
    std::pair<size_t, ecsid> ereloc(atype* to, const size_t frow, const ecsid excludee = NULL_ID)
    {
      const size_t new_row = to->entities.size();
      for(size_t i = 0; i < size; i++)
      {
        if(dense_components[i] == excludee) continue;
        clcs[i]->copy_element(to->clcs[to->cidx(dense_components[i])], frow, new_row);
      }
      to->entities.push_back(entities[frow]);
      const ecsid swapped_eid = evict_entity(frow);
      return {new_row, swapped_eid};
    }
    hash hash;
    size_t size = 0;
    std::vector<ecsid> dense_components {};
    std::vector<size_t> sparse_components;
    std::vector<ecsid> entities {};
    std::vector<cc*> clcs {};
  };

  struct sim
  {
#ifdef TESTS
  public:
#else
  private:
#endif
    struct record{ atype* at; size_t row; };
    enum  E_ASSERT_STATE : char { VALID = 0, INVALID = -1, OBSOLETE = -2 };
    static constexpr std::hash<std::string> hasher {};
    static hash create_hash()
    {
      std::string str(HASH_SIZE, 0);
      std::ranges::generate_n(str.begin(), HASH_SIZE, []()
      {
        static constexpr auto chars =
          "0123456789"
          "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
          "abcdefghijklmnopqrstuvwxyz";
        static constexpr size_t max_index = (strlen(chars) - 1);
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> distributor(0, max_index);
        return chars[distributor(gen)];
      });
      return hasher(str);
    }
    static hash thash(const hash h1, const hash h2)
    {
      return h1 ^ h2;
    }
    std::pair<E_ASSERT_STATE, ecsid> assert_and_get(const ecsid entity) const
    {
      const ecsid ident = ident(entity);
      if(e_locations.size() <= ident) return {E_ASSERT_STATE::INVALID, NULL_ID};
      if(entities[ident] != entity) return {E_ASSERT_STATE::OBSOLETE, NULL_ID};
      return {E_ASSERT_STATE::VALID, ident};
    }
    void register_atype(const atype* atype);
    void update_queries(ecsid component);
    atype* get_next_atype(const atype* current, const hash nhash, const ecsid nid, const bool remove, const size_t size)
    {
      if(atypes.contains(nhash)) [[likely]] return atypes[nhash];
      auto* natype = new atype(nhash);
      //natype->size = size > 1 ? remove ? current->size - 1 : current->size + 1 : current->size;
      natype->dense_components.reserve(remove ? current->size - 1 : current->dense_components.size() + 1);
      for(size_t i = 0; i < current->dense_components.size(); i++)
      {
        const ecsid id = current->dense_components[i];
        if(remove && id == nid) continue;
        if(i < current->size) natype->include(id, current->clcs[i]->element_size);
        else natype->include(id, 0);
        // TODO c_locations[id].insert(natype->hash);
      }
      if(!remove)
      {
        natype->include(nid, size);
        // TODO c_locations[nid].insert(natype->hash);
      }
      atypes[nhash] = natype;
      register_atype(natype);
      return natype;
    }
    bool has(const ecsid entity, const ecsid type_id) const
    {
      const auto pair = assert_and_get(entity);
      if(pair.first)
        return false;
      const record& r = e_locations[pair.second];
      return c_hashes.size() > type_id ? r.at->cidx(type_id) < SIZE_MAX : false;
    }
    void* get(const ecsid entity, const ecsid type_id) const
    {
      const auto pair = assert_and_get(entity);
      if(pair.first)
        return nullptr;
      const record& r = e_locations[pair.second];
      if(c_hashes.size() <= type_id
        || /*!c_locations[id].contains(r.at->hash)*/ r.at->cidx(type_id) == SIZE_MAX)
        return nullptr;
      if(const size_t loc = r.at->cidx(type_id); loc < SIZE_MAX && loc < r.at->size)
        return r.at->clcs[loc]->at(r.row);
      return nullptr;
    }
    template<typename T, typename... Args>
    char add_i(const ecsid entity, const ecsid type_id, Args&&... args)
    {
      const auto pair = assert_and_get(entity);
      if(pair.first)
        return pair.first;
      record& r = e_locations[pair.second];
      if(c_hashes.size() <= type_id)
      {
        c_hashes.push_back(create_hash());
        // TODO c_locations.emplace_back();
      }
      if(/* TODO c_locations[id].contains(r.at->hash)*/ r.at->cidx(type_id) != SIZE_MAX)
      {
        if(sizeof(T) > 1)
        {
          r.at->clcs[r.at->cidx(type_id)]->aplace<T>(r.row, std::forward<Args>(args)...);
        }
        return pair.first;
      }

      const hash chash = c_hashes[type_id];
      const hash nhash = thash(r.at->hash, chash);

      atype* natype = get_next_atype(r.at, nhash, type_id, false, sizeof(T));
      auto [nrow, swent] = r.at->ereloc(natype, r.row);
      if(sizeof(T) > 1)
        natype->clcs[natype->cidx(type_id)]->aplace<T>(nrow, std::forward<Args>(args)...);
      e_locations[ident(swent)].row = r.row;
      r.at = natype;
      r.row = nrow;
      return pair.first;
    }
    template<typename T>
    char remove(const ecsid entity, const ecsid type_id)
    {
      const auto pair = assert_and_get(entity);
      if(pair.first)
        return pair.first;
      record& r = e_locations[pair.second];
      if(c_hashes.size() <= type_id || /* TODO !c_locations[id].contains(r.at->hash)*/ r.at->cidx(type_id) == SIZE_MAX) return 1;

      const hash chash = c_hashes[type_id];
      const hash nhash = thash(r.at->hash, chash);

      atype* natype = get_next_atype(r.at, nhash, type_id, true, sizeof(T));
      auto [nrow, swent] = r.at->ereloc(natype, r.row, type_id);
      e_locations[ident(swent)].row = r.row;
      r.at = natype;
      r.row = nrow;
      return pair.first;
    }
  public:
    sim()
    {
      entities = {};
      e_locations = {};
      c_hashes = {};
      // TODO c_locations = {};
      atypes.insert({0, new atype(0)});
      queries = {};
      head = NULL_ID;
    }
    ~sim(){ reset(); }
    void reset()
    {
      head = NULL_ID;
      avail_count = 0;
      entities.clear();
      e_locations.clear();
      c_hashes.clear();
      families.clear();
      for(auto& pair : atypes)
      {
        delete pair.second;
      }
      atypes.clear();
      queries.clear();
      atypes.insert({0, new atype(0)});
    }
    ecsid entity()
    {
      const auto void_type = atypes[0];
      const size_t row = void_type->entities.size();
      ecsid generated;
      if(avail_count > 0)
      {
        const ecsid version = version(entities[head]);
        generated = set_v(head, version);
        head = ident(entities[head]);
        entities[ident(generated)] = generated;
        --avail_count;
        e_locations[ident(generated)] = {void_type, row};
      }
      else
      {
        generated = entities.size();
        entities.push_back(generated);
        e_locations.push_back({void_type, row});
      }
      void_type->entities.push_back(generated);
      return generated;
    }
    char erase(const ecsid entity)
    {
      const auto pair = assert_and_get(entity);
      if(pair.first)
        return pair.first;
      const record& r = e_locations[pair.second];
      const ecsid swent = r.at->evict_entity(r.row);
      e_locations[swent].row = r.row;

      const ecsid current_version = version(entity);
      const ecsid input_id = set_v(head, current_version+1);
      entities[ident(entity)] = input_id;
      head = ident(entity);
      ++avail_count;
      return pair.first;
    }
    ecsid family()
    {
      families.emplace_back();
      return families.size() - 1;
    }
    template<typename T>
    void include_in_fam(const ecsid family)
    {
      if(families.size() <= family
        || families[family].contains(tyfo::id<T>)) return;
      families[family].insert(tyfo::id<T>);
      update_queries(tyfo::id<T>);
    }
    template<typename T>
    void remove_from_fam(const ecsid family)
    {
      if(families.size() <= family || !families[family].contains(tyfo::id<T>)) return;
      families[family].erase(tyfo::id<T>);
      update_queries(tyfo::id<T>);
    }
    std::shared_ptr<find> query();

    template<typename T>
    bool has(const ecsid entity) const
    {
      return has(entity, tyfo::id<T>);
    }
    template<typename T>
    T* get(const ecsid entity) const
    {
      return (T*) get(entity, tyfo::id<T>);
    }
    template<typename T, typename... Args>
    char add(const ecsid entity, Args&&... args)
    {
      return add_i<T, Args...>(entity, tyfo::id<T>, std::forward<Args>(args)...);
    }
    template<typename T>
    char remove(const ecsid entity)
    {
      return remove<T>(entity, tyfo::id<T>);
    }
#ifdef TESTS
  public:
#else
  private:
#endif
    ecsid head;
    size_t avail_count = 0;
    std::vector<ecsid> entities;
    std::vector<record> e_locations;

    std::vector<hash> c_hashes;
    // TODO std::vector<std::unordered_set<hash>> c_locations;

    std::vector<std::unordered_set<ecsid>> families;

    std::unordered_map<hash, atype*> atypes;

    std::vector<std::shared_ptr<find>> queries;

    friend find;
  };

  struct find
  {
#ifdef TESTS
  public:
#else
  private:
#endif
    void traverse_atypes()
    {
      for(const auto pair : simptr->atypes)
      {
        eval_atype(pair.second);
      }
    }
    template<typename T, typename K, typename... N>
    bool eval_system() const
    {
      if(!allof.contains(tyfo::id<T>) || sizeof(T) <= 1) return false;
      return eval_system<K, N...>();
    }
    template<typename T>
    bool eval_system() const
    {
      return allof.contains(tyfo::id<T>) && sizeof(T) > 1;
    }
    void eval_atype(const atype* at)
    {
      for(const ecsid id : allof)
      {
        if(at->cidx(id) == SIZE_MAX) return;
      }
      for(const ecsid id : noneof)
      {
        if(at->cidx(id) != SIZE_MAX) return;
      }
      if(anyof.empty()) goto forward;
      for(const ecsid id : anyof)
      {
        if(at->cidx(id) != SIZE_MAX)
          goto forward;
      }
      return;
      forward:
      for(const ecsid id : oneof)
      {
        ecsid family = NULL_ID;
        for(size_t i = 0; i < simptr->families.size(); i++)
        {
          if(simptr->families[i].contains(id))
          {
            family = i;
            break;
          }
        }
        if(family == NULL_ID) continue;
        for(const ecsid atype_has : at->dense_components)
        {
          if(atype_has == id) continue;
          if(simptr->families[family].contains(atype_has)) return;
        }
      }
      caches.push_back(at);
    }
    template<typename T>
    T* unpack_each(const atype* at, const ecsid component, const size_t row)
    {
      return (T*) at->clcs[at->cidx(component)]->at(row);
    }
    template<typename T>
    T* unpack_all(const atype* at, const ecsid component)
    {
      return (T*) at->clcs[at->cidx(component)]->elements;
    }
  public:
    explicit find(const sim* ptr) : simptr(ptr) {};

    template<typename T, typename K, typename... N>
    void all_of(){ allof.insert(tyfo::id<T>); all_of<K, N...>(); }
    template<typename T>
    void all_of(){ allof.insert(tyfo::id<T>); }

    template<typename T, typename K, typename... N>
    void none_of(){ noneof.insert(tyfo::id<T>); none_of<K, N...>(); }
    template<typename T>
    void none_of(){ noneof.insert(tyfo::id<T>); }

    template<typename T, typename K, typename... N>
    void any_of(){ anyof.insert(tyfo::id<T>); any_of<K, N...>(); }
    template<typename T>
    void any_of(){ anyof.insert(tyfo::id<T>); }

    template<typename T, typename K, typename... N>
    void one_of_family(){ oneof.insert(tyfo::id<T>); one_of_family<K, N...>(); }
    template<typename T>
    void one_of_family(){ oneof.insert(tyfo::id<T>); }

    void update(){caches.clear(); traverse_atypes();}

    template<typename... T>
    void each(void(*lambda)(ecsid, T*...))
    {
      if(!eval_system<T...>()) return;
      for(const auto& cache : caches)
      {
        for(size_t i = 0; i < cache->entities.size(); i++)
        {
          lambda(cache->entities[i], unpack_each<T>(cache, tyfo::id<T>, i)...);
        }
      }
    }
    template<typename... T, typename... K>
    void each(void(*lambda)(ecsid, std::tuple<K...>&,T*...),std::tuple<K...>& tup)
    {
      if(!eval_system<T...>()) return;
      for(const auto& cache : caches)
      {
        for(size_t i = 0; i < cache->entities.size(); i++)
        {
          lambda(cache->entities[i], tup, unpack_each<T>(cache, tyfo::id<T>, i)...);
        }
      }
    }
    template<typename... T>
    void batch(void(*lambda)(const ecsid*,size_t,T*...))
    {
      if(!eval_system<T...>()) return;
      for(const auto& cache : caches)
      {
        lambda(cache->entities.data(), cache->entities.size(), unpack_all<T>(cache, tyfo::id<T>)...);
      }
    }
    template<typename... T, typename... K>
    void batch(void(*lambda)(const ecsid*,size_t,std::tuple<K...>&,T*...),std::tuple<K...>& tup)
    {
      if(!eval_system<T...>()) return;
      for(const auto& cache : caches)
      {
        lambda(cache->entities.data(), cache->entities.size(), tup, unpack_all<T>(cache, tyfo::id<T>)...);
      }
    }

#ifdef TESTS
  public:
#else
  private:
#endif
    const sim* simptr;
    std::vector<const atype*> caches;
    std::unordered_set<ecsid> allof;
    std::unordered_set<ecsid> anyof;
    std::unordered_set<ecsid> noneof;
    std::unordered_set<ecsid> oneof;

    friend sim;
  };

  inline void sim::register_atype(const atype* atype)
  {
    for(const auto& query : queries)
      query->eval_atype(atype);
  }

  inline std::shared_ptr<find> sim::query()
  {
    queries.push_back(std::make_shared<find>(this));
    return queries.back();
  }

  inline void sim::update_queries(const ecsid component)
  {
    for(const auto& query : queries)
    {
      if(query->oneof.contains(component))
      {
        query->update();
      }
    }
  }

}
