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

Mat charExtraction::Cluster(Mat img)
{

	Mat3b imlab;
    	cvtColor(img, imlab, CV_BGR2Lab);

    	/* Cluster image */
    	vector<cv::Mat3b> imgRGB;
    	int k = 2;
    	int n = img.rows * img.cols;
    	Mat img3xN(n, 3, CV_8U);

    	split(imlab, imgRGB);

    	for (int i = 0; i != 3; ++i)
        	imgRGB[i].reshape(1, n).copyTo(img3xN.col(i));

    	img3xN.convertTo(img3xN, CV_32F);

    	vector<int> bestLables;
    	kmeans(img3xN, k, bestLables, cv::TermCriteria(), 10, cv::KMEANS_RANDOM_CENTERS);

    	/* Find the largest cluster*/
    	int max = 0, indx= 0, id = 0;
    	vector<int> clusters(k,0);

    	for (size_t i = 0; i < bestLables.size(); i++)
    		{
        		id = bestLables[i];
        		clusters[id]++;

        		if (clusters[id] > max)
        		{
            			max = clusters[id];
            			indx = id;
        		}
    		}

    /* save largest cluster */
    	int cluster = indx;

    	vector<Point> shape; 
    	shape.reserve(2000);

    	for (int y = 0; y < imlab.rows; y++)
    	{
        	for (int x = 0; x < imlab.cols; x++)
        	{
            		if (bestLables[y*imlab.cols + x] == cluster) 
            			{
                			shape.emplace_back(x, y);
            			}
        	}
    	}

    	int inc = shape.size();

    // Show results
    	Mat3b res(img.size(), Vec3b(0,0,0));
    	vector<Vec3b> colors;
    	for(int i=0; i<k; ++i)
    	{
        	if(i == indx) {
            		colors.push_back(Vec3b(255, 255,255));
        	} else {
            		colors.push_back(Vec3b(0, 0 / (i+1), 0));
        		}
    	}

    	for(int r=0; r<img.rows; ++r)
    	{
        	for(int c=0; c<img.cols; ++c)
        	{
           		 res(r,c) = colors[bestLables[r*imlab.cols + c]];
        	}
    	}

    	imshow("Clustering", res);
    	waitKey(0);
	return res;

	
}

void charExtraction::enhanceImage()
{
	//cvtColor(src,src,CV_BGR2GRAY);
        resize(src, src, Size(), 2,2, INTER_CUBIC );
	//src=Cluster(src);

	//TODO Blob detection thresholding area and inertia for removal of non characters

   	//dilation helps in certain cases
	int erosion_size=1;
        Mat element = getStructuringElement( MORPH_RECT,
                                            Size( 2*erosion_size + 1, 2*erosion_size+1 ),
                                            Point( erosion_size, erosion_size ) );

        //dilate( src,src,element);
	//erode( src,src,element);
	cvtColor(src, src, CV_BGR2RGB);
	imwrite("./res/tmp.jpg",src);
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
