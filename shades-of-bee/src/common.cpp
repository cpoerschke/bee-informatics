
#include "common.h"

#include <iostream>

const std::string commonCommandLineParserKeys(const bool manipulateImage, const int defaultNumBins)
{
    std::stringstream keysStream;
    keysStream << "{ help | | print this message }"
               << "{ outputImageFileName |<none>| }"
               ;
    if (manipulateImage)
    {
        keysStream << "{ rotateCode |<none>| cv::RotateFlags value }"
                   << "{ topLeftBottomRight |<none>| }"
                   ;
    }
    if (defaultNumBins)
    {
        keysStream << "{ numBins | " << defaultNumBins << " | }"
                   << "{ histogramImage |<none>| }"
                   << "{ histogramImageMask |<none>| }"
                   << "{ histogramNorm | 255 | }"
                   ;
    }
    return keysStream.str();
}

void rotateAndCropImage(const cv::CommandLineParser& parser, cv::Mat& image)
{
    if (parser.has("rotateCode"))
    {
        cv::rotate(image, image, parser.get<int>("rotateCode"));
    }

    if (parser.has("topLeftBottomRight"))
    {
        const cv::Scalar topLeftBottomRight = parser.get<cv::Scalar>("topLeftBottomRight");

        const cv::Point p1(topLeftBottomRight.val[0], topLeftBottomRight.val[1]);
        const cv::Point p2(topLeftBottomRight.val[2], topLeftBottomRight.val[3]);

        image = cv::Mat(image,
                        cv::Range(p1.y, p2.y) /* rowRange */,
                        cv::Range(p1.x, p2.x) /* colRange */);
    }
}

void writeImage(const cv::CommandLineParser& parser, const cv::Mat& image, const int idx)
{
    if (parser.has("outputImageFileName"))
    {
        cv::imwrite(cv::format(parser.get<cv::String>("outputImageFileName").c_str(), idx), image);
    }
}

Histogram::Histogram(const cv::CommandLineParser& parser)
    : numBins(parser.get<int>("numBins"))
    , histogramNorm(parser.get<int>("histogramNorm"))
    , histogram()
{
    if (parser.has("histogramImage"))
    {
        cv::Mat histogramImage = cv::imread(parser.get<cv::String>("histogramImage"));

        cv::Mat histogramImageMask;
        if (parser.has("histogramImageMask"))
        {
            histogramImageMask = cv::imread(parser.get<cv::String>("histogramImageMask"));
            cv::cvtColor(histogramImageMask, histogramImageMask, cv::COLOR_RGB2GRAY);
        }

        calcHist(histogramImage, histogramImageMask);
    }
}

void Histogram::calcHist(const cv::Mat& image, const cv::Mat& mask)
{
    const int channels[3] = { 0, 1, 2 };
    const int histSizes[3] = { numBins, numBins, numBins };
    const float histRange[2] = { 0, 256 }; // 0 (inclusive) to 256 (exclusive) i.e. 0..255
    const float* histRanges[3] = { histRange, histRange, histRange };

    cv::calcHist(&image /* images */,
                 1 /* number of images */,
                 channels,
                 mask /* mask */,
                 histogram,
                 3 /* dims */,
                 histSizes,
                 histRanges);

    if (histogramNorm)
    {
        cv::normalize(histogram, histogram, 0, histogramNorm, cv::NORM_MINMAX);
    }
}

cv::Mat Histogram::backProject(const cv::Mat& in) const
{
    cv::Mat out;

    const int channels[3] = { 0, 1, 2 };
    const float histRange[2] = { 0, 256 }; // 0 (inclusive) to 256 (exclusive) i.e. 0..255
    const float* histRanges[3] = { histRange, histRange, histRange };

    cv::calcBackProject(&in /* images */,
                        1 /* number of images */,
                        channels,
                        histogram,
                        out /* backProject */,
                        histRanges);

    return out;
}

void Histogram::print(std::ostream &os, const std::string& suffix) const
{
    std::string delimiter;
    for (int dim1 = 0; dim1 < numBins; ++dim1)
    {
        for (int dim2 = 0; dim2 < numBins; ++dim2)
        {
            for (int dim3 = 0; dim3 < numBins; ++dim3)
            {
                std::cout << delimiter
                          << (int)histogram.at<float>(dim1,dim2,dim3);
                delimiter = " ";
            }
        }
    }

    if (!suffix.empty()) std::cout << delimiter << suffix;
}

