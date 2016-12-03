/*
    Copyright (C) 2016  Vaibhav Darbari

    OpenANPR is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenANPR is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
 g++ -std=c++11 -o ANPR ../main.cpp ../segmentation.h ../segmentation.cpp ../charExtraction.h  ../charExtraction.cpp ../TextDetection.h ../TextDetection.cpp -I /usr/local/include/opencv2 -L/usr/local/lib -lopencv_core -lopencv_imgproc -lopencv_highgui -ltesseract -llept  
*/
#include <iostream>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <cmath>
#include <stdlib.h>
#include <stdio.h>
#include<iostream>
#include"segmentation.h"
#include"charExtraction.h"

using namespace std;
using namespace cv;

int main(int argc, char *argv[])
{
    //Mat src=imread("/home/vaibhav/Desktop/cars/road.jpg");
    Mat src=imread(argv[1],CV_LOAD_IMAGE_COLOR);
    if(! src.data )                              // Check for invalid input
        {
            cout <<  "Could not open or find the image" << std::endl ;
            cout<<argv[1]<<endl;
            return -1;
        }
    segmentation X(src);
    vector<Mat> res=X.getResult();

    for(int i=0;i<res.size();i++)
   {
        charExtraction tmp(res[i],i);
        cout<<"number_plate "<<i<<":"<<tmp.getResultString()<<endl;
    }

    return 0;
}

