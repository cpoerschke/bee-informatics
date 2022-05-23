
#include "common.h"

#include <opencv2/opencv.hpp>

void putPointAndNumber(cv::Mat& image, const size_t pointNumber, cv::Point point,
                       const cv::Scalar& fontColor = cv::Scalar(0, 255, 255) /* yellow */,
                       const int fontFace = cv::FONT_HERSHEY_SIMPLEX,
                       const int pixelHeight = 9,
                       const int thickness = 1)
{
    cv::circle(image, point, pixelHeight/3 /* radius */, fontColor, cv::FILLED);

    const double fontScale = cv::getFontScaleFromHeight(fontFace, pixelHeight, thickness);

    std::stringstream textStr;
    textStr << pointNumber;

    point.x += (pointNumber % 2 ? -2*pixelHeight : +pixelHeight);
    point.y += (+pixelHeight / 2);
    cv::putText(image, textStr.str(), point, fontFace, fontScale, fontColor, thickness);
}

void contourLine(cv::Mat& image,
                 cv::Point& prevPoint,
                 const std::vector<cv::Point>& contour,
                 const cv::Scalar& color)
{
    for (const auto& currPoint : contour)
    {
        if (prevPoint != currPoint)
        {
            cv::line(image, prevPoint, currPoint, color);
            prevPoint = currPoint;
        }
    }
}

cv::Mat contourLines(const cv::Mat& image,
                     const std::vector<std::vector<cv::Point>>& previousContours,
                     const std::vector<cv::Point>& currentContour,
                     const cv::Scalar& previousContourColor = cv::Scalar(0, 255, 0) /* green */,
                     const cv::Scalar& currentContourColor = cv::Scalar(0, 0, 255) /* red */)
{
    cv::Mat imageCopy = image.clone();

    cv::Point prevPoint = *(previousContours.begin()->begin());

    for (const auto& previousContour : previousContours)
    {
        contourLine(imageCopy, prevPoint, previousContour, previousContourColor);
    }

    contourLine(imageCopy, prevPoint, currentContour, currentContourColor);
  
    return imageCopy;
}

// forward declare
void mouseCallback(int event, int x, int y, int flags, void* userData);

class Display {
public:
    Display(const std::string& _windowName, const cv::Mat& _image, const cv::Point& sourcePoint, const bool interactive)
      : windowName(_windowName)
      , image(_image)
      , contours()
    {
        cv::imshow(windowName, image);
        if (interactive) cv::setMouseCallback(windowName, mouseCallback, this /* userData */);

        contours.push_back(std::vector<cv::Point>(1, sourcePoint));
    }
    ~Display()
    {
        cv::destroyWindow(windowName);
    }
    cv::Mat setCursor(const cv::Point& cursorPoint, const int event = cv::EVENT_LBUTTONDOWN, const char *cursorPointDesc = NULL)
    {
        cv::segmentation::IntelligentScissorsMB tool;
        tool.applyImage(image);

        tool.buildMap(*(contours.back().rbegin()) /* source point */); // end of previous contour

        std::vector<cv::Point> nextContour;
        tool.getContour(cursorPoint /* target point */, nextContour);

        const cv::Mat annotatedImage = contourLines(image, contours, nextContour);
        cv::imshow(windowName, annotatedImage);

        if (cv::EVENT_LBUTTONDOWN == event)
        {
            contours.push_back(nextContour);
            if (cursorPointDesc) std::cout << cursorPointDesc << " = " << cursorPoint << std::endl;
        }

        return annotatedImage;
    }
    cv::Mat getMask() const
    {
        cv::Mat mask(image.size(), image.type(), cv::Scalar(0, 0, 0) /* black */);

        std::vector<cv::Point> overallContour;

        for (const auto& currContour : contours)
        {
            for (const auto& currPoint : currContour)
            {
                overallContour.push_back(currPoint);
            }
        }

        const std::vector<std::vector<cv::Point>> overallContours(1, overallContour);
        cv::drawContours(mask, overallContours, 0, cv::Scalar(255, 255, 255) /* white */, cv::FILLED);
  
        return mask;
    }
private:
    const std::string windowName;
    const cv::Mat& image;
    std::vector<std::vector<cv::Point>> contours;
};

void mouseCallback(int event, int x, int y, int flags, void* userData)
{
    Display *data = (Display*) userData;
    data->setCursor(cv::Point(x, y), event, "mouseClick");
}

int main(int argc, char* argv[])
{
    std::stringstream keysStream;
    {
        keysStream
            << commonCommandLineParserKeys()

            << "{ inputVideoFileName      |<none>| }"
            // outputImageFileName is part of common keys

            << "{ sampleFrameNumber       |    1 | }"

            << "{ interactive             | true | }"
            ;
        for (char c = 'A'; c <= 'Z'; ++c)
        {
            keysStream << "{ mousePoint_" << c << " |<none>| }";
        }
    }
    const cv::CommandLineParser parser(argc, argv, keysStream.str());

    if (parser.has("help"))
    {
        parser.printMessage();
        return 0;
    }

    cv::Mat image;
    {
        cv::VideoCapture videoReader(parser.get<cv::String>("inputVideoFileName"));
        size_t frameNumber = 0;
        while (videoReader.read(image))
        {
            frameNumber++;
            if (parser.get<int>("sampleFrameNumber") == frameNumber)
            {
                std::cout << "originalImage.size() = " << image.size() << std::endl;
                rotateAndCropImage(parser, image);
                std::cout << "image.size() = " << image.size() << std::endl;
                break;
            }
        }
    }
    writeImage(parser, image, 0 /* idx */);

    std::vector<cv::Point> mousePoints;
    for (char c = 'A'; c <= 'Z'; ++c)
    {
        std::stringstream param;
        param << "mousePoint_" << c;
        if (parser.has(param.str()))
        {
            const cv::Scalar s = parser.get<cv::Scalar>(param.str());
            mousePoints.push_back(cv::Point(s.val[0], s.val[1]));
        }
    }

    if (mousePoints.empty())
    {
        mousePoints.push_back(cv::Point(0, 0));
    }
    else
    {
        std::cout << "mousePoints[0] = " << mousePoints[0] << std::endl;
    }

    Display display("image", image, mousePoints[0], parser.get<bool>("interactive"));

    for (size_t mm = 1; mm < mousePoints.size(); ++mm)
    {
        std::cout << "mousePoints["<<mm<<"] = " << mousePoints[mm] << std::endl;
        cv::Mat annotatedImage = display.setCursor(mousePoints[mm]);
        for (size_t nn = 0; nn <= mm; ++nn)
        {
            putPointAndNumber(annotatedImage, nn + 1 /* pointNumber */, mousePoints[nn] /* point */);
        }
        writeImage(parser, annotatedImage, mm);
    }

    if (parser.get<bool>("interactive"))
    {
        std::cout << "press 'q' to stop" << std::endl;
        while ('q' != cv::waitKey(1));
    }

    if (1 < mousePoints.size())
    {
        writeImage(parser, display.getMask(), mousePoints.size() /* idx */);
    }

    return 0;
}

