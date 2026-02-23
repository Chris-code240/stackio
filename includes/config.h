#include <iostream>
#include <string>
#include <fstream>
#include <vector>
// #include "../external/rapidyaml-master/src/c4/yml/yml.hpp"
#include "pugixml.hpp"
struct serverConfig{
    int _port;
    std::string _host;
    int _threads;
    serverConfig(): _port(8080), _host("0.0.0.0"), _threads(4){}
    serverConfig(std::string host, int port, int threads): _host(host), _port(port), _threads(threads){}
};

/*
storage:
  data_dir: "./data"
  wal_file: "wal.log"
  snapshot_file: "snapshot.dat"
  flush_interval_ms: 5000
*/
struct storageConfig{

    std::string _data_dir;
    std::string _wal_file;
    std::string _snapshot_file;
    int _flush_interval_ms;
    storageConfig(std::string data_dir = "./data",std::string wal_file = "wal.log", std::string snapshot_file = "snapshot.dat", int flust_interval = 5000): _data_dir(data_dir), _wal_file(wal_file), _snapshot_file(snapshot_file), _flush_interval_ms(5000){}
};
/*
auth:
  enabled: true
  jwt_secret: "change_me"
  token_expiry_minutes: 60*/
struct authConfig {
    bool _enabled;
    std::string _secret;
    int _token_expiry_minutes;
    authConfig():_enabled(false){}
    authConfig(std::string secret, int token_expiry_minutes = 60, bool enabled = true) : _secret(secret), _token_expiry_minutes(token_expiry_minutes), _enabled(enabled){}
};
/*
api:
  max_body_size_kb: 512
  rate_limit_per_minute: 60*/

struct apiConfig {
    int _max_body_size_kb;
    int _rate_limit_per_minute;

    apiConfig(int max_body_size_kb = 512, int rate_limit_per_minute = 60) : _max_body_size_kb(max_body_size_kb), _rate_limit_per_minute(rate_limit_per_minute) {}
};
struct Configuration{
    serverConfig _serverConfig;
    storageConfig _storageConfig;
    authConfig _authConfig;
    apiConfig _apiConfig;

    Configuration(std::string config_file_path){
        if(config_file_path.empty()){
            std::cerr<<"[CONFIG_ERROR] Configuration file ("<<config_file_path<<") is empty\n";
            std::exit(-1);
        }
        std::ifstream file(config_file_path);
        if(!file.is_open()){
            std::cerr<<"[CONFIG_ERROR] Could not open "<<config_file_path<<"\n";
            std::exit(-1);
        }
        std::string content =  "", line;
        while(std::getline(file, line)){
            content += line;
        }
        pugi::xml_document doc;
        pugi::xml_parse_result parsedDoc = doc.load_string(content.c_str());
        if(!parsedDoc){
            std::cerr<<"[CONFIG_ERROR] Could not parse config file: "<<config_file_path<<"\nDescription:\n\t"<<parsedDoc.description();
            std::exit(-1);
        }
        for(pugi::xml_node node : doc.child("config")){
            if(node.name() == "storage"){
                for(pugi::xml_node tag : node){
                    if(tag.name() == "data_dir") _storageConfig._data_dir = tag.text().as_string();
                    if(tag.name() == "flush_interval_ms") _storageConfig._flush_interval_ms = tag.text().as_int();
                    if(tag.name() == "snapshot_file") _storageConfig._snapshot_file = tag.text().as_string();
                    if(tag.name() == "wal_file") _storageConfig._wal_file = tag.text().as_string();
                }
            }
            if(node.name() == "server"){
                for(pugi::xml_node tag : node){
                    if(tag.name() == "threads") _serverConfig._threads = tag.text().as_int();
                    if(tag.name() == "port") _serverConfig._port = tag.text().as_int();
                    if(tag.name() == "host") _serverConfig._host = tag.text().as_string();
                }
            }
            if(node.name() == "auth"){
                for(pugi::xml_node tag : node){
                    if(tag.name() == "enabled") _authConfig._enabled = tag.text().as_bool();
                    if(tag.name() == "secret") _authConfig._secret = tag.text().as_string();
                    if(tag.name() == "token_expiry_minutes") _authConfig._token_expiry_minutes = tag.text().as_int();

                }
            }
            if(node.name() == "api"){
                for(pugi::xml_node tag : node){
                    if(tag.name() == "max_body_size_kb") _apiConfig._max_body_size_kb = tag.text().as_int();
                    if(tag.name() == "rate_limit_per_minutes") _apiConfig._rate_limit_per_minute = tag.text().as_int();
                }
            }

        }
    }
};