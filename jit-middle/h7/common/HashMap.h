#ifndef HASHMAP_H
#define HASHMAP_H

#include <unordered_map>
#include <map>
#include <functional>
#include <sstream>

#include "h7/common/common.h"
#include "h7/common/SkRefCnt.h"
#include "h7/utils/convert.hpp"
#include "h7/utils/string_utils.hpp"

namespace h7 {


template <typename K, typename V>
class HashMap: public SkRefCnt{
public:
    std::unordered_map<K,V> map;

    HashMap(){}
    HashMap(const std::unordered_map<K,V>& map):map(map){}
    HashMap(const HashMap<K,V>& map):map(map.map){}

    inline V put(const K& key, const V& v){
        V oldv = map[key];
        map[key] = v;
        return oldv;
    }
    inline V& get(const K& key){
        return map[key];
    }
    inline const V get(const K& key)const{
        return map[key];
    }
    inline void put0(const K& key, const V& v){
        map[key] = v;
    }
    inline int size()const{
        return map.size();
    }
    inline std::pair<K,V> at(int index){
        auto it = map.begin() + index;
        return {it.first, it.second};
    }
    inline void remove(const K& key){
        map.erase(key);
    }
    inline void clear(){
        map.clear();
    }
    inline bool hasKey(const K& key){
        return map.find(key) != map.end();
    }
    V& operator[](const K& key){
       return map[key];
    }
    String toString(bool oneLine=true){
        std::stringstream out;
        if(oneLine){
            out << "{";
            auto end = map.end();
            for(auto it = map.begin() ; it != end ; it++){
                out << toStringImpl<K>(it->first, "") << "=" << toStringImpl<V>(it->second, "");
                if(std::next(it) != end){
                    out << ", ";
                }
            }
            out << "}";
        }else{
            out << "{" << h7::utils::newLineStr();
            auto end = map.end();
            for(auto it = map.begin() ; it != end ; it++){
                out << toStringImpl<K>(it->first, "") << "=" << toStringImpl<V>(it->second, "");
                if(std::next(it) != end){
                    out << h7::utils::newLineStr();;
                }
            }
            out << h7::utils::newLineStr() << "}";
        }
        return out.str();
    }
};

template <typename K, typename V>
class TreeMap : public SkRefCnt{
public:
    std::map<K,V> map;

    TreeMap(){}
    TreeMap(const std::map<K,V>& map):map(map){}
    TreeMap(const TreeMap<K,V>& map):map(map.map){}

    inline V put(const K& key, const V& v)const{
        V oldv = map[key];
        map[key] = v;
        return oldv;
    }
    inline V& get(const K& key){
        return map[key];
    }
    inline const V get(const K& key)const{
        return map[key];
    }
    inline void put0(const K& key, const V& v){
        map[key] = v;
    }
    inline int size()const{
        return map.size();
    }
    inline std::pair<K,V> at(int index){
        auto it = map.begin() + index;
        return {it.first, it.second};
    }

    inline void clear(){
        map.clear();
    }
    inline bool hasKey(const K& key){
        return map.find(key) != map.end();
    }
    V& operator[](const K& key){
       return map[key];
    }
    String toString(bool oneLine=true){
        std::stringstream out;
        if(oneLine){
            out << "{";
            auto end = map.end();
            for(auto it = map.begin() ; it != end ; it++){
                out << toStringImpl<K>(it->first, "") << "="
                    << toStringImpl<V>(it->second, "");
                if(std::next(it) != end){
                    out << ", ";
                }
            }
            out << "}";
        }else{
            out << "{" << h7::utils::newLineStr();
            auto end = map.end();
            for(auto it = map.begin() ; it != end ; it++){
                out << toStringImpl<K>(it->first, "") << "="
                    << toStringImpl<V>(it->second, "");
                if(std::next(it) != end){
                    out << h7::utils::newLineStr();
                }
            }
            out << h7::utils::newLineStr() << "}";
        }
        return out.str();
    }
};

}

#endif // HASHMAP_H
