#include <iostream>
#include <math.h>
using namespace std;

class Golomb{
    public:
        Golomb(int m);
        void encoder(int num);
        void decoder();
        void decToBinary(int n);
    private:
        int m_;
};

Golomb::Golomb(int m=0){
    if(m=0){
        //calculate m;
    }
    m_=m;
}

void Golomb::encoder(int num){
    int q=0;
    int r=0;

    q=floor(num/m_);
    r=num-q*m_;


}

// void decToBinary(int n) {
//     // array to store binary number
//     int binaryNum[1000];
  
//     // counter for binary array
//     int i = 0;
//     while (n > 0) {
  
//         // storing remainder in binary array
//         binaryNum[i] = n % 2;
//         n = n / 2;
//         i++;
//     }
  
//     // printing binary array in reverse order
//     for (int j = i - 1; j >= 0; j--)
//         cout << binaryNum[j];
// }
