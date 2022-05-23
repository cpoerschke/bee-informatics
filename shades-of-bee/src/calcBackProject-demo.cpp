
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

            << "{ inputImageFileName      |<none>| }"
            // outputImageFileName is part of common keys

            << "{ interactive | true | }"
            ;
    }
    const cv::CommandLineParser parser(argc, argv, keysStream.str());

    if (parser.has("help"))
    {
        parser.printMessage();
        return 0;
    }

    const Histogram histogram(parser);

    const cv::Mat image = cv::imread(parser.get<cv::String>("inputImageFileName"));
    const cv::Mat backProject = histogram.backProject(image);

    writeImage(parser, backProject, 0 /* idx */);

    if (parser.get<bool>("interactive"))
    {
        cv::imshow("image", image);
        cv::imshow("backProject", backProject);
        cv::waitKey(10000); // wait up to 10 seconds
    }

    return 0;
}

