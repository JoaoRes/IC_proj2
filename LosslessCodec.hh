#include <iostream>
#include <math.h>
#include <bits/stdc++.h>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>


using namespace std;
using namespace cv;

class LosslessCodec{
    public:
        LosslessCodec(int m);
    private:
        Mat yuv;
        predictor()


};

void LosslessCodec::YUV420(Mat img){
    cvtColor(img, img, COLOR_RGB2YUV);
    // cvtColor(img, img, COLOR_RGB2YUV_I420);
    split(img, yuv_channels);

    Mat tmp_u (img.size().height/2, img.size().width/2, CV_8UC1);
    Mat tmp_v (img.size().height/2, img.size().width/2, CV_8UC1);

    int u_i = 0, v_i = 0;
    int u_j = 0, v_j = 0;
    for (int i = 0; i < img.size().height; i+=2) {
        for (int j = 0; j < img.size().width; j+=2) {
            tmp_u.at<uchar>(u_i, u_j++) = yuv_channels[1].at<uchar>(i,j);
            tmp_v.at<uchar>(v_i, v_j++) = yuv_channels[2].at<uchar>(i,j);
        }
        u_i++; v_i++;
        u_j = v_j = 0;
    }

    yuv_channels[1] = tmp_u;
    yuv_channels[2] = tmp_v;
}

Mat LosslessCodec::predictor(Mat img){
    Mat error (img.size().height, img.size().width, CV_8UC1);
    int a=0,b=0,c=0,x;
    for (int i=0; i < img.size().height ; i++){
        for( int j=0 ; j< img.size().width ; j++){
            if(i==0 and j==0){
                a =0;
                b =0;
                c =0;
            }
            else if (i==0 and j!=0){
                b =0;
                c =0;
                a =(int) img.at<uchar>(i,j-1);
            }
            else if(i=!0 and j==0){
                a =0;
                c =0;
                b =(int) img.at<uchar>(i-1,j);
            }
            else{
                a =(int) img.at<uchar>(i,j-1);
                b =(int) img.at<uchar>(i-1,j);
                c =(int) img.at<uchar>(i-1,j-1);
            }
            
            if(c >= max(a,b)){
                x = min(a,b);
            }
            else if (c <= min(a,b)){
                x = max(a,b);
            }
            else{
                x = a+b-c;
            }

            error.at<uchar>(i,j) = img.at<uchar>(i,j) - (uchar) x ;
            
        }
    }
    return error;
}



void LosslessCodec::encode(string path){
    Mat img = imread(path);

    Mat channels[3];
    YUV420(img,channels);

    Mat error[3];
    for(int i =0 ; i<3 ; i++){
        error[i] = predictor(channels[i]);
    }
}


