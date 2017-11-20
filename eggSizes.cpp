#include <opencv2/highgui/highgui.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/core/core.hpp>
#include <iostream>
#include <fstream>
#include <stdio.h>
using namespace cv;
using namespace std;


int main( int argc, char** argv )
{
    //ERROR HANDLING
    if(argc < 2)
    {
        std::cout<<"Usage: "<<argv[0]<<" file1 file2 ..."<<std::endl;
        return 0;
    }
    
    std::ofstream fout("results.txt");
    fout << "image" <<"\t"<< "egg" <<"\t"<< "width" <<"\t"<< "length" <<"\t"<<std::endl;
            
    //FILE LOOP
    for(int f=1; f<argc; f++)
    {
        
        // storage
        Mat img = imread(argv[f]);
        cv::Mat input,grey,imgBlurred,imgThreshed,output;
        cvtColor(img, grey, COLOR_BGR2GRAY);

        // pre-processing
        cv::blur(grey, imgBlurred, cv::Size(9,9));
        cv::threshold(imgBlurred, imgThreshed, 155, 255, CV_THRESH_BINARY);

        // find contours
        vector<vector<Point>> contours;
        findContours(imgThreshed.clone(), contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);

        vector<int> indices(contours.size());
        iota(indices.begin(), indices.end(), 0);

        sort(indices.begin(), indices.end(), [&contours](int lhs, int rhs) {
            return contours[lhs].size() > contours[rhs].size();
        });

        // return N largest contours
        int N = 3; 
        N = min(N, int(contours.size()));
        
        // draw contours
        cv::Mat result(imgThreshed.size(),CV_8U,cv::Scalar(0));
        cv::Mat imgOutput(imgThreshed.size(),CV_8U,cv::Scalar(255));
        
        for (int i = 0; i < N; ++i)
        {
            cv::drawContours(result,contours,indices[i],cv::Scalar(255),CV_FILLED);
            cv::drawContours(imgOutput,contours,indices[i],cv::Scalar(0),CV_FILLED);
        }
        
        // minimum enclosing rectangle for each contour
        cvtColor(imgOutput, imgOutput, CV_GRAY2BGR);
        for (int i = 0; i < N; ++i)
        {
            // find
            cv::RotatedRect minRect = cv::minAreaRect(cv::Mat(contours[indices[i]]));
            
            // draw
            cv::Point2f vertices[4];
            minRect.points(vertices);
            for (int j = 0; j < 4; j++)
            {
                line(imgOutput, vertices[j], vertices[(j+1)%4], cv::Scalar(0,0,255),5);
            }
            
            // find bounding box centroid and add text
            std::string s = std::to_string(i+1);
            putText(imgOutput, s, minRect.center, FONT_HERSHEY_DUPLEX, 6, Scalar(255,0,0), 8);
        
            // print width and height
            fout<< argv[f] << "\t"<< i+1 << "\t"<< minRect.size.width <<"\t"<< minRect.size.height <<"\t"<< std::endl;
        }

        // write image mask
        std::ostringstream maskfile;
        maskfile << argv[f] << "_mask.tif";
        cv::imwrite(maskfile.str(),imgOutput);
    }
    
    return 0;
}