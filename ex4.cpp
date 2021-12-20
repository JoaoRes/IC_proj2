#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "BitStream.hh"
#include "Golomb.hh"

using namespace std;




int main(int argc, char* argv[]){


    // uint64_t numDecimal = 19;
    // uint64_t numUnary = 0;
    //int *p;
    
    // if(numDecimal == 0) {
    //     numUnary = 0;
    // }
    // else if (numDecimal>=1) {
    //     numUnary=1;
    //     for(int i = 0 ; i< numDecimal; i++) {
    //         numUnary= (numUnary*10)+1;
    //     }
    //     numUnary=numUnary-1;
    // }

    // p = decToBinary(2);

    // for(int i = 0; i< sizeof(p)-1 ; i++){
    //     printf("%d"), *(p+1);
    // }
    Golomb c(4);
    //Golomb a(3);
    c.encoder(13);
    c.decoder("11101",4);
}
