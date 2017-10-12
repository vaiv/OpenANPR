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

#ifndef SEGMENTATION_H
#define SEGMENTATION_H

#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include<iostream>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include<vector>
#include<cmath>
#include<cstring>
#include<sstream>
#include "arapaho.hpp"

#define kernel_size 3
#define Ratio 3
#define lowThresh 70
#define minContourArea 100
#define minCos -0.4
#define maxCos 0.4
#define minAspectRatio 0.9
#define maxAspectRatio 9.0
#define MAX_OBJ_LIM 100

using namespace cv;
using namespace std;

class segmentation
{
    	std::vector<std::vector<cv::Point> > seg,fin;
	Mat src;
	ArapahoV2ImageBuff arapahoImage;
	vector<Mat> res,detected_edges,detected_plates;
	vector<RotatedRect> num_list;

	string INPUT_DATA_FILE,INPUT_CFG_FILE,INPUT_WEIGHTS_FILE;
	ArapahoV2* p;
	ArapahoV2Params ap;
	bool valid_config;

	void detect_edges(Mat src_gray, Mat chan[]);
	void extract_contours();
	void affine_transform(Mat img, vector< vector < cv::Point > > contours,int im_id);
	bool set_params();
	void detect_plates();
	void localize_plate();

public:
	segmentation(Mat src,int argc,char** argv);
	vector<Mat> getResult();
	bool set_files(int argc, char**argv);

};

#endif // SEGMENTATION_H
