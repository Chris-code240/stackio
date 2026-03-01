#include "../includes/storage_engine.h"

void StorageEngine::set(const std::string &key, const std::string &value){
    std::lock_guard<std::mutex> lock(store_mutex);
    store[key] = value;
    keys.push_back(key);
}

 std::optional<std::string> StorageEngine::get(const std::string & key){
    std::lock_guard<std::mutex> lock(store_mutex);
    auto it = store.find(key);

    if(it != store.end()) return it->second;

    return std::nullopt;
 }

void StorageEngine::pop(const std::string &key){
    std::lock_guard<std::mutex> lock(store_mutex);

    auto it = store.find(key);
    if(it != store.end()) {
        store.erase(key);
        keys.erase(std::find(keys.begin(), keys.end(), key));
    }
    else{
        throw std::runtime_error("Invalid key");
    }
}


json StorageEngine::writeAhead(std::optional<json> data) {
    std::lock_guard<std::mutex> lock(store_mutex);
    if(!data.has_value() || !data.value().is_object()) return json({{"success", false}, {"message", "Data must be object"}});

    int fileDescriptor = _open(_walFilePath.c_str(), _O_WRONLY | _O_CREAT | _O_APPEND, _S_IREAD | _S_IWRITE);
    
    if(fileDescriptor == -1) return json({{"success", false}, {"message", "Error opening"}});

    if(data.has_value()){
        if(_write(fileDescriptor, data.value().dump().c_str(), (unsigned int)data.value().dump().length()) == -1) {
            _close(fileDescriptor);
            return json({{"success", false}, {"message", "Error Writing"}});
        }

        _close(fileDescriptor);
        return json({{"success", true}, {"message", "Successfull"}});
    }


    if(!keys.size()) return {{"success", true}, {"message", "No new Data to Write"}};
    std::string payload = "";
    for( auto k : keys){
        payload += (json({{"key", k}, {"value", store[k]}, {"op", "SET"}}).dump() + "\n");
    }
    keys = {}; // reset keys

    if(_write(fileDescriptor, payload.c_str(), (unsigned int)payload.length()) == -1) {
        _close(fileDescriptor);
        return json({{"success", false}, {"message", "Error Writing"}});
    }

    _close(fileDescriptor);
    return json({{"success", true}, {"message", "Successfull"}});
}