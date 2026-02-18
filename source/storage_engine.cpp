#include "../includes/storage_engine.h"

void StorageEngine::set(const std::string &key, const std::string &value){
    std::lock_guard<std::mutex> lock(store_mutex);
    store[key] = value;
}

 std::optional<std::string> StorageEngine::get(const std::string & key){
    std::lock_guard<std::mutex> lock(store_mutex);
    auto it = store.find(key);

    if(it != store.end())
    return it->second;

    return std::nullopt;
 }