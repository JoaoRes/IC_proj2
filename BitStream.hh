#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace std;

class BitStream{
    public:
        BitStream(string inFile, string outFile);
        void writeBit(int bit);
        int readBit(void);
        vector<int> readNBits(int nBits);
        void writeNBits(vector<int> nBits);
        vector<int>getBuffer();
        void close();
    private:
        vector<int> buffer;
        fstream inFile;
        fstream outFile;
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
        cout << "No out File to open" << endl;
    }
}


void BitStream::writeBit(int bit){
    if(outFile.is_open()){
        outFile << bit;
    }else{
        cout << "File not Open!!" << endl;
    }
}

int BitStream::readBit(){
    if(inFile.is_open()){
        vector<int> aux;
        int bit=buffer.at(0);
        for(int i=1 ; i<buffer.size() ; i++){
            aux.push_back(buffer[i]);
        }
        buffer=aux;
        return bit;
    }
    cout << "File not Open!!" << endl;
    return 1;
}

vector<int> BitStream::readNBits(int nBits){
    vector<int> nbits_vector;
    if(inFile.is_open()){
        vector<int> aux;
        for(int i=1 ; i<buffer.size() ; i++){
            aux.push_back(buffer[i]);
        }
        for(int i=0 ; i<nBits ; i++){
            nbits_vector.push_back(buffer[i]);
        }
        buffer=aux;
        return nbits_vector;
    }
    cout << "File not Open!!" << endl;
    return nbits_vector;
}

void BitStream::writeNBits(vector<int> nBits){
    if(outFile.is_open()){
        for(int x: nBits){
            outFile << x;
        }
    }else{
        cout << "File not Open!!" << endl;
    }
}

void BitStream::close(void){
    //BitStream close method
    if(inFile.is_open()){
        inFile.close();
    }
    if(outFile.is_open()){
        outFile.close();
    }
}

vector<int> BitStream::getBuffer(){
    return buffer;
}
