
#include "common.h"

#include <iostream>

#include <opencv2/opencv.hpp>

int main(int argc, char* argv[])
{
    // compose command line parser
    std::stringstream keysStream;
    {
        keysStream
            << commonCommandLineParserKeys(false /* manipulateImage */, 8 /* defaultNumBins */)
            << "{ blockSize |<none>| }"
            ;
    }
    const cv::CommandLineParser parser(argc, argv, keysStream.str());

    if (parser.has("help"))
    {
        parser.printMessage();
        return 0;
    }

    if (parser.has("histogramImage"))
    {
        const Histogram histogram(parser);
        histogram.print(std::cout, "\n");
    }

    if (parser.has("blockSize"))
    {
        cv::Mat paletteImage;

        std::vector<std::pair<std::string,cv::Scalar>> palette;
        palette.push_back(std::make_pair("black\n"   , cv::Scalar(  0,   0,   0)));
        palette.push_back(std::make_pair("red\n"     , cv::Scalar(  0,   0, 255)));
        palette.push_back(std::make_pair("green\n"   , cv::Scalar(  0, 255,   0)));
        palette.push_back(std::make_pair("yellow\n"  , cv::Scalar(  0, 255, 255)));
        palette.push_back(std::make_pair("blue\n"    , cv::Scalar(255,   0,   0)));
        palette.push_back(std::make_pair("magenta\n" , cv::Scalar(255,   0, 255)));
        palette.push_back(std::make_pair("cyan\n"    , cv::Scalar(255, 255,   0)));
        palette.push_back(std::make_pair("white\n"   , cv::Scalar(255, 255, 255)));
        for (const auto& pair : palette)
        {
            const cv::Mat image(cv::Size(parser.get<int>("blockSize"), parser.get<int>("blockSize")), CV_8UC3, pair.second);
            if (paletteImage.empty())
            {
                paletteImage = image;
            }
            else
            {
                cv::hconcat(paletteImage /* src1 */, image /* src2 */, paletteImage /* dst */);
            }
            Histogram histogram(parser);
            histogram.calcHist(image, cv::Mat());
            histogram.print(std::cout, pair.first);
        }
        // thin gray border
        cv::rectangle(paletteImage, cv::Point(0, 0), cv::Point(paletteImage.cols-1, paletteImage.rows-1), cv::Scalar(127, 127, 127) /* gray */);

        writeImage(parser, paletteImage, 0 /* idx */);

        cv::imshow("paletteImage", paletteImage);
        cv::waitKey(10000); // wait up to 10 seconds
    }

    return 0;
}

