#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <bitset>
#include "BitStream.hh"
#include "Golomb.hh"

using namespace std;

int main(int argc, char* argv[]){
    Golomb c;
    cout << c.encoder(14, 5) << endl;
    cout << c.decoder("110111", 5) << endl;

    cout << c.encoder(15, 5) << endl;
    cout << c.decoder("111000", 5) << endl;

    return 0;
}
