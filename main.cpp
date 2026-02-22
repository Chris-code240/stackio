#include "includes/server.h"
#include <iostream>

int main(int argc, char* argv[]){
    Configuration config(argv[1]);
    
    Server s(config);
    s.serve();
    return 0;
}