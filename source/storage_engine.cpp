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

json StorageEngine::writeAhead(json data) {
    std::lock_guard<std::mutex> lock(store_mutex);
    if(!data.is_object()) return json({{"success", false}, {"message", "Data must be object"}});

    // 1. Correct Flag placement: _O_APPEND belongs here
    int fileDescriptor = _open(_walFilePath.c_str(), _O_WRONLY | _O_CREAT | _O_APPEND, _S_IREAD | _S_IWRITE);
    
    if(fileDescriptor == -1) return json({{"success", false}, {"message", "Error opening"}});

    // 2. Dump once, add newline for JSONL format
    std::string payload = data.dump() + "\n";

    // 3. Use the stored string length
    if(_write(fileDescriptor, payload.c_str(), (unsigned int)payload.length()) == -1) {
        _close(fileDescriptor);
        return json({{"success", false}, {"message", "Error Writing"}});
    }

    _close(fileDescriptor);
    return json({{"success", true}, {"message", "Successfull"}});
}