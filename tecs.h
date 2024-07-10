#pragma once

#include <vector>
#include <unordered_map>
#include <queue>
#include <string>
#include <cstdint>
#include <functional>

// namespace ls::lecs
// {
//   using eid = uint64_t;
//   using cid = uint64_t;
//   using family = uint64_t;
//   using hash = uint64_t;
//   using qid = uint64_t;
//
//   constexpr size_t INIT_CAP = 10;
//   constexpr hash VOID_HASH = 0;
//   constexpr size_t HASH_SIZE = 64;
//
//   template<typename T>
//   struct component
//   {
//     static constexpr cid _id {};
//     static constexpr size_t _size {};
//     static family family;
//   };
//
//   struct column
//   {
//     void* at(size_t row) const;
//     template<typename T, typename... Args>
//     void aplace(size_t row, Args&&... args);
//     void rswap(size_t row);
//     void copy_element(column* other, size_t frow, size_t trow);
//   private:
//     static void mset(void* from, void* to, size_t fidx, size_t tidx, size_t length);
//     void resize();
//   public:
//     size_t capacity, count;
//     size_t element_size;
//     cid component_id;
//     void* elements;
//   };
//
//   struct group
//   {
//     group() = default;
//     group(group& g, cid exclude = 0);
//     group(const group& g, cid exclude = 0);
//     eid evict_entity(size_t row);
//     std::pair<size_t, eid> ereloc(group* to, size_t frow, bool excludes = false, eid excludee = 0);
//
//     hash group_hash {0};
//     std::vector<column*> columns {};
//     std::vector<eid> entities {};
//   };
//
//   struct e_query
//   {
//     void* query_ptr;
//   };
//
//   struct world
//   {
//   private:
//     struct gid_record
//     {
//       group* group_ptr;
//       std::vector<cid> components;
//     };
//     struct eid_record
//     {
//       hash group_hash;
//       size_t row;
//     };
//   public:
//     std::vector<e_query> query_objects {};
//     static const qid queries[];
//     static const size_t query_amount;
//   private:
//     static char rchar();
//     static hash thash(hash h1, hash h2);
//   public:
//     world();
//     eid new_entity();
//     void destroy_entity(eid entity);
//     template<typename T>
//     void include_in_family(cid family);
//     template<typename T>
//     bool has(eid entity);
//     template<typename T>
//     T* get(eid entity);
//     template <typename T, typename... Args>
//     void add(eid entity, Args&&... args);
//     template<typename T>
//     void remove(eid entity);
//     template<qid N>
//     void register_group(group* g, hash g_hash);
//     template<qid N, typename... T>
//     void execute(void(*lambda)(eid, T*...));
//     void internal_put_components(size_t i);
//   private:
//     template<typename T>
//     void new_component();
//
//   private:
//     eid e_counter = 0;
//     std::queue<eid> open_indices{};
//
//     static constexpr std::hash<std::string> hasher{};
//
//   public:
//     std::unordered_map<hash, gid_record> groups {};
//     std::vector<hash> component_hashes {};
//     std::vector<eid_record> entities {};
//     std::vector<std::unordered_map<hash, size_t>> cid_groups {};
//     std::unordered_map<cid, std::vector<cid>> families{};
//     std::vector<cid> component_family{};
//   };
//
//   struct ccom
//   {
//     std::vector<const column*> columns;
//     const group* g;
//   };
//
//   template<qid N>
//   struct query
//   {
//     template<typename... T>
//     void each(void(*lambda)(eid, T*...))
//     {
//       for(auto& ccom : _ccoms)
//       {
//         for(size_t i = 0; i < ccom.g->entities.size(); i++)
//         {
//           size_t k = 0;
//           while(k < ccom.columns.size())
//           {
//             lambda(
//               ccom.g->entities[i],
//               exec<T>(ccom, (k++), i)...
//               );
//           }
//         }
//       }
//     }
//     template <typename T>
//     static T* exec(const ccom& com, const size_t column_index, const size_t position)
//     {
//       return (T*)com.columns[column_index]->at(position);
//     }
//     static void check_group(world* w, group* g, const hash gh, size_t& index)
//     {
//       const auto& [group_ptr, cid_vec] = w->groups[gh];
//       for(auto c : cid_vec)
//       {
//         for(size_t i = 0; i < has_not_amount; i++)
//         {
//           if(has_not[i] == c) return;
//         }
//       }
//
//       for(size_t i = 0; i < has_amount; i++)
//       {
//         bool exists = false;
//         for(auto c : cid_vec)
//         {
//           if(c == has[i])
//           {
//             exists = true;
//             break;
//           }
//         }
//         if(!exists) return;
//       }
//
//       // TODO exclusivity
//
//       bool n = false;
//       for(size_t i = fetch_amount; i != SIZE_MAX; i--)
//       {
//         const cid query_for = fetch[i];
//         for(size_t k = 0; k < cid_vec.size(); k++)
//         {
//           if(cid_vec[k] == query_for)
//           {
//             if(_ccoms.size() <= index)
//             {
//               _ccoms.push_back({{}, nullptr});
//               _ccoms[index].columns.push_back(g->columns[w->cid_groups.at(query_for).at(gh)]);
//               _ccoms[index].g = g;
//             }
//             else
//             {
//               _ccoms[index].columns.push_back(g->columns[w->cid_groups.at(query_for).at(gh)]);
//             }
//             n = true;
//             break;
//           }
//         }
//       }
//       if(n) index++;
//     }
//
//   public:
//     static std::vector<ccom> _ccoms;
//     static const cid has[];
//     static const cid has_not[];
//     static const cid fetch[];
//     static const size_t has_amount;
//     static const size_t has_not_amount;
//     static const size_t fetch_amount;
//   private:
//     friend world;
//   };
//
//
//
// }

namespace ls::lecs
{
  using eid = uint64_t;
  using cid = uint64_t;
  using hash = uint64_t;
  using family = uint64_t;
  constexpr size_t INIT_CAP = 10;
  constexpr hash VOID_HASH = 0;
  constexpr size_t HASH_SIZE = 64;

  struct query;

  template<cid C>
  struct _hash_
  {
    static hash _hash;
  };
  template<cid C> hash _hash_<C>::_hash;

  template<cid C>
  struct _family_
  {
    static family _family;
  };
  template<cid C> family _family_<C>::_family;

  template<typename T>
  struct component
  {
    static constexpr cid _id {0};
    static constexpr size_t _size {0};
  };
#define register_component(type, id)\
  template<> struct ls::lecs::component<type>{\
  static constexpr cid _id {id};\
  static constexpr size_t _size {sizeof(type)};\
  };


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
    cid component_id;
    void* elements;
  };

  struct group
  {
    group() = default;
    group(group& g, cid exclude = 0);
    group(const group& g, cid exclude = 0);
    eid evict_entity(size_t row);
    std::pair<size_t, eid> ereloc(group* to, size_t frow, bool excludes = false, eid excludee = 0);

    hash group_hash {0};
    std::vector<column*> columns {};
    std::vector<eid> entities {};
    std::vector<cid> components {};
  };

  struct world
  {
  private:
    struct gid_record
    {
      group* group_ptr;
    };
    struct eid_record
    {
      hash group_hash;
      size_t row;
    };
  private:
    static char rchar();
    static hash thash(hash h1, hash h2);
  public:
    world();
    eid new_entity();
    family new_family();
    void destroy_entity(eid entity);
    template<typename T>
    int include_in_family(family f);
    template<typename T>
    bool has(eid entity);
    template<typename T>
    T* get(eid entity);
    template <typename T, typename... Args>
    void add(eid entity, Args&&... args);
    template<typename T>
    void remove(eid entity);
    void register_query(query* q);
  private:
    void register_group(group* g, hash gh);
    template<typename T>
    void w_register_component();

  private:
    eid e_counter = 0;
    family f_counter = 0;
    std::queue<eid> open_indices {};

    static constexpr std::hash<std::string> hasher {};

    std::vector<query*> queries {};
    std::unordered_map<hash, gid_record> groups {};
    std::vector<eid_record> entities {};
  public:
    std::unordered_map<cid, std::unordered_map<hash, size_t>> cid_groups {};
  private:
    std::vector<std::vector<cid>> families {};
  };


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
    //void(*lambda)(group*,int&);
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
    //void(*lambda)(group*,int&);
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

    template<typename T>
    void t_traverse(world* w, query* q, group* g, hash gh, size_t& index, bool& n);
    // template<typename T>
    // void traverse(world* w, query* q, group* g, hash gh, size_t& index, bool& n);
    // template<typename T, typename... K>
    // void traverse(world* w, query* q, group* g, hash gh, size_t& index, bool& n);
  private:
    size_t amount = 0;
    std::function<void(world*,query*,group*,hash,size_t&)> lambda;
    //void(*lambda)(world*,query*,group*,hash,size_t&);
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
    void fetch(){ _fetch.construct<T...>(); }

    template<typename... T>
    void each(void(*lambda)(eid, T*...));
    template<typename... T>
    void batch_each(void(*lambda)(eid*,size_t,T*...));
    template<typename... T>
    void anon_each(void(*lambda)(T*...));
    template<typename... T>
    void anon_batch(void(*lambda)(size_t,T*...));
    void inspect_group(world* w, group* g, hash gh, size_t& index);
  private:
    template<typename T>
    T* exec(const ccom& com, size_t column_index, size_t position);
    template<typename T>
    T* all_exec(const ccom& com, const size_t column_index);
  public:
    std::vector<ccom> _ccoms {};
  private:
    _has_ _has {};
    _has_not_ _has_not {};
    _fetch_ _fetch {};
  };
}