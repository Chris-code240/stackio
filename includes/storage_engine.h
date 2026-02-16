#include <unordered_map>
#include <thread>
#include <optional>
#include <iostream>
#include <string>
#include <mutex>

class StorageEngine{

    private:
        std::unordered_map<std::string, std::string> store;
        std::mutex store_mutex;

        public:
            StorageEngine(){}
            void set(const std::string &key, const std::string &value);
            std::optional<std::string> get(const std::string & key);
};