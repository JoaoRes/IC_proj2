#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "BitStream.hh"

using namespace std;

int main(int argc, char* argv[]){
    
    BitStream bstream("test.txt", "result.txt");
    // for(int x : bstream.getBuffer()){
    //     bstream.writeBit(x);
    // }

    // int b = bstream.readBit();
    // cout << b << endl;

    bstream.writeNBits("01100001");

    // vector<int> a = bstream.readNBits(5);
    //printf("%d", a);

    // for(int x : a){
    //     printf("%d", x);
    // }
    bstream.close();
}