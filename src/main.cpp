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
#include <fstream>
#include <cmath>
#include <stdlib.h>
#include <stdio.h>
#include<iostream>
#include "charRecognition.h"
#include"segmentation.h"
#include"charExtraction.h"


//using namespace std;
using namespace cv;

int main(int argc, char *argv[])
{
    //Mat src=imread("/home/vaibhav/Desktop/cars/road.jpg");
    Mat src=imread(argv[1],CV_LOAD_IMAGE_COLOR);
    fstream fil;
    fil.open("./res/predictions.txt");
    Classifier recogniser("../CharNet/lenet_char.prototxt","../CharNet/lenet_char_iter_10000.caffemodel");
    if(! src.data )                              // Check for invalid input
        {
            std::cout <<  "Could not open or find the image" << std::endl ;
            std::cout<<argv[1]<<std::endl;
            return -1;
        }
    segmentation X(src,argc,argv);
    vector<Mat> res=X.getResult();
    std::cout<<"beginning char recognition"<<std::endl;

    for(int i=0;i<res.size();i++)
   {
        charExtraction tmp(res[i],i);
	//initialise classifier
	//Classifier recogniser("../CharNet/lenet_char.prototxt","../CharNet/lenet_char_iter_10000.caffemodel",tmp.getCharMasks());
	recogniser.Run(tmp.getCharMasks());
        std::string out = recogniser.getRes();
	if(out.length()>4 && tmp.getResultString().length()>4) //sanity check
        {
		std::cout<<"CNN predicition: Number Plate "<<i<<": "<<out<<std::endl;
        	std::cout<<"tesseract prediction: Number Plate "<<i<<": "<<tmp.getResultString()<<std::endl;
		fil<<"CNN predicition: Number Plate "<<i<<": "<<out<<std::endl;
        	fil<<"tesseract prediction: Number Plate "<<i<<": "<<tmp.getResultString()<<std::endl;
	}
    }

    return 0;
}

