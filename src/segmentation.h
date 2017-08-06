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

#define kernel_size 3
#define Ratio 3
#define lowThresh 70
#define minContourArea 100
#define minCos -0.4
#define maxCos 0.4
#define minAspectRatio 2.4
#define maxAspectRatio 9.0

using namespace cv;
using namespace std;

class segmentation
{
    	std::vector<std::vector<cv::Point> > seg,fin;
	Mat src,src_gray,chan[3],detected_edges[4];
	vector<Mat> res;
	vector<RotatedRect> num_list;

	void detect_edges();
	void extract_contours();
	void affine_transform();

public:
	segmentation(Mat src);
	vector<Mat> getResult();
	

};

#endif // SEGMENTATION_H
