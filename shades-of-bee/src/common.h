
#include <opencv2/opencv.hpp>

const std::string commonCommandLineParserKeys(const bool manipulateImage = true, const int defaultNumBins = 0);

void rotateAndCropImage(const cv::CommandLineParser& parser, cv::Mat& image);

void writeImage(const cv::CommandLineParser& parser, const cv::Mat& image, const int idx);

class Histogram {
public:
    Histogram(const cv::CommandLineParser& parser);
    void calcHist(const cv::Mat& image, const cv::Mat& mask);
    cv::Mat backProject(const cv::Mat& in) const;
    void print(std::ostream &os, const std::string& suffix) const;
private:
    const int numBins;
    const int histogramNorm;
    cv::Mat histogram;
};

