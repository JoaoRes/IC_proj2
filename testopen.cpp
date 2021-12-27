#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

//#include "LosslessCodec.hh"


using namespace std;
using namespace cv;

int main(int argc, char* argv[]) {

    // LosslessCodec g("/home/diogo/Documents/IC/airplane.ppm");

    Mat img = imread("imagens_PPM/boat.ppm");

    if (img.empty()) {
        cout << "Wrong path" << endl;
    }

    Mat yuv;
    Mat again;

    cvtColor(img, yuv, COLOR_RGB2YUV_I420);
    cvtColor(yuv,again,COLOR_YUV420p2BGR);
    imshow("teste YUV", yuv);
    imshow("teste YUV 2 RGB",again );


    Mat maty (img.size().height, img.size().width, CV_8UC1);
    Mat matu (img.size().height, img.size().width, CV_8UC1);
    Mat matv (img.size().height, img.size().width, CV_8UC1);
    float r,g,b;
    int y , u, v;
    for (int i=0; i < img.size().height ; i++){
        for( int j=0 ; j< img.size().width ; j++){
            r = img.at<Vec3b>(i,j)[0];
            g = img.at<Vec3b>(i,j)[1];
            b = img.at<Vec3b>(i,j)[2]; 
            
            y =  0.257 * r + 0.504 * g + 0.098 * b +  16;
            u = -0.148 * r - 0.291 * g + 0.439 * b + 128;
            v =  0.439 * r - 0.368 * g - 0.071 * b + 128;

            //u = 128 - 0.168736*r - 0.331264*g + 0.5*b;
            //v = 128 + 0.5*r - 0.418688*g - 0.081312*b;

            maty.at<uchar>(i,j) =y;
            matu.at<uchar>(i,j)= u;
            matv.at<uchar>(i,j) =v;

        }
    }
    Mat channels[3] = {maty,matu,matv};
    Mat output;
    merge(channels,3,output);
    
    imshow("fuck", img);
    waitKey(0);
}