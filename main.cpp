#include "includes/server.h"
#include <iostream>

int main(){

    Server s(8080);
    s.serve();
    return 0;
}