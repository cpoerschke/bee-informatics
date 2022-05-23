
#include "common.h"

#include <iostream>

#include <opencv2/opencv.hpp>

void putFrameNumber(cv::Mat& image, const size_t frameNumber,
                    const cv::Scalar& fontColor = cv::Scalar(255,255,255) /* white */,
                    const int thickness = 1, const int fontFace = cv::FONT_HERSHEY_SIMPLEX,
                    const int pixelHeight = 24)
{
    const double fontScale = cv::getFontScaleFromHeight(fontFace, pixelHeight, thickness);

    std::stringstream textStr;
    textStr << "frame " << frameNumber;

    cv::putText(image, textStr.str(),
                cv::Point(pixelHeight, 2*pixelHeight),
                fontFace, fontScale, fontColor, thickness);
}

class Display {
public:
    Display(const std::string& _windowName, const cv::CommandLineParser& parser, const int _radius = 0)
      : windowName(_windowName)
      , radius(_radius)
      , rectWidth(parser.get<int>("rectWidth"))
      , rectHeight(parser.get<int>("rectHeight"))
      , rectThickness(parser.get<int>("rectThickness"))
      , rects()
      , colors()
      , termCriteria()
    {
        colors.push_back(cv::Scalar(0, 0, 255)); // red
        colors.push_back(cv::Scalar(0, 255, 0)); // green
        colors.push_back(cv::Scalar(255, 0, 0)); // blue

        if (parser.has("termCriteria_COUNT"))
        {
            termCriteria.type |= cv::TermCriteria::COUNT;
            termCriteria.maxCount = parser.get<double>("termCriteria_COUNT");
        }

        if (parser.has("termCriteria_EPS"))
        {
            termCriteria.type |= cv::TermCriteria::EPS;
            termCriteria.epsilon = parser.get<double>("termCriteria_EPS");
        }
    }
    ~Display()
    {
        cv::destroyWindow(windowName);
    }
    void initRects(const cv::Point& cursorPoint)
    {
        rects.clear();
        for (size_t f = 1; f <= colors.size(); ++f)
        {
            const int width = f * rectWidth;
            const int height = f * rectHeight;
            rects.push_back(cv::Rect(cursorPoint.x - width/2, cursorPoint.y - height/2, width, height));
        }
    }
    cv::Mat show(const cv::Mat& image, const cv::Mat& backProject)
    {
        cv::Mat images = combine(image, backProject);
        cv::imshow(windowName, images);
        return images;
    }
private:
    cv::Mat combine(const cv::Mat& image, const cv::Mat& backProject)
    {
        cv::Mat imageCopy = image.clone();

        cv::Mat backProjectCopy;
        cv::cvtColor(backProject, backProjectCopy, cv::COLOR_GRAY2BGR);

        for (size_t idx = 0; idx < rects.size(); ++idx)
        {
            if (idx == 0 && 0 < radius)
            {
                const cv::Point center((rects[idx].tl().x + rects[idx].br().x) / 2,
                                       (rects[idx].tl().y + rects[idx].br().y) / 2);
                cv::circle(imageCopy, center, radius, cv::Scalar(0, 255, 255) /* yellow */, cv::FILLED);
            }

            cv::meanShift(backProject, rects[idx], termCriteria);

            if (idx == 0)
            {
                cv::rectangle(imageCopy, rects[idx].tl(), rects[idx].br(), colors[idx], rectThickness);
            }
            cv::rectangle(backProjectCopy, rects[idx].tl(), rects[idx].br(), colors[idx], rectThickness);
        }

        return concatImages(imageCopy, backProjectCopy);
    }
    static cv::Mat concatImages(const cv::Mat& imageA, const cv::Mat& imageB)
    {
        cv::Mat images;
        if (imageA.rows < imageA.cols)
        {
            cv::vconcat(imageA /* src1 */, imageB /* src2 */, images /* dst */);
        }
        else
        {
            cv::hconcat(imageA /* src1 */, imageB /* src2 */, images /* dst */);
        }
        return images;
    }
protected:
    const std::string windowName;
private:
    const int radius;
    const int rectWidth;
    const int rectHeight;
    const int rectThickness;
    std::vector<cv::Rect> rects;
    std::vector<cv::Scalar> colors;
    cv::TermCriteria termCriteria;
};

// forward declare
void mouseCallback(int event, int x, int y, int flags, void* userData);

class InteractiveDisplay : public Display {
public:
    InteractiveDisplay(const cv::CommandLineParser& parser,
                       const cv::Mat& _image, const cv::Mat& _backProject)
      : Display("move cursor and then click left mouse button once",
                parser,
                parser.get<int>("cursorRadius"))
      , image(_image)
      , backProject(_backProject)
      , cursorPoint()
      , gotMouseClick(false)
    {
        show(image, backProject);
        cv::setMouseCallback(windowName, mouseCallback, this /* userData */);
    }
    void callback(int event, const cv::Point& point)
    {
        if (!gotMouseClick)
        {
            initRects(point);
            show(image, backProject);

            if (cv::EVENT_LBUTTONDOWN == event)
            {
                cursorPoint = point;
                gotMouseClick = true;
            }
        }
    }
    const cv::Point& getMouseClickPoint() const
    {
        while (!gotMouseClick) cv::waitKey(1);
        return cursorPoint;
    }
private:
    const cv::Mat& image;
    const cv::Mat& backProject;
    cv::Point cursorPoint;
    bool gotMouseClick;
};

void mouseCallback(int event, int x, int y, int flags, void* userData)
{
    InteractiveDisplay *data = (InteractiveDisplay*) userData;
    data->callback(event, cv::Point(x, y));
}

int main(int argc, char* argv[])
{
    // compose command line parser
    std::stringstream keysStream;
    {
        keysStream
            << commonCommandLineParserKeys(true /* manipulateImage */, 8 /* defaultNumBins */)

            << "{ inputVideoFileName  |<none>| input video file name }"
            << "{ outputVideoFileName |      | output video file name (if empty then output is on-screen only) }"
            << "{ fourcc              |      | output video codec (if empty then the input video codec is used) }"

            << "{ minFrameNumber |         1 | first frame to use }"
            << "{ maxFrameNumber | 123456789 | last frame to use }"

            << "{ sampleFrameNumbers |<none>| (up to 4) frames }"

            << "{ cursorPoint |<none>| }"
            << "{ cursorRadius | 12 | }"

            << "{ rectWidth | 50 | }"
            << "{ rectHeight | 50 | }"

            << "{ rectThickness | 2 | }"

            << "{ termCriteria_COUNT | 100 | }"
            << "{ termCriteria_EPS   | 0.1 | }"
            ;
    }
    const cv::CommandLineParser parser(argc, argv, keysStream.str());

    if (parser.has("help"))
    {
        parser.printMessage();
        return 0;
    }

    const cv::String inputVideoFileName = parser.get<cv::String>("inputVideoFileName");
    const cv::String outputVideoFileName = parser.get<cv::String>("outputVideoFileName");

    const cv::String outputImageFileName = parser.get<cv::String>("outputImageFileName");
    std::set<int> outputImageFrameNumbers;
    if (parser.has("sampleFrameNumbers"))
    {
        const cv::Scalar sampleFrameNumbers = parser.get<cv::Scalar>("sampleFrameNumbers");
        outputImageFrameNumbers.insert(sampleFrameNumbers.val[0]);
        outputImageFrameNumbers.insert(sampleFrameNumbers.val[1]);
        outputImageFrameNumbers.insert(sampleFrameNumbers.val[2]);
        outputImageFrameNumbers.insert(sampleFrameNumbers.val[3]);
    }

    cv::VideoCapture videoReader(inputVideoFileName);
    cv::VideoWriter videoWriter;

    const cv::String fourcc = parser.get<cv::String>("fourcc");
    const int fourccInt = (fourcc.size() == 4
        ? cv::VideoWriter::fourcc(fourcc[0],fourcc[1],fourcc[2],fourcc[3])
        : (int) videoReader.get(cv::CAP_PROP_FOURCC));

    const int frameCount = (int) videoReader.get(cv::CAP_PROP_FRAME_COUNT);
    std::cout << "frameCount = " << frameCount << std::endl;

    const double framesPerSecond = (double) videoReader.get(cv::CAP_PROP_FPS);
    std::cout << "framesPerSecond = " << framesPerSecond << std::endl;

    const Histogram histogram(parser);

    Display display("images", parser);

    cv::Mat image;
    size_t frameNumber = 0;
    while (videoReader.read(image))
    {
        frameNumber++;

        rotateAndCropImage(parser, image);

        if (frameNumber < parser.get<int>("minFrameNumber")) continue;
        if (frameNumber > parser.get<int>("maxFrameNumber")) break;

        const cv::Mat backProject = histogram.backProject(image);

        putFrameNumber(image, frameNumber);

        if (frameNumber == parser.get<int>("minFrameNumber")) 
        {
            std::cout << "imageSize = " << image.size() << std::endl;

            cv::Point cursorPoint;
            if (parser.has("cursorPoint"))
            {
                cursorPoint = cv::Point(parser.get<cv::Scalar>("cursorPoint").val[0],
                                        parser.get<cv::Scalar>("cursorPoint").val[1]);
            }
            else
            {
                InteractiveDisplay interactiveDisplay(parser, image, backProject);
                cursorPoint = interactiveDisplay.getMouseClickPoint();
            }
            display.initRects(cursorPoint);

            std::cout << "cursorPoint = " << cursorPoint << std::endl;
        }

        const cv::Mat images = display.show(image, backProject);

        if (!outputVideoFileName.empty())
        {
             if (!videoWriter.isOpened())
             {
                 if (!videoWriter.open(outputVideoFileName, fourccInt, framesPerSecond, images.size()))
                 {
                    std::cerr << "could not open output file: " << outputVideoFileName << std::endl;
                    return 1;
                 }
            }
            videoWriter.write(images);
        }

        if (outputImageFrameNumbers.find(frameNumber) != outputImageFrameNumbers.end())
        {
            writeImage(parser, images, frameNumber);
        }

        if ('q' == (char) cv::waitKey(1))
        {
            std::cerr << "user asked to (q)uit" << std::endl;
            return 2;
        }
    }

    std::cout << "press any key to stop" << std::endl;
    cv::waitKey(10000); // wait up to 10 seconds

    return 0;
}

