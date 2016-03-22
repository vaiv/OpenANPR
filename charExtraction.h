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

using namespace DetectText;

class charExtraction
{
	int idx;
	double imgArea;
	string res,path;
	Mat src;

	void enhanceImage();
//function to compute stroke width transform
	Mat SWT();
//function for call to ocr
	void ocr();
	
public:

	charExtraction(Mat src,int idx);
	string getResultString();
	
};
#endif // CHAREXTRACTION_H

