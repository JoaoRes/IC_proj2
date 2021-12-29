#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "BitStream.hh"

using namespace std;

int main(int argc, char* argv[]){
    
    BitStream bstream("test.txt", "result.txt");
    
    // test.txt contem um 'a'
    // deverão ser impressos os primeiros 5 bits
    // da representação ascii de 'a' 
    for(char x : bstream.readNBits(5)){
        cout << x;
    }
    cout << endl;

    bstream.writeNBits("0110000111");
    bstream.close();
    // como só é possivel escrever 1 byte
    // são adicionados '0' no final para completar 1 byte

    BitStream bstream1("result.txt", "");
    string a = bstream1.readNBits(10);

    cout << a << endl; // deverá ser impresso os bits anteriormente escritos no ficheiro

    bstream1.close();
}