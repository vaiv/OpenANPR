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

segmentation::segmentation(Mat src)
{
	//resize(src,src,cvSize(480,320));
	this->src=src;	
	cvtColor( src, src_gray, CV_BGR2GRAY );
	split(src,chan);
	detect_edges();
	extract_contours();
	affine_transform();
}

void segmentation::detect_edges()
{
//reduce noise in grayscale with a 2x2 kernel
	adaptiveThreshold(src_gray,src_gray,255,ADAPTIVE_THRESH_GAUSSIAN_C,THRESH_BINARY,11,2);
    	blur( src_gray, detected_edges[0], Size(2,2) );

//reduce noise in all the channels with a 2x2 kernel
	for(int i=0;i<3;i++)
	{
		blur( chan[i], detected_edges[i+1], Size(2,2) );
	}
// canny detector
	for(int i=0;i<4;i++)
	{
		int highThresh=lowThresh*Ratio;
		Canny( detected_edges[i], detected_edges[i], lowThresh, highThresh, kernel_size );
	}
}

void segmentation::extract_contours()
{
	std::vector<std::vector<cv::Point> > contours;

	for(int i=0;i<4;i++)
	{
		findContours(detected_edges[i].clone(), contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);
		imshow("edges",detected_edges[i]);
//		waitKey(0);
		std::vector<cv::Point> approx;

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
				
           				//fin.push_back(contours[i]);

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
                          		if (mincos >= minCos && maxcos <=maxCos)
                          		{

                              			cv::Rect r = cv::boundingRect(contours[i]);
                              			double ratio = (double)r.width / r.height;

                              			if(ratio >minAspectRatio && ratio<maxAspectRatio)
                                 			fin.push_back(contours[i]);
						else
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


	}

}


void segmentation::affine_transform()
{
	for(int i=0;i<fin.size();i++)
    		{
			int flag=0;
        		Mat roi= src(boundingRect(fin[i]));

        		RotatedRect rec=minAreaRect(fin[i]);
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
            		warpAffine(src, rotated, warpAffineMatrix, size, INTER_LINEAR, BORDER_CONSTANT);
            		//flip(rotated,rotated,-1);
			//prevent repetition
			for(int z=0;z<num_list.size();z++)
			{
				if(fabs(num_list[z].center.x-cent.x)<4 && fabs(num_list[z].center.y-cent.y)<4)
				flag=1;
			}
			if(!flag)
			{
				res.push_back(rotated);
				num_list.push_back(rec);
			}
			imshow("hg",rotated);
//        		cv::waitKey(0);
			stringstream ss;
			ss<<i;
			string path=string("res")+ss.str()+string(".jpg");
			imwrite(path.c_str(),rotated);
		}

}

vector<Mat> segmentation::getResult()
{
	return res;
}









