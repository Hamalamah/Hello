#include <iostream> // for standard I/O
#include <string>   // for strings
#include <iomanip>  // for controlling float print precision
#include <sstream>  // string to number conversion
#include "opencv2/features2d/features2d.hpp"
#include <opencv2/core/core.hpp>        // Basic OpenCV structures (cv::Mat, Scalar)
#include <opencv2/imgproc/imgproc.hpp>  // Gaussian Blur
#include <opencv2/highgui/highgui.hpp>  // OpenCV window I/O
#include "opencv2/nonfree/features2d.hpp"

using namespace std;
using namespace cv;
int main(int argc, char *argv[])
{

if (argc != 5)
{
    cout << "Not enough parameters" << endl;
    return -1;
}
    
    stringstream conv;
    
    const string source=argv[1];
    int Value, delay,redValue;
    conv << argv[2] << endl << argv[3]<<endl<<argv[4];       // put in the strings
    conv >> Value >> delay>>redValue;
    
    
    char c;
    int frameNum=-1;

    VideoCapture capt(source);
    if (!capt.isOpened())
    {
        cout << "Could not open Video"<<source<<endl;
        return -1;
    }

    namedWindow("Video",1);
    namedWindow("NewVideo",1);
    Mat frame;
    Mat redframe;
    
    for(;;)
    {
        capt>>frame;
        if(frame.empty())
        {cout << "game over";
            break;
        }
        ++frameNum;
       
        ///RedExtractor
        redframe=Mat::zeros(frame.size(),frame.type());
        for( int y = 0; y < redframe.rows; y++ )
        {
            for( int x = 0; x < redframe.cols; x++ )
            {
                if(frame.at<Vec3b>(y,x)[2]>redValue && frame.at<Vec3b>(y,x)[1]<redValue && frame.at<Vec3b>(y,x)[0]<redValue)
                {
                    for( int c = 0; c < 3; c++ )
                    {
                        redframe.at<Vec3b>(y,x)[c]=frame.at<Vec3b>(y,x)[c];
                    }
                }
            }
        }

        
        ///specular reflection
        /*
        for( int y = 0; y < frame.rows; y++ )
        {
            for( int x = 0; x < frame.cols; x++ )
            {
                if(frame.at<Vec3b>(y,x)[0]>Value && frame.at<Vec3b>(y,x)[1]>Value && frame.at<Vec3b>(y,x)[2]>Value)
                {
                    for( int c = 0; c < 3; c++ )
                    { if(y>0 && x>0)
                        {
                            frame.at<Vec3b>(y,x)[c]=(frame.at<Vec3b>(y-1,x-1)[c]+frame.at<Vec3b>(y,x-1)[c]+frame.at<Vec3b>(y-1,x)[c])/3;
                        }
                    }
                }
            }
        }
        
        */
        
        ///Feature Detection
        int minHessian=400;
        
        SurfFeatureDetector detector(minHessian);
        
        vector<KeyPoint> keypoint;
        vector<KeyPoint> keypointr;
        
        detector.detect( frame, keypoint);
        detector.detect( redframe, keypointr);
        
        Mat frame_keypoints;
        Mat frame_keypointsr;
        
        drawKeypoints(frame,keypoint,frame_keypoints,Scalar::all(-1), DrawMatchesFlags::DEFAULT);
        drawKeypoints(redframe,keypointr,frame_keypointsr,Scalar::all(-1), DrawMatchesFlags::DEFAULT);
        
       
        ///Feature Extractor
        SurfDescriptorExtractor extractor;
        
        Mat descriptors, descriptorsr;
        
        extractor.compute( frame, keypoint, descriptors );
        extractor.compute( redframe, keypointr, descriptorsr );
        ///
        
        ///Flann Based Matcher
        FlannBasedMatcher matcher;
        std::vector< DMatch > matches;
        matcher.match( descriptors, descriptorsr, matches );
        
        double max_dist = 0; double min_dist = 100;
        
        for( int i = 0; i < descriptors.rows; i++ )
        { double dist = matches[i].distance;
            if( dist < min_dist ) min_dist = dist;
            if( dist > max_dist ) max_dist = dist;
        }

        
        std::vector< DMatch > good_matches;
        
        for( int i = 0; i < descriptors.rows; i++ )
        { if( matches[i].distance <= 2*min_dist )
        { good_matches.push_back( matches[i]); }
        }
        
        Mat img_matches;
        drawMatches( frame, keypoint, redframe, keypointr,
                    good_matches, img_matches, Scalar::all(-1), Scalar::all(-1),
                    vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );
        
        ///
        
        cout<<"Frame: "<<frameNum<<"# "<<endl;
        imshow( "Good Matches", img_matches );
        //imshow("Video",frame_keypoints);
        //imshow("NewVideo",frame_keypointsr);
        //imshow("NewVideo",frame_keypointsn);
        c = (char)cvWaitKey(delay);
        if (c == 27) break;
    }
    return 1;
}
