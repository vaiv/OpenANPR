#include <caffe/caffe.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <algorithm>
#include <iosfwd>
#include <memory>
#include <string>
#include <utility>
#include <vector>


using namespace caffe;  // NOLINT(build/namespaces)
using namespace cv;

/* Pair (label, confidence) representing a prediction. */
typedef std::pair<string, float> Prediction;


class Classifier {
 public:
  Classifier(const std::string& model_file,const std::string& trained_file, vector<Mat> char_poss);

  std::vector<Prediction> Classify(const cv::Mat& img, int N = 5);
  void Run();
  std::string getRes();

 private:
  std::string find_label(int idx);
  void WrapInputLayer(std::vector<cv::Mat>* input_channels);

  void Preprocess(const cv::Mat& img,std::vector<cv::Mat>* input_channels);
  std::vector<float> Predict(const cv::Mat& img);
  

 private:
  shared_ptr<Net<float> > net_;
  cv::Size input_geometry_;
  int num_channels_;
  cv::Mat mean_;
  std::vector<string> labels_;
  std::vector<Mat> char_masks;
  std::string extractedString;
};

