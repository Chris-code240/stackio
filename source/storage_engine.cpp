#include "../includes/storage_engine.h"

void StorageEngine::set(const std::string &key, const std::string &value){
    std::lock_guard<std::mutex> lock(store_mutex);
    auto it = store.find(key);
    if(it != store.end() && it->second != value){
        std::cout<<"Key exits but value has changed from "<<it->second<<" to "<<value<<"\n";
         keys.push_back(key);
         std::set<std::string> s(keys.begin(),keys.end());
         keys = std::vector<std::string>(s.begin(), s.end());
    }else if(it == store.end()){
        std::cout<<"Key doesnt exit..appending..\n";
        for(auto [k,v]: store)std::cout<<"key: "<<k<<" - "<<v<<" ";
        std::cout<<"\n";
        keys.push_back(key);
        std::set<std::string> s(keys.begin(),keys.end());
        keys = std::vector<std::string>(s.begin(), s.end());
    }
    
    store[key] = value;
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
        deleted.push_back(it->first);
        std::set<std::string> s(deleted.begin(), deleted.end());
        deleted = std::vector<std::string>(s.begin(), s.end());
        
        store.erase(key);
        auto it2 = std::find(keys.begin(), keys.end(), key);
        if(it2 != keys.end())keys.erase(it2);

    }
    else{
        throw std::runtime_error("Invalid key");
    }
}


json StorageEngine::writeAhead(std::optional<json> data) {
    std::lock_guard<std::mutex> lock(store_mutex);
    if(data.has_value() && !data.value().is_object()) return json({{"success", false}, {"message", "Data must be object"}});

    int fileDescriptor = _open(_walFilePath.c_str(), _O_WRONLY | _O_CREAT | _O_APPEND, _S_IREAD | _S_IWRITE);
    
    if(fileDescriptor == -1) return json({{"success", false}, {"message", "Error opening"}});

    if(data.has_value()){
        if(_write(fileDescriptor, (data.value().dump() + "\n").c_str(), (unsigned int)(data.value().dump() + "\n").length()) == -1) {
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
    for( auto k : deleted){
        payload += (json({{"key", k}, {"value", store[k]}, {"op", "DELETE"}}).dump() + "\n");
    }
    keys.clear(); // reset keys
    deleted.clear(); // reset

    if(_write(fileDescriptor, payload.c_str(), (unsigned int)payload.length()) == -1) {
        _close(fileDescriptor);
        return json({{"success", false}, {"message", "Error Writing"}});
    }

    _close(fileDescriptor);
    return json({{"success", true}, {"message", "Successfull"}});
}

void StorageEngine::replay() {
    std::lock_guard<std::mutex> lock(store_mutex);

    std::ifstream file(_walFilePath);
    if (!file.is_open())
        return; 

    std::string line;

    while (std::getline(file, line)) {
        if (line.empty())
            continue;

        json entry;

        try {
            entry = json::parse(line);
        } catch (...) {
            break; 
        }

        if (!entry.is_object())
            continue;

        if (!entry.contains("key") || !entry.contains("op") )
            continue;

        const auto& key = entry["key"];
        const auto& op  = entry["op"];

        if (op == "SET") {
            if (!entry.contains("value"))
                continue;

            store[key] = entry["value"];
        }
        else if (op == "DELETE") {
            store.erase(key);
        }
    }
}
