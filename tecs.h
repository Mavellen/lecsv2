#pragma once

#include <vector>
#include <unordered_map>
#include <queue>
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

  using eid = uint32_t;
  using cid = uint64_t;
  using hash = uint64_t;
  using family = uint8_t;

  constexpr size_t INIT_CAP = 10;
  constexpr hash VOID_HASH = 0;
  constexpr size_t HASH_SIZE = sizeof(hash);
  constexpr size_t COMPONENT_SIZE = sizeof(cid);

#define to_tag (COMPONENT_SIZE-1)
#define to_tt_relation (COMPONENT_SIZE-2)
#define to_te_relation (COMPONENT_SIZE-3)
#define to_tWv_relation (COMPONENT_SIZE-4)

  struct query;
  struct _exclusive_has_;
  struct _fetch_;

  template<class T>
  struct _component_
  {
    static constexpr cid _id {};
  };

#define new_component(type, id)\
static_assert(id > 0 && "ID 0 is reserved!");\
template<> struct ls::lecs::_component_<type>{\
static constexpr cid _id {id};\
};\

#define id_from_type(type) _component_<type>::_id

#define new_tag(type, id)\
static_assert(id > 0 && "ID 0 is reserved!");\
template<> struct ls::lecs::_component_<type>{\
static constexpr cid _id {(cid)id | ((cid)1<<to_tag)};\
};\

  template<typename T, typename K>
  struct _relation_
  {
    static cid _id;
  };
  template<typename T, typename K> cid _relation_<T,K>::_id = 0;

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
      size = 0;
    };
    group(group& g, hash h, bool is_tag, cid exclude = 0);
    group(const group& g, hash h, bool is_tag, cid exclude = 0);
    eid evict_entity(size_t row);
    std::pair<size_t, eid> ereloc(group* to, size_t frow, eid excludee = 0);

    hash group_hash {0};
    int size = 0;
    std::vector<column*> columns {};
    std::vector<eid> entities {};
    std::vector<cid> components {};
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
    static bool is_tag(const cid id){ return (id >> to_tag); }
    static bool is_ttr(const cid id){ return (id >> to_tt_relation); }
    static bool is_ter(const cid id){ return (id >> to_te_relation); }
    static bool is_tevr(const cid id){ return (id >> to_tWv_relation); }
    static cid to_realid_tag(const cid id){return id ^ ((cid)1<<to_tag);}

    static hash create_hash();
    static hash thash(hash h1, hash h2);
  public:
    explicit world(size_t ct_amount);
    eid new_entity();
    cid new_relation();
    family new_family();
    void destroy_entity(eid entity);
    template<typename T>
    void include_in_family(family f);
    template<typename T>
    bool has(eid entity);
    template<typename T>
    T* get(eid entity);
    template <typename T, typename... Args>
    void add(eid entity, Args&&... args);
    template<typename T>
    void tag(eid entity);
    template<typename T, typename K>
    void relate(eid entity);
    template<typename T, typename K, typename N, typename... Args>
    void relate(eid entity, Args&&... args);
    template<typename T>
    void remove_tag(eid entity);
    template<typename T>
    void remove(eid entity);
    void register_query(query* q);
  private:
    void register_group(group* g);
    [[nodiscard]] group* create_group(const group* og, hash gh, bool is_tag, cid excludee = 0);

  private:
    eid e_counter = 1;
    family f_counter = 1;
    std::queue<eid> open_indices {};

    static constexpr std::hash<std::string> hasher {};

    std::vector<query*> queries {};
    std::unordered_map<hash, group*> groups {};
    std::vector<eid_record> entities {};
    std::vector<std::vector<hash>> _component_groups_vec {};
    std::vector<std::unordered_set<cid>> _families {};
    std::vector<hash> _component_hashes {};
    std::vector<hash> _relations {};
    // entity-entity relations dont need to be in archetypes
    // not only for the fact that this would lead to a doubling of archetypes
    // because every archetype can be with or without a relation of such an id
    // also its just entity-entity, there is not real values involved
    //std::unordered_map<eid, std::vector<ee_relation>> _entity_entity_relations {};

    friend query;
    friend group;
#ifdef V2
    friend _exclusive_has_;
    friend _fetch_;
#endif
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
    void has(){ _has.push_back(id_from_type(T)); has<K, N...>(); }
    template<typename T>
    void has(){ _has.push_back(id_from_type(T)); }
    template<typename T, typename K, typename... N>
    void has_not(){ _hasnt.push_back(id_from_type(T)); has_not<K, N...>(); }
    template<typename T>
    void has_not(){ _hasnt.push_back(id_from_type(T)); }
    template<typename T, typename K, typename... N>
    void exclusive(){ _exclusive_in_family.push_back(id_from_type(T)); exclusive<K, N...>();}
    template<typename T>
    void exclusive(){ _exclusive_in_family.push_back(id_from_type(T));}
    template<typename T, typename K, typename... N>
    void fetch(){ _fetch.push_back(id_from_type(T)); fetch<K, N...>(); }
    template<typename T>
    void fetch(){ _fetch.push_back(id_from_type(T)); }

    template<typename... T>
    void each(void(*lambda)(eid, T*...));
    template<typename... T, typename... K>
    void each(void(*lambda)(eid,std::tuple<K...>&,T*...),std::tuple<K...>&);

    template<typename... T>
    void batch_each(void(*lambda)(const eid*,size_t,T*...));
    template<typename... T, typename... K>
    void batch_each(void(*lambda)(const eid*,size_t,std::tuple<K...>&,T*...),std::tuple<K...>&);

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
    std::vector<cid> _has {};
    std::vector<cid> _hasnt {};
    std::vector<cid> _fetch {};
    std::vector<cid> _exclusive_in_family {};;

    friend world;
  };
#endif

#ifdef V2
  struct _has_
  {
    _has_() : lambda([](group*,int& i){i = 1;}){}
    template<typename... T>
    void construct();
    void execute(group* g, int& i) const { lambda(g, i); }
  private:
    template<typename T>
    void traverse(group* g, int& i, size_t& amount)
    {
      ++amount;
      for(auto c : g->components)
      {
        if(component<T>::_id == c) i++;
      }
    }
  private:
    std::function<void(group*,int&)> lambda;
  };

  struct _has_not_
  {
    _has_not_() : lambda([](group*,int& i){i = 1;}) {}
    template<typename... T>
    void construct();
    void execute(group* g, int& i) const { return lambda(g, i); }
  private:
    template<typename T>
    void traverse(cid c, int& i)
    {
      if(component<T>::_id == c)
        --i;
    }
  private:
    std::function<void(group*,int&)> lambda;
  };

  struct _exclusive_has_
  {
    _exclusive_has_() : lambda([](world* w, group*,int& i){i = 1;}){}
    template<typename... T>
    void construct();
    void execute(world* w, group* g, int& i) const { lambda(w, g, i); }
  private:
    template<typename T>
    void traverse(world* w, group* g, int& i);
  private:
    std::function<void(world* w, group*,int&)> lambda;
  };

  struct _fetch_
  {
    _fetch_() : lambda([](world*,query*,group*,hash,size_t&){}) {};
    template<typename... T>
    void construct();
    void execute(world* w, query* q, group* g, const hash gh, size_t& index) const {lambda(w,q,g,gh,index);}
  private:
    template<typename T>
    void count()
    {
      ++amount;
    }
    template<typename T, typename... K>
    void count()
    {
      ++amount;
      count<K...>();
    }
    // void t_traverse(world* w, query* q, group* g, hash gh, size_t& index, bool& n){};
    // template<typename... T>
    // void t_traverse(world* w, query* q, group* g, hash gh, size_t& index, bool& n);
    template<typename T, typename K, typename... N>
    void t_traverse(world* w, query* q, group* g, hash gh, size_t& index, size_t& n);
    // template<typename T>
    // void t_traverse(world* w, query* q, group* g, hash gh, size_t& index, bool& n);
    template<typename T>
    void t_traverse(world* w, query* q, group* g, hash gh, size_t& index, size_t& n);
  private:
    size_t amount = 0;
    std::function<void(world*,query*,group*,hash,size_t&)> lambda;
  };


  struct query
  {
  private:
    struct ccom
    {
      std::vector<const column*> columns;
      const group* g;
    };
  public:
    template<typename... T>
    void has(){ _has.construct<T...>(); }
    template<typename... T>
    void has_not(){ _has_not.construct<T...>(); }
    template<typename... T>
    void exclusive(){ _exclusive_has.construct<T...>(); }
    template<typename... T>
    void fetch(){ _fetch.construct<T...>(); }

    template<typename... T>
    void each(void(*lambda)(eid, T*...));
    template<typename... T, typename... K>
    void each(void(*lambda)(eid,std::tuple<K...>&,T*...),std::tuple<K...>&);

    template<typename... T>
    void batch_each(void(*lambda)(const eid*,size_t,T*...));
    template<typename... T, typename... K>
    void batch_each(void(*lambda)(const eid*,size_t,std::tuple<K...>&,T*...),std::tuple<K...>&);

    template<typename... T>
    void anon_each(void(*lambda)(T*...));
    template<typename... T, typename... K>
    void anon_each(void(*lambda)(std::tuple<K...>&, T*...),std::tuple<K...>&);

    template <typename... T>
    void batch_anon(void (*lambda)(size_t, T*...));
    template<typename... T, typename... K>
    void batch_anon(void(*lambda)(size_t,std::tuple<K...>&,T*...),std::tuple<K...>&);

    void inspect_group(world* w, group* g, hash gh, size_t& index);
  private:
    template<typename T>
    T* exec(const ccom& com, size_t column_index, size_t position);
    template<typename T>
    T* all_exec(const ccom& com, size_t column_index);
    std::vector<ccom> _ccoms {};
  private:
    _has_ _has {};
    _has_not_ _has_not {};
    _exclusive_has_ _exclusive_has {};
    _fetch_ _fetch {};

    friend _fetch_;
    friend world;
  };
#endif
}
