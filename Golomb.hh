#include <iostream>
#include <math.h>
#include <bitset>
using namespace std;

class Golomb{
    public:
        string encoder(int num, int m);
        short decoder(string code, int m);
        string decToBinary(int n);
    private:
        int q=0;
        int r=0;
};

string Golomb::decToBinary(int n)
{
    // array to store binary number
    int binaryNum[32];
 
    // counter for binary array
    int i = 0;
    while (n > 0) {
 
        // storing remainder in binary array
        binaryNum[i] = n % 2;
        n = n / 2;
        i++;
    }
 
    // printing binary array in reverse order
    string result;
    for (int j = i - 1; j >= 0; j--)
        result += to_string(binaryNum[j]);

    return result;
}

string Golomb::encoder(int i, int m){
    bitset<64> r_binary;
    string numUnary;
    string numBinary;

    q=floor(i/m);
    r=i-q*m;

    if(q == 0) {
        numUnary = "0";
    }
    else if (q >= 1) {
        for(int j=0 ; j<q ; j++) {
            numUnary +="1";
        }
        numUnary=numUnary+"0";
    }

    // if m is power of 2
    if(ceil(log2(m)) == floor(log2(m))){
        if(r==0){
            numBinary="0";
        }else{
            numBinary=decToBinary(r);
        }
    } else { // if m is not power of 2
        int b=ceil(log2(m));
        int nBits;

        if(r<pow(2, b)-m){
            nBits=b-1;
        }
        else {
            r = r+pow(2, b)-m;
            nBits = b;
        }

        string auxStr = bitset<64>(r).to_string();
        string reverse;

        for (int j=63; j>63-nBits; j--){
            reverse=auxStr[j]+reverse;
        }

        numBinary+=reverse;
    }
    
    //cout << "unary " << numUnary  << " binary " << numBinary << endl;
    return numUnary+numBinary;
}

short Golomb::decoder(string code, int m){
    short value=0;

    int separation = (int) code.find('0');
    string unary_q = code.substr(0,separation);
    string binary_r;
    if((separation+1)<(int)code.length()){
        binary_r = code.substr(separation+1);
    }else{
        binary_r="0";
    }

    //quotient convertion - unary to decimal
    int q = unary_q.size();
    value = q*m;

    //remainder convertion - binary to decimal
    int r = (int) stoull(binary_r, 0, 2);
    
    //if m is power of 2
    if(ceil(log2(m)) == floor(log2(m))){
        value+=r;
    }else{ // if m is not power of 2
        int b=ceil(log2(m));

        if(r<pow(2, b)-m){
            value+=r;
        }else{
            r = r - pow(2, b) + m;
            value+=r;
        }
    }

    return value;
}
