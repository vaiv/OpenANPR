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

#ifndef CHAREXTRACTION_H
#define CHAREXTRACTION_H


#include "TextDetection.h"
#include "segmentation.h"
#include <cstring>
#include<sstream>

#define minArea 3000
#define charPlateRatio 0.5
#define areaTolerance 0.2

using namespace DetectText;

struct contour_sorter // 'less' for contours
{
    bool operator ()( const vector<Point>& a, const vector<Point> & b )
    {
        Rect ra(boundingRect(a));
        Rect rb(boundingRect(b));
        // scale factor for y should be larger than img.width
        return ( ra.x < rb.x );
    }
};

class charExtraction
{
	int idx;
	double imgArea;
	string res,path;
	Mat src;
	vector<Mat> charMasks;

	Mat Cluster(Mat img);
	void enhanceImage();
	vector<Mat> detectBlobs(Mat& LPlate);
//function to compute stroke width transform
	Mat SWT();
//function for call to ocr
	void ocr();
	
public:

	charExtraction(Mat src,int idx);
        vector<Mat> getCharMasks();
	string getResultString();
	
};
#endif // CHAREXTRACTION_H

