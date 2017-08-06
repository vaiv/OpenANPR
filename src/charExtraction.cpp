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
	cout<<"beginning enhancement"<<endl;
	enhanceImage();
        cout<<"enhancement complete"<<endl;
	ocr();	
	
}

Mat charExtraction::Cluster(Mat img)
{

	Mat3b imlab;
	
    	cvtColor(img, imlab, CV_BGR2Lab);
	
    	/* Cluster image */
    	vector<cv::Mat1b> imgRGB;
    	int k = 3;
    	int n = img.rows * img.cols;
    	Mat img3xN(n, 3, CV_8U);

    	split(imlab, imgRGB);
	
	cout<<"clustering done"<<endl;
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
            		colors.push_back(Vec3b(255,255,255));
        	} else {
            		colors.push_back(Vec3b(0,0,0));
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
//    	waitKey(0);
	return res;

	
}

vector<Mat> charExtraction::detectBlobs(Mat &src)
{
	Mat edges,LPlate;
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	vector<Mat> chars,charMasks;
	vector <Rect> allPossChars;
	if(LPlate.channels()>1)
	cvtColor( src, LPlate, CV_BGR2GRAY );
	else LPlate = src.clone();
	
	Canny( LPlate, edges, 70 , 210 , 3 );
	imshow("char edges", edges);
//	waitKey(0);
	findContours( edges, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
	std::sort(contours.begin(), contours.end(), contour_sorter());
	cout<<"num of possibilities"<<contours.size()<<endl;
	for(int i=0;i<contours.size();i++)
	{
		Rect poss_char = boundingRect(Mat(contours[i]));
		if(poss_char.height >= charPlateRatio*LPlate.rows && poss_char.area()<areaTolerance*LPlate.rows*LPlate.cols)
			{
				Mat poss = src(poss_char).clone();
				//create a uniform square frame around possible character for CNN input
				Mat sq_mask = Mat(poss.rows*1.8,poss.rows*1.8,CV_8UC3,cvScalar(255,255,255));
				poss.copyTo(sq_mask(Rect(poss.rows*0.6,poss.rows*0.45,poss.cols,poss.rows)));
				//add the possible character
				chars.push_back(poss.clone());
				charMasks.push_back(sq_mask.clone());
				//add the roi of the character
				allPossChars.push_back(poss_char);
				imwrite("./res/char"+to_string(i)+".png",sq_mask);
//				cout<<"showing possibilities"<<poss.rows<<" "<<poss.cols<<" "<<poss.channels()<<endl;
//				imshow("extracted char",poss);
//				waitKey(0);
			}
	}

	src = cvScalar(255,255,255);
	for(int i=0;i<allPossChars.size();i++)
	{
		//copy characters back to their respective regions.
		chars[i].copyTo(src(allPossChars[i]));
	}
	return charMasks;
	
}

void charExtraction::enhanceImage()
{
	//cvtColor(src,src,CV_BGR2GRAY);
        resize(src, src, Size(), 2,2, INTER_CUBIC );
	src=Cluster(src);

	//TODO Blob detection thresholding area and inertia for removal of non characters
	cout<<"starting blob detection"<<endl;
	charMasks = detectBlobs(src);
	
   	//dilation helps in certain cases
	int erosion_size=1;
        Mat element = getStructuringElement( MORPH_RECT,
                                            Size( 2*erosion_size + 1, 2*erosion_size+1 ),
                                            Point( erosion_size, erosion_size ) );

        dilate( src,src,element);
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
	//Stroke Width Transform
	Mat swt_output=SWT();
	Mat sub;
	if(imgArea<minArea)
	{
		sub=swt_output;
		cout<<"area low res"<<endl;
	}
	else 
	{
		sub=src;
		cout<<"area high res"<<endl;
	}

	stringstream ss;
	ss<<idx;
	path= string("./res/swt_out")+ ss.str()+string(".jpg");
	imwrite(path.c_str(),sub);

	//tesseract OCR
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

vector<Mat> charExtraction::getCharMasks()
{
	return charMasks;
}
