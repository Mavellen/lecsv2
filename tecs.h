#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <cstdint>
#include <functional>
#include <algorithm>
#include <unordered_set>

namespace ls::lecs
{
#define V3

#ifndef INIT_CAP_ENTITIES
#define INIT_CAP_ENTITIES 100
#endif
  using ecsid = uint32_t;
  using hash = uint64_t;
  using tupid = uint64_t;

  using family = uint8_t;

  constexpr size_t INIT_CAP = 10;
  constexpr hash VOID_HASH = 0;
  constexpr size_t HASH_SIZE = sizeof(hash) * 8;
  constexpr size_t COMPONENT_SIZE = sizeof(ecsid) * 8;
  constexpr size_t ECSID_SIZE = sizeof(ecsid) * 8;
  constexpr char RESERVED = 3;
  constexpr size_t VERSION_BITS = 6;

#define _set(T, id) _id_<T>::_id = id
#define _get(T) _id_<T>::_id
#define _set_tuple(T, K, id) _pair_<T, K>::_id = id; _pair_<T, K>::_size = sizeof(T)
#define _get_tuple_id(T, K) _pair_<T, K>::_id
#define _get_tuple_size(T, K) _pair_<T, K>::_size

#define _tag ((ecsid)1 << COMPONENT_SIZE-1)
#define _pair ((ecsid)1 << COMPONENT_SIZE-2)
#define _tuple ((ecsid)1 << COMPONENT_SIZE-3)

#define _get_version(id) ((id >> (ECSID_SIZE - VERSION_BITS)) << (ECSID_SIZE - VERSION_BITS))
#define _get_id(id) ((id << VERSION_BITS) >> VERSION_BITS)
#define _set_version(id, version) ((((id >> ECSID_SIZE - VERSION_BITS) | version) << ECSID_SIZE - VERSION_BITS) | id)

#define _left(tupid) (ecsid)tupid
#define _right(tupid) (ecsid)(id >> COMPONENT_SIZE/2)
#define _combine(left, right) (((tupid)left)<<(COMPONENT_SIZE/2) | right)

  constexpr bool is_tag(const ecsid id){ return id >> (COMPONENT_SIZE-1); }
  constexpr bool is_pair(const tupid id){ return (id << 1) >> (COMPONENT_SIZE-1); }
  constexpr bool is_tuple(const ecsid id){ return (id << 2) >> (COMPONENT_SIZE-1); }
  constexpr ecsid real_id(const ecsid id){ return (id << RESERVED) >> (RESERVED); }
  constexpr void* imalloc(const size_t size){ return malloc(size); }

  struct query;
  struct _exclusive_has_;
  struct _fetch_;

  template<class T>
  struct _id_
  {
    static ecsid _id;
  };
  template<typename T> ecsid _id_<T>::_id = 0;

  template<class T, class K>
  struct _pair_
  {
    static ecsid _id;
    static size_t _size;
  };
  template<class T, class K> ecsid _pair_<T,K>::_id = 0;
  template<class T, class K> size_t _pair_<T,K>::_size = 0;

  template<class T, class K>
  struct tuple
  {
    typedef T _left;
    typedef K _right;
    T* data;
  };

  template<class T>
  struct _relation_
  {
    static ecsid _id;
    static ecsid _inverse;
  };
  template<typename T> ecsid _relation_<T>::_id = 0;
  template<typename T> ecsid _relation_<T>::_inverse = 0;

  struct column
  {
    void* at(size_t row) const;
    template<typename T, typename... Args>
    void aplace(size_t row, Args&&... args);
    void rswap(size_t row);
    void copy_element(column* other, size_t frow, size_t trow);
  private:
    static void mset(void* from, void* to, size_t fidx, size_t tidx, size_t length);
    void resize();
  public:
    size_t capacity, count;
    size_t element_size;
    void* elements;
  };

  struct group
  {
    group()
    {
      entities.reserve(INIT_CAP_ENTITIES);
    };
    ecsid evict_entity(size_t row);
    std::pair<size_t, ecsid> ereloc(group* to, size_t frow, ecsid excludee = 0);

    hash group_hash {0};
    int size = 0;
    bool is_finalized = false;
    std::vector<column*> columns {};
    std::vector<ecsid> entities {};
    std::vector<ecsid> components {};
  };

  struct broker
  {
    ecsid inverse;
    std::vector<ecsid> entities {};
    std::unordered_map<ecsid, std::vector<ecsid>> relatives {};
  };

  struct world
  {
  private:
    struct eid_record
    {
      group* group;
      size_t row;
    };
  private:
    static hash create_hash();
    static hash thash(hash h1, hash h2);
    template<typename T>
    ecsid register_id_as_tag();
    template<typename T, typename K>
    ecsid register_as_tuple();
    template<typename T>
    ecsid register_as_component();
    [[nodiscard]]
    group* get_next_group(const group* current, hash nhash, ecsid nid, bool remove, size_t size);
    void finalize_group(group* group);
    void register_group(group* g);
  public:
    world();

    family new_family();

    template<typename T>
    void include_in_family(family family);
    template<typename T, typename K>
    void include_in_family(family family);



    // add a Tag w/o data, where T's id is the identifier
    template<typename T>
    void add(ecsid entity);
    // add a (T,K) pair w/o data, where T's id is the identifier
    template<typename T, typename K>
    void add(ecsid entity);
    // Add a (T,K) pair with data, where T's id is the identifier
    template<typename T, typename K, typename... Args>
    void add(ecsid entity, Args&&... args);
    // Add a component with data, where T's id is the identifier
    template<typename T, typename... Args>
    void add(ecsid entity, Args&&... args);
    // Add a (T,E) pair w/o data, where T's id is the identifier
    template<typename T>
    void add(ecsid entity, ecsid other);
    // maybe add void add(eid entity, eid other, Args&&... args) for (T,E) with data

    template<typename T, typename K>
    void remove(ecsid entity);
    template<typename T>
    void remove(ecsid entity);
    template<typename T>
    void remove(ecsid entity, ecsid other);

    template<typename T, typename K>
    bool has(ecsid entity);
    template<typename T>
    bool has(ecsid entity);

    template<typename T, typename K>
    T* get(ecsid entity);
    template<typename T>
    T* get(ecsid entity);

    ecsid entity();
    void erase(ecsid entity);

    void register_query(query* q);

  private:
    template<typename T>
    void add_relation(ecsid entity, ecsid other)
    {
      const ecsid id = _relation_<T>::_id;
      const ecsid inv = _relation_<T>::_inverse;
      _entity_relations[entity].insert(id);
      _entity_relations[other].insert(inv);

      _relations[id].relatives[entity].push_back(other);
      _relations[id].entities.push_back(entity);

      _relations[inv].relatives[other].push_back(entity);
      _relations[inv].entities.push_back(other);
    }
    template<typename T>
    void remove_relation(ecsid entity, ecsid other)
    {
      remove_relation(entity, other, _relation_<T>::_id);
    }
    void remove_relation(ecsid entity, ecsid other, ecsid id);
    void clear_relation(ecsid entity, ecsid relation);
    template<typename T>
    const std::vector<ecsid>& get_relatives(ecsid entity)
    {
      return _relations[_relation_<T>::_id].relatives[entity];
    }
    template<typename T>
    bool has_relation(ecsid entity)
    {
      return _entity_relations[entity].contains(_relation_<T>::_id);
    }

  private:
    static constexpr std::hash<std::string> hasher {};

    std::vector<ecsid> _entities {};
    ecsid _head;
    size_t _count = 0;

    //ecsid entity_counter = 1;
    //std::queue<ecsid> _open_indices {};
    family f_counter = 1;

    std::vector<query*> queries {};
    std::unordered_map<hash, group*> groups {};
    std::vector<eid_record> entities {};
    std::vector<std::unordered_set<ecsid>> _families {};
    std::vector<hash> _component_hashes {};
    std::vector<std::unordered_set<hash>> _component_locations {};



    // For these, it doesn't make sense to create a record since entities can
    // have many relations. Just make a lookup
    std::vector<std::unordered_set<ecsid>> _entity_relations {};
    std::unordered_map<ecsid, broker> _relations {};



    friend query;
    friend group;
  };


#ifdef V3
  struct query
  {
  private:
    struct cache
    {
      std::vector<column*> columns {};
      const group* g;
    };
  public:
    template<typename T, typename K, typename... N>
    void has(){ _has.push_back(_id_<T>::_id); has<K, N...>(); }
    template<typename T>
    void has(){ _has.push_back(_id_<T>::_id); }
    template<typename T, typename K>
    void has_tuple(){ _has.push_back(_get_tuple_id(T, K)); }
    template<typename T, typename K, typename... N>
    void has_not(){ _hasnt.push_back(_id_<T>::_id); has_not<K, N...>(); }
    template<typename T>
    void has_not(){ _hasnt.push_back(_id_<T>::_id); }
    template<typename T, typename K>
    void has_not_tuple(){_hasnt.push_back(_get_tuple_id(T, K));}
    template<typename T, typename K, typename... N>
    void exclusive(){ _exclusive_in_family.push_back(_id_<T>::_id); exclusive<K, N...>();}
    template<typename T>
    void exclusive(){ _exclusive_in_family.push_back(_id_<T>::_id);}
    template<typename T, typename K>
    void has_exclusive_tuple(){ _exclusive_in_family.push_back(_get_tuple_id(T, K));}
    template<typename T, typename K, typename... N>
    void fetch(){ _fetch.push_back(_id_<T>::_id); fetch<K, N...>(); }
    template<typename T>
    void fetch(){ _fetch.push_back(_id_<T>::_id); }
    template<typename T, typename K>
    void fetch_tuple(){ _fetch.push_back(_get_tuple_id(T, K));}

    template<typename... T>
    void each(void(*lambda)(ecsid, T*...));
    template<typename... T, typename... K>
    void each(void(*lambda)(ecsid,std::tuple<K...>&,T*...),std::tuple<K...>&);

    template<typename... T>
    void batch_each(void(*lambda)(const ecsid*,size_t,T*...));
    template<typename... T, typename... K>
    void batch_each(void(*lambda)(const ecsid*,size_t,std::tuple<K...>&,T*...),std::tuple<K...>&);

    template<typename... T>
    void anon_each(void(*lambda)(T*...));
    template<typename... T, typename... K>
    void anon_each(void(*lambda)(std::tuple<K...>&, T*...),std::tuple<K...>&);

    template <typename... T>
    void batch_anon(void (*lambda)(size_t, T*...));
    template<typename... T, typename... K>
    void batch_anon(void(*lambda)(size_t,std::tuple<K...>&,T*...),std::tuple<K...>&);

    void inspect_group(world* w, group* g);
    void create_cache(world* w);
  private:
    std::unordered_set<hash> intersect(std::unordered_set<hash>&,std::unordered_set<hash>&);
    void fetch_traverse(group*);
    bool has_traverse(group*);
    bool hasnt_traverse(group*);
    bool exclusive_traverse(world* w, group*);

    template<typename T>
    T* unpack_each(const cache& com, size_t column_index, size_t position);
    template<typename T>
    T* unpack_all(const cache& com, size_t column_index);
  public:
    char _up_to_date = 0;
  private:
    std::vector<cache> _caches {};
    std::vector<ecsid> _has {};
    std::vector<ecsid> _hasnt {};
    std::vector<ecsid> _fetch {};
    std::vector<ecsid> _exclusive_in_family {};;

    friend world;
  };
#endif
}
