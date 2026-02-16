#include "../includes/storage_engine.h"

void StorageEngine::set(const std::string &key, const std::string &value){
    store[key] = value;
}

 std::optional<std::string> StorageEngine::get(const std::string & key){
    return store.at(key);
 }