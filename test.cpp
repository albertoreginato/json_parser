#include <iostream>
#include <string>
#include <limits>
#include <assert.h>
#include <fstream>


#include "json.cpp"
using std::cout;
using std::cin;
using std::endl;

int main() {
    
    
    json jsn3;
    std::ifstream file1("prova3.json");
    file1 >> jsn3;
    cout << jsn3 << endl;
    cout << "fine" << endl;

    
    
    return 0;
    
}

