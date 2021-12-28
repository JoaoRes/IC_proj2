#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace std;

class BitStream{
    public:
        BitStream(string inFile, string outFile);
        int readBit(void);
        string readNBits(int nBits);
        void writeBit(char bit);
        void writeNBits(string nBits);
        vector<int>getBuffer();
        void close();
    private:
        vector<int> buffer;
        int index=0;
        fstream inFile;
        fstream outFile;

        int bitcount = 0;
        char currentByte = 0;

};

BitStream::BitStream(string inF, string outF){
    if(not inF.empty()) {
        inFile.open(inF);
        char c;
        while (inFile.get(c)) {
            for (int i = 7; i >= 0; i--) // or (int i = 0; i < 8; i++)  if you want reverse bit order in bytes
                buffer.push_back(((c >> i) & 1));
        }
    }else{
        cout << "No In File to open" << endl;
    }

    if(not outF.empty()){
        outFile.open(outF, fstream::out | fstream::app);
    }else{
        cout << "No Out File to open" << endl;
    }
}

int BitStream::readBit(){
    if(inFile.is_open()){
        return buffer.at(index++);
    }else{
        cout << "File not Open!!" << endl;
        return 1;
    }
}

string BitStream::readNBits(int nBits){
    if(inFile.is_open()){
        string nbits_string="";
        int stopCond=index+nBits;
        for( ; index<stopCond ; index++)
            nbits_string += to_string(buffer[index]);
            
        return nbits_string;

    }else{
        cout << "File not Open!!" << endl;
        return "";
    }
}

// not 100%, not sure how
void BitStream::writeBit(char bit){
    if(outFile.is_open()){
        currentByte = currentByte << 1;
        currentByte |= bit-'0';
        bitcount++;
        if (bitcount == 8){
            outFile << currentByte;
            currentByte = 0;
            bitcount = 0;
        }
    }else{
        cout << "File not Open!!" << endl;
    }
}

void BitStream::writeNBits(string nBits){
        for(int i=0 ; i<(int)nBits.length() ; i++){
            writeBit(nBits[i]);
        }
}

void BitStream::close(void){
    //BitStream close method
    if(inFile.is_open()){
        inFile.close();
    }
    if(outFile.is_open()){
        if (bitcount != 0){
            currentByte = currentByte << (8-bitcount);
            outFile << currentByte;
        }
        outFile.close();
    }
}

vector<int> BitStream::getBuffer(){
    return buffer;
}
