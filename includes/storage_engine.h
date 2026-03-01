#include <unordered_map>
#include <thread>
#include <optional>
#include <iostream>
#include <string>
#include <mutex>
#include "json/json.hpp"
#include <io.h>      
#include <fcntl.h>   
#include <stdio.h> 
using json = nlohmann::json;
class StorageEngine{

    private:
        std::unordered_map<std::string, std::string> store;
        std::mutex store_mutex;
        std::vector<std::string> keys;
        public:
            std::string _walFilePath;
            int startIndex = 0;
            StorageEngine(){}
            void set(const std::string &key, const std::string &value);
            std::optional<std::string> get(const std::string & key);
            void pop(const std::string &key);
            json writeAhead(std::optional<json> data);
};