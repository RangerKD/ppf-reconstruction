#ifndef OPENCVHELPERS_H
#define OPENCVHELPERS_H

#endif // OPENCVHELPERS_H

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <string>
#include <sstream>
#include <iostream>

#include <string>
#include <iostream>

namespace OpenCVHelpers {
using namespace std;
using namespace cv;

// parameter processing
template<typename T> bool getParam(std::string param, T &var, int argc, char **argv)
{
    const char *c_param = param.c_str();
    for(int i=argc-1; i>=1; i--)
    {
        if (argv[i][0]!='-') continue;
        if (strcmp(argv[i]+1, c_param)==0)
        {
            if (!(i+1<argc)) continue;
            std::stringstream ss;
            ss << argv[i+1];
            ss >> var;
            std::cout<<"PARAM[SET]: "<<param<<" : "<<var<<std::endl;
            return (bool)ss;
        }
    }
    std::cout<<"PARAM[DEF]: "<<param<<" : "<<var<<std::endl;
    return false;
}

// parameter processing: template specialization for T=bool
template<> inline bool getParam<bool>(std::string param, bool &var, int argc, char **argv)
{
    const char *c_param = param.c_str();
    for(int i=argc-1; i>=1; i--)
    {
        if (argv[i][0]!='-') continue;
        if (strcmp(argv[i]+1, c_param)==0)
        {
            if (!(i+1<argc) || argv[i+1][0]=='-') { var = true; return true; }
            std::stringstream ss;
            ss << argv[i+1];
            ss >> var;
            return (bool)ss;
        }
    }
    return false;
}

string getOSSeparator() {
#ifdef _WIN32
  return "\\";
#else
  return "/";
#endif
}

#include <dirent.h>
#include <vector>

bool isPrefixAndSuffix(const char* file, uint16_t filename_length, string prefix, string suffix){
   /// const char* file = filename.c_str();
    char* startPrefix=strstr(file,prefix.c_str());
    char* startSuffix=strstr(file,suffix.c_str());

    bool isPrefix = (startPrefix-file == 0);
    bool isSuffix = (startSuffix-file) == filename_length - suffix.length();

    //cout<<"start: "<<isPrefix<<" end:"<<isSuffix<<endl;

    return isPrefix && isSuffix;
}
#include <algorithm>    // std::any_of
//#include <array>        // std::array

const vector<string> SUFFIX_IMAGE = {".png", ".jpg",".tif",".exr"};
const vector<string> SUFFIX_TEXT  = {".txt",".xyz"};
//const std::array<string,1> SUFFIX_PLY   = {".ply"};

bool hasPrefixAndSuffixes(const char* file, uint16_t filename_length, string prefix,std::vector<string> suffixes){
  return std::any_of(suffixes.begin(), suffixes.end(),
         [&](string suffix){return isPrefixAndSuffix(file,filename_length,prefix,suffix);
  });
}

//http://stackoverflow.com/questions/9277906/stdvector-to-string-with-custom-delimiter
string join(vector<string>& v, const string& delim) {
    ostringstream s;
    for (const auto& i : v) {
        if (&i != &v[0]) {
            s << delim;
        }
        s << i;
    }
    return s.str();
}

vector<string> getAllFilesFromFolder(string dirStr, string prefix, vector<string> suffixes) {
  DIR *dir = NULL;
  struct dirent *entry;
  vector<string> allImages;

  dir = opendir(dirStr.c_str());

  if (!dir) {
    cerr << "Could not open directory " << dirStr << ". Exiting..." << endl;
    exit(1);
  }

  const string sep = getOSSeparator();

  while(entry = readdir(dir)) {
    if (hasPrefixAndSuffixes(entry->d_name,entry->d_namlen,prefix,suffixes)){
      string fileName(entry->d_name);
      string fullPath = dirStr + sep + fileName;
      allImages.push_back(fullPath);
    }
  }
  closedir(dir);

  std::sort(allImages.begin(), allImages.end(), [](const std::string &left, const std::string &right) {
      int lengthdiff=left.size()-right.size();
      if(lengthdiff==0){
          return left < right;
      }
      return lengthdiff<0;
  });

  string joined = join(suffixes,"|");

  cout<<"Found "<<allImages.size()<<" files in "<<dirStr << " that match the pattern: "<<prefix<<"*"<<joined<<" .";
  if(allImages.size()>0){
      cout<<" first is: "<<allImages[0];
  }
  cout<<endl;
  return allImages;
}

vector<string> getAllImagesFromFolder(string dirStr, string prefix){
    return getAllFilesFromFolder(dirStr,prefix,SUFFIX_IMAGE);
}

vector<string> getAllTextFilesFromFolder(string dirStr, string prefix){
    return getAllFilesFromFolder(dirStr,prefix,SUFFIX_TEXT);
}

void showImage(const string &wndTitle, const Mat& img){
    cv::namedWindow(wndTitle, WINDOW_AUTOSIZE);
    cv::imshow(wndTitle.c_str(), img);
}

void imagesc(std::string title, cv::Mat mat,InputArray mask = noArray()) {
  double min,max;
  cv::minMaxLoc(mat,&min,&max, 0, 0, mask);

  Mat scaled = mat;
  //Mat meanCols;
  //reduce(mat, meanCols, 0, REDUCE_AVG );

  //Mat mean;
  //reduce(meanCols, mean, 1, REDUCE_AVG);

  //cout << "Max value: " << max << endl;
  //cout << "Mean value: " << mean.at<float>(0) << endl;
  //cout << "Min value: " << min << endl;

  if (std::abs(max) > 0.0000001f)
    scaled /= max;

  showImage(title, scaled);
}


void showDepthImage(const string &wndTitle, const Mat& img, InputArray mask = noArray(),bool print=false) {
  double min, max;
  minMaxIdx(img, &min, &max, 0, 0, mask);

  if(print) cout<<"    showDepthImage scaled:  min: "<<min<<" max:"<<max;




  //Mat img2=img*mask;
  Mat depthMap;

  float scale = 255.0f / (max - min);
  img.convertTo(depthMap, CV_8UC1, scale, -min*scale);


  Mat heatMap;
  applyColorMap(depthMap, heatMap, cv::COLORMAP_JET);

  cv::namedWindow(wndTitle, WINDOW_AUTOSIZE);
  cv::imshow(wndTitle.c_str(), heatMap);
}

std::string getImageType(int imgTypeInt)
{
    int numImgTypes = 35; // 7 base types, with five channel options each (none or C1, ..., C4)

    int enum_ints[] =       {CV_8U,  CV_8UC1,  CV_8UC2,  CV_8UC3,  CV_8UC4,
                             CV_8S,  CV_8SC1,  CV_8SC2,  CV_8SC3,  CV_8SC4,
                             CV_16U, CV_16UC1, CV_16UC2, CV_16UC3, CV_16UC4,
                             CV_16S, CV_16SC1, CV_16SC2, CV_16SC3, CV_16SC4,
                             CV_32S, CV_32SC1, CV_32SC2, CV_32SC3, CV_32SC4,
                             CV_32F, CV_32FC1, CV_32FC2, CV_32FC3, CV_32FC4,
                             CV_64F, CV_64FC1, CV_64FC2, CV_64FC3, CV_64FC4};

    string enum_strings[] = {"CV_8U",  "CV_8UC1",  "CV_8UC2",  "CV_8UC3",  "CV_8UC4",
                             "CV_8S",  "CV_8SC1",  "CV_8SC2",  "CV_8SC3",  "CV_8SC4",
                             "CV_16U", "CV_16UC1", "CV_16UC2", "CV_16UC3", "CV_16UC4",
                             "CV_16S", "CV_16SC1", "CV_16SC2", "CV_16SC3", "CV_16SC4",
                             "CV_32S", "CV_32SC1", "CV_32SC2", "CV_32SC3", "CV_32SC4",
                             "CV_32F", "CV_32FC1", "CV_32FC2", "CV_32FC3", "CV_32FC4",
                             "CV_64F", "CV_64FC1", "CV_64FC2", "CV_64FC3", "CV_64FC4"};

    for(int i=0; i<numImgTypes; i++)
    {
        if(imgTypeInt == enum_ints[i]) return enum_strings[i];
    }
    return "unknown image type";
}

}