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


#include"segmentation.h"

static double angle(cv::Point pt1, cv::Point pt2, cv::Point pt0)
{
    double dx1 = pt1.x - pt0.x;
    double dy1 = pt1.y - pt0.y;
    double dx2 = pt2.x - pt0.x;
    double dy2 = pt2.y - pt0.y;
    return (dx1*dx2 + dy1*dy2)/sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}

segmentation::segmentation(Mat src,int argc,char** argv)
{
	//resize(src,src,cvSize(480,320));
	this->src=src;
	set_files(argc,argv);	
	p =  new ArapahoV2();
	if(!p)
	    {
		cout<<"error laoding localisation module"<<endl;
		exit(0);
	    }
	
	if(valid_config)
	{
		set_params();
		detect_plates();
	}
	for(int i=0;i<detected_plates.size();i++)
	{
		Mat src_gray, chan[3];
		Mat Res;
		pyrMeanShiftFiltering( detected_plates[i], Res, 10, 25, 3);
		imshow("meanshift res",Res);
		waitKey(0);
		cvtColor(Res, src_gray, CV_BGR2GRAY );
		split(Res,chan);
		detect_edges(src_gray,chan);
	}
	// canny detector
	for(int i=0;i<detected_edges.size();i++)
	{
//		imshow("edges",detected_edges[i]);
//		waitKey(0);
		int highThresh=lowThresh*Ratio;
		Canny( detected_edges[i], detected_edges[i], lowThresh, highThresh, kernel_size, true );

	}
	//localize_plate();
	extract_contours();
}

bool file_exists(const char *fileName)
{
    std::ifstream infile(fileName);
    return infile.good();
}

bool segmentation::set_files(int argc, char**argv)
{
	INPUT_DATA_FILE = argv[2];
	INPUT_CFG_FILE =  argv[3];
	INPUT_WEIGHTS_FILE = argv[4];
	
	valid_config = (file_exists(INPUT_DATA_FILE.c_str()) && file_exists(INPUT_CFG_FILE.c_str()) && file_exists(INPUT_WEIGHTS_FILE.c_str()) );
	return valid_config;
	
}
bool segmentation::set_params()
{
    ap.datacfg = (char*)INPUT_DATA_FILE.c_str();
    ap.cfgfile = (char*)INPUT_CFG_FILE.c_str();
    ap.weightfile = (char*)INPUT_WEIGHTS_FILE.c_str();
    ap.nms = 0.4;
    ap.maxClasses = 1;
    int f = 0;
    return p->Setup(ap, f, f);

}

void segmentation::detect_plates()
{
	arapahoImage.bgr = src.data;
        arapahoImage.w = src.size().width;
        arapahoImage.h = src.size().height;
        arapahoImage.channels = src.channels();
	
	int numDetections=0;
	p->Detect(src,0.24,0.5,numDetections);
	
	box boxes[numDetections];
	string labels[numDetections];

	if(numDetections > 0 && numDetections < MAX_OBJ_LIM)
	{
		p->GetBoxes(boxes,labels,numDetections);	
	}
	cout<<"number of detections are "<<numDetections<<endl;
	for(int Id=0;Id<numDetections;Id++)
	{
		int leftTopX = 1 +src.size().width*(boxes[Id].x - boxes[Id].w / 2);
                int leftTopY = 1 + src.size().height*(boxes[Id].y - boxes[Id].h / 2);
		int roi_width = src.size().width*boxes[Id].w;
		int roi_height = src.size().height*boxes[Id].h;

		roi_height = src.size().height >= leftTopY + roi_height ? roi_height : src.size().height - leftTopY;
		roi_width = src.size().width >= leftTopX + roi_width ? roi_width : src.size().width - leftTopX;
		try
			{
				Mat tmp = src(Rect(leftTopX - roi_width*0.25,leftTopY - roi_height*0.75 ,roi_width*1.5,roi_height*2.5)).clone();
//				imshow("roi",tmp);
//				waitKey(0);
				detected_plates.push_back(tmp.clone());
				tmp = src(Rect(leftTopX,leftTopY,roi_width,roi_height)).clone();
				Point top_left = Point(leftTopX,leftTopY);
				Point bot_right = Point(leftTopX + roi_width, leftTopY + roi_height);
				rectangle(src,top_left,bot_right,Scalar(0,0,255));
				imwrite("predicted.png",src);
				res.push_back(tmp.clone());
			}
		catch(...)
		{
			continue;
		}
				
	}	
}

void segmentation::detect_edges(Mat src_gray , Mat chan[] )
{
//reduce noise in grayscale with a 2x2 kernel
	adaptiveThreshold(src_gray,src_gray,255,ADAPTIVE_THRESH_GAUSSIAN_C,THRESH_BINARY,11,2);
	Mat L1,L2,L3,L4,L5,L6;
    	GaussianBlur( src_gray, L1, Size(3,3),0 );
	cv::addWeighted(src_gray, 1.5, L1, -0.5, 0, L2);
	GaussianBlur( src_gray, L3, Size(5,5),0 );
	GaussianBlur( src_gray, L4, Size(7,7),0 );
	cv::addWeighted(L3, 1.5, L4, -0.5, 0, L5);
	blur( src_gray, L6, Size(2,2) );
	//detected_edges.push_back(L1.clone());
	detected_edges.push_back(L2.clone());
	//detected_edges.push_back(L3.clone());
	//detected_edges.push_back(L4.clone());
	detected_edges.push_back(L5.clone());
	detected_edges.push_back(L6.clone());
	//imshow("L2",L2);
//reduce noise in all the channels with a 2x2 kernel
	for(int i=0;i<3;i++)
	{
		//adaptiveThreshold(chan[i], chan[i],255,CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY,11,2);
//		imshow("chan",chan[i]);
//		waitKey(0);
		Mat G1,G2,G3,G4,G5,G6;
		GaussianBlur( chan[i], G1, Size(3,3),0 );
		cv::addWeighted(chan[i], 1.5, G1, -0.5, 0, G2);
		GaussianBlur( chan[i], G3, Size(5,5),0 );
		GaussianBlur( chan[i], G4, Size(7,7),0 );
		cv::addWeighted(G3, 1.5, G4, -0.5, 0, G5);
		blur( chan[i], G6, Size(2,2) );
		//detected_edges.push_back(G1.clone());
		detected_edges.push_back(G2.clone());
		//detected_edges.push_back(G3.clone());
		//detected_edges.push_back(G4.clone());
		detected_edges.push_back(G5.clone());
		detected_edges.push_back(G6.clone());
		//imshow("G2",G2);
		//waitKey(0);
	}

}

/*Size2f reorder(vector<Point> appx)
{
	float dim1 = sqrt(pow(appx[0].x - appx[3].x,2)+pow(appx[3].y - appx[2].y,2));
	float dim2 = sqrt(pow(appx[0].x - appx[1].x,2)+pow(appx[1].y - appx[2].y,2));
	Size2f dims;
	dims.height = (dim1>=dim2) ? dim2 : dim1;
	dims.width =  (dim1>=dim2) ? dim1 : dim2;
	return dims;
}*/

void segmentation::extract_contours()
{
	std::vector<std::vector<cv::Point> > contours;

	for(int k=0;k<detected_edges.size();k++)
	{
		findContours(detected_edges[k].clone(), contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);
		fin.clear();
		std::vector<cv::Point> approx;
/*		Mat tmp = Mat(detected_edges[j].size(),detected_edges[j].type());
		drawContours(tmp, contours, -1, (0,255,0), 3);
		cout<<"showing for channel "<<j<<endl;
		imshow("possible contours",tmp);
		waitKey(0);
*/
//		imshow("edges",detected_edges[k]);
//		waitKey(0);
  		for (int i = 0; i < contours.size(); i++)
  		{
      			// Approximate contour with accuracy proportional
      			// to the contour perimeter
      			cv::approxPolyDP(
          			cv::Mat(contours[i]),
          			approx,
        			cv::arcLength(cv::Mat(contours[i]), true) * 0.02,
                		//7,
          			true
      			);

			// Skip small or non-convex objects
      			if (std::fabs(cv::contourArea(contours[i])) < 100 || !cv::isContourConvex(approx))
          		continue;
     			if(approx.size() >= 4)
      				{
            				seg.push_back(contours[i]);
					cout<<"quadrilateral roi detected"<<endl;
					
           				//fin.push_back(contours[i]);
//					imshow("res",detected_plates[k/12](boundingRect(contours[i])));
//							waitKey(0);
          				// Number of vertices of polygonal curve
                  			int vtc = approx.size();
                  			// Get the degree (in cosines) of all corners
                  			std::vector<double> cos;
                  			for (int j = 2; j < vtc+1; j++)
                      				cos.push_back(angle(approx[j%vtc], approx[j-2], approx[j-1]));

                  			// Sort ascending the corner degree values
                  			std::sort(cos.begin(), cos.end());

                  			// Get the lowest and the highest degree
                  			double mincos = cos.front();
                  			double maxcos = cos.back();


                  			// Use the degrees obtained above and the number of vertices
                          		// to determine the shape of the contour
					cv::Rect r = cv::boundingRect(contours[i]);
						double ratio = (double)r.width / r.height;
                          		if (mincos >= minCos && maxcos <=maxCos)
                          		{
						cout<<"approx polygon vertices"<<approx<<endl;

                              			if(ratio >minAspectRatio && ratio<maxAspectRatio)
                                 			fin.push_back(contours[i]);
							

						//else
							cout<<"curr ratio:"<<ratio<<endl;

                                 		cout<<mincos<<" "<<maxcos<<endl;

                                 		Point2f vert[3],maxV(0,0);
                               			for (int z = 0; z < vtc; z++)
                                  		{
                                   			cout<<approx[z].x<<" "<<approx[z].y<<endl;
                               			}


                              
                          		}

      				}
    		}

	    
	    affine_transform(detected_plates[k/12],fin,k/12);


	}

}


void segmentation::affine_transform(Mat img, vector< vector < cv::Point > >contours,int im_id)
{
	num_list.clear();
	for(int i=0;i<contours.size();i++)
		{
			bool flag=false;
        		Mat roi= img(boundingRect(contours[i]));
        		RotatedRect rec=minAreaRect(contours[i]);
        		Point2f vertices[4];
        		rec.points(vertices);

        		Point2f src_vertices[3];
		//            src_vertices[0] = vertices[3];
		//            src_vertices[1] = vertices[0];
		//            src_vertices[2] = vertices[2];

            		Point2f cent;
            		cent=rec.center;
            		for(int z=0;z<4;z++)
            			{
                			if(vertices[z].x<cent.x && vertices[z].y < cent.y)
                    				src_vertices[0]=vertices[z];
                			else if(vertices[z].x>cent.x && vertices[z].y < cent.y)
                    				src_vertices[1]=vertices[z];
                			else if(vertices[z].x<cent.x && vertices[z].y > cent.y)
                    				src_vertices[2]=vertices[z];
            			}

        		Point2f dst_vertices[3];
            		dst_vertices[0] = Point(0, 0);
            		dst_vertices[1] = Point(rec.boundingRect().width-1, 0);
            		dst_vertices[2] = Point(0, rec.boundingRect().height-1);

            		Mat warpAffineMatrix = getAffineTransform(src_vertices, dst_vertices);

            		Mat rotated;
            		Size size(rec.boundingRect().width, rec.boundingRect().height);
            		warpAffine(img, rotated, warpAffineMatrix, size, INTER_LINEAR, BORDER_CONSTANT);
            		//flip(rotated,rotated,-1);
			//prevent repetition
			for(int z=0;z<num_list.size();z++)
			{
				if(fabs(num_list[z].center.x-cent.x)<4 && fabs(num_list[z].center.y-cent.y)<4)
				flag=true;
			}
			if(!flag)
			{
				res.push_back(rotated);
				num_list.push_back(rec);
			}
//			imshow("hg",rotated);
//        		cv::waitKey(0);
			string path=string("res_")+to_string(im_id)+"_"+to_string(i)+string(".jpg");
			imwrite(path.c_str(),rotated);
		}

}

vector<Mat> segmentation::getResult()
{
	return res;
}


cv::Point2f computeIntersect(cv::Vec4i a, cv::Vec4i b)  
 {  
   int x1 = a[0], y1 = a[1], x2 = a[2], y2 = a[3];  
   int x3 = b[0], y3 = b[1], x4 = b[2], y4 = b[3];  
   if (float d = ((float)(x1-x2) * (y3-y4)) - ((y1-y2) * (x3-x4)))  
   {  
     cv::Point2f pt;  
     pt.x = ((x1*y2 - y1*x2) * (x3-x4) - (x1-x2) * (x3*y4 - y3*x4)) / d;  
     pt.y = ((x1*y2 - y1*x2) * (y3-y4) - (y1-y2) * (x3*y4 - y3*x4)) / d;  
           //-10 is a threshold, the POI can be off by at most 10 pixels
           if(pt.x<min(x1,x2)-10||pt.x>max(x1,x2)+10||pt.y<min(y1,y2)-10||pt.y>max(y1,y2)+10){  
                return Point2f(-1,-1);  
           }  
           if(pt.x<min(x3,x4)-10||pt.x>max(x3,x4)+10||pt.y<min(y3,y4)-10||pt.y>max(y3,y4)+10){  
                return Point2f(-1,-1);  
           }  
     return pt;  
   }  
   else  
     return cv::Point2f(-1, -1);  
 }  


bool comparator(Point2f a,Point2f b){  
           return a.x<b.x;  
      }  

 void sortCorners(std::vector<cv::Point2f>& corners, cv::Point2f center)  
 {  
   std::vector<cv::Point2f> top, bot;  
   for (int i = 0; i < corners.size(); i++)  
   {  
     if (corners[i].y < center.y)  
       top.push_back(corners[i]);  
     else  
       bot.push_back(corners[i]);  
   }  
      sort(top.begin(),top.end(),comparator);  
      sort(bot.begin(),bot.end(),comparator);  
   cv::Point2f tl = top[0];
   cv::Point2f tr = top[top.size()-1];
   cv::Point2f bl = bot[0];
   cv::Point2f br = bot[bot.size()-1];  
   corners.clear();  
   corners.push_back(tl);  
   corners.push_back(tr);  
   corners.push_back(br);  
   corners.push_back(bl);  
 }  


void segmentation::localize_plate()
{
    for(int id=0;id<detected_edges.size();id++)
	{

		vector<Vec4i> lines;
	 	HoughLinesP(detected_edges[id], lines, 1, CV_PI/180, 100, 40, 10);
		Mat cdst = Mat::zeros(detected_edges[id].size(),detected_edges[id].type());
		cvtColor(cdst, cdst, CV_GRAY2BGR);
		for( size_t i = 0; i < lines.size(); i++ )
		{
		  Vec4i l = lines[i];
		  line( cdst, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 3, CV_AA);
		}
		imshow("houghP",cdst);
		waitKey(0);
		int* poly = new int[lines.size()];  
  		for(int i=0;i<lines.size();i++)poly[i] = - 1;  
  		int curPoly = 0;  
       		vector<vector<cv::Point2f> > corners;  
      		for (int i = 0; i < lines.size(); i++)  
      			{  
           			for (int j = i+1; j < lines.size(); j++)  
           			{  
          
               				 cv::Point2f pt = computeIntersect(lines[i], lines[j]);  
                			if (pt.x >= 0 && pt.y >= 0&&pt.x<detected_edges[id].size().width&&pt.y<detected_edges[id].size().height){  
              
                     			if(poly[i]==-1&&poly[j] == -1){  
                          		vector<Point2f> v;  
                          		v.push_back(pt);  
                          		corners.push_back(v);       
                          		poly[i] = curPoly;  
                          		poly[j] = curPoly;  
                          		curPoly++;  
                          		continue;  
                     		}  
                     	if(poly[i]==-1&&poly[j]>=0){  
                          corners[poly[j]].push_back(pt);  
                          poly[i] = poly[j];  
                          continue;  
                     }  
                     	if(poly[i]>=0&&poly[j]==-1){  
                          corners[poly[i]].push_back(pt);  
                          poly[j] = poly[i];  
                          continue;  
                     }  
                     	if(poly[i]>=0&&poly[j]>=0){  
                          	if(poly[i]==poly[j]){  
                               		corners[poly[i]].push_back(pt);  
                               		continue;  
                          	}  
                        
                          for(int k=0;k<corners[poly[j]].size();k++){  
                               corners[poly[i]].push_back(corners[poly[j]][k]);  
                          }  
                       
                          corners[poly[j]].clear();  
                          poly[j] = poly[i];  
                          continue;  
                     }  
                }  
           }  
      }  


		for(int i=0;i<corners.size();i++){  
           cv::Point2f center(0,0);  
           if(corners[i].size()<4)continue;  
           for(int j=0;j<corners[i].size();j++){  
                center += corners[i][j];  
           }  
           center *= (1. / corners[i].size());  
           sortCorners(corners[i], center);  
      }  

	for(int i=0;i<corners.size();i++){  
           if(corners[i].size()<4)continue;  
           Rect r = boundingRect(corners[i]);
	   Mat tmp = detected_edges[id](boundingRect(corners[i])); 
	   imshow("poss quads",tmp);
	   waitKey(0); 
           if(r.area()<500 || r.width < r.height)continue;  
           cout<<r.area()<<endl;  
           // Define the destination image  
           cv::Mat quad = cv::Mat::zeros(r.height, r.width, CV_8UC3);  
           // Corners of the destination image  
           std::vector<cv::Point2f> quad_pts;  
           quad_pts.push_back(cv::Point2f(0, 0));  
           quad_pts.push_back(cv::Point2f(quad.cols, 0));  
           quad_pts.push_back(cv::Point2f(quad.cols, quad.rows));  
           quad_pts.push_back(cv::Point2f(0, quad.rows));  
           // Get transformation matrix  
           cv::Mat transmtx = cv::getPerspectiveTransform(corners[i], quad_pts);  
           // Apply perspective transformation  
           cv::warpPerspective(detected_edges[id], quad, transmtx, quad.size());  
           stringstream ss;  
           ss<<i<<".jpg";  
           imshow(ss.str(), quad);
	   waitKey(0);  
      }    
	}
}






