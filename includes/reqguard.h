#pragma once
#include <unordered_map>
#include <vector>
#include <iostream>
#include <string>
#include "request.h"
#include "config.h"
#include <time.h>
#include <ctime>
class BucketState{
    private:
        int _requests_per_minute;
        bool _allow;
        int request_counter = 0;
        time_t start_time;
    public:
    BucketState(int requests_per_minute = 60): _requests_per_minute(requests_per_minute), _allow(true){
        time(&start_time);
     }

    void reset(){
        time_t newTime;
        time(&newTime);
        if(difftime(newTime, start_time) > 60){
            request_counter = 0;
            time(&start_time);
            _allow = true;
        }
    }
    json checkRequest(){
        reset();
        
        if(request_counter < _requests_per_minute){
            request_counter++;
            _allow = true;
            return json{
                {"allow", true},
                {"requests_used", request_counter},
                {"requests_remaining", _requests_per_minute - request_counter}
            };
        } else {
            _allow = false;
            time_t now;
            time(&now);
            int retry_after = 60 - static_cast<int>(difftime(now, start_time));
            return json{
                {"allow", false},
                {"retry_after_seconds", retry_after},
                {"requests_remaining", 0}
            };
        }
    }
};

class RequestGuard{
    private:
        std::unordered_map<std::string, BucketState> _buckets;
        apiConfig _config;
    public:
        RequestGuard(apiConfig config): _config(config){}

        json examineRequest(HttpRequest  request){

            if(_buckets.find(request._ip) == _buckets.end()){
                _buckets[request._ip] = BucketState(_config._rate_limit_per_minute);
            }

            return _buckets[request._ip].checkRequest();
        }
    

};