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

#include"charExtraction.h"

charExtraction::charExtraction(Mat src,int idx)
{
	this->idx=idx;
	this->src=src;
	imgArea=src.rows*src.cols;
	res="NA";

	enhanceImage();
	ocr();	
	
}

void charExtraction::enhanceImage()
{
	//cvtColor(src,src,CV_BGR2GRAY);
        resize(src, src, Size(), 2,2, INTER_CUBIC );

//	int erosion_size=1.5;
//      Mat element = getStructuringElement( MORPH_RECT,
//                                              Size( 2*erosion_size + 1, 2*erosion_size+1 ),
//                                              Point( erosion_size, erosion_size ) );

//      erode( rotated,rotated,element);
	cvtColor(src, src, CV_BGR2RGB);

	imshow("ww",src);
	waitKey(0);
}

Mat charExtraction::SWT()
{
	Mat swt_output = textDetection(src, 1);
	return swt_output;
} 

void charExtraction::ocr()
{
	Mat swt_output=SWT();
	Mat sub;
	if(imgArea<minArea)
	{
		sub=src;
		cout<<"area low res"<<endl;
	}
	else 
	{
		sub=swt_output;
		cout<<"area high res"<<endl;
	}

	stringstream ss;
	ss<<idx;
	path= string("./res/swt_out")+ ss.str()+string(".jpg");
	imwrite(path.c_str(),sub);

	const char* out;
	tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
	if (api->Init(NULL, "eng")) 
		{
        		fprintf(stderr, "Could not initialize tesseract.\n");
        		exit(1);
    		}

	//tesseract::TessBaseAPI tess; 
	//tess.SetImage((uchar*)src.data, src.size().width, src.size().height, src.channels(), src.step1());
	//tess.Recognize(0);

	Pix *image = pixRead(path.c_str());
    	api->SetImage(image);
    	// Get OCR result
    	out = api->GetUTF8Text();
	//const char* out = tess.GetUTF8Text();
	res=string(out);


}


string charExtraction::getResultString()
{
	if(!res.empty())
	return res;
	else return string("NA");
}
