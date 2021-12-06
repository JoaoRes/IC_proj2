#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "BitStream.hh"

using namespace std;

int main(int argc, char* argv[]){
    
    BitStream bstream("test.txt", "result.txt");
    for(int x : bstream.getBuffer()){
        bstream.writeBit(x);
    }
    bstream.close();
}