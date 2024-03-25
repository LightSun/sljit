#pragma once

#include "h7/Classer.h"

namespace h7 {

class ObjectScope
{
public:
    ObjectScope(ObjectScope* parent = nullptr): m_parent(parent){}

    ObjectScope* parent(){
        return m_parent;
    }
    void putObject(CString key, ObjectPtr p){
        m_map[key] = p;
    }
    ObjectPtr getFromSelf(CString key){
        auto it = m_map.find(key);
        if(it == m_map.end()){
            return nullptr;
        }
        return it->second;
    }

    ObjectPtr get(CString key){
        ObjectPtr ret = getFromSelf(key);
        if(ret != nullptr) return ret;
        ObjectScope* os = this;
        do{
            os = os->parent();
            if(os == nullptr){
                break;
            }
            ret = os->getFromSelf(key);
        }while(ret == nullptr);
        return ret;
    }
private:
    ObjectScope* m_parent {nullptr};
    HashMap<String, ObjectPtr> m_map;
};

}

