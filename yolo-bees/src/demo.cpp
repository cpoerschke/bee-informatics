
#include <iostream>

#include <opencv2/opencv.hpp>

void putFrameNumber(cv::Mat& image, const size_t frameNumber,
                    const cv::Scalar& fontColor = cv::Scalar(0,0,0) /* black */,
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

std::vector<cv::Point> splitIntoSquares(const cv::Mat& image, int& squareSize)
{
    std::vector<cv::Point> offsets;

    squareSize = std::min(image.cols, image.rows);

    if (image.rows < image.cols) // landscape format
    {
        offsets.push_back(cv::Point((image.cols - squareSize) / 2, 0)); // middle square

        if (image.cols <= 2 * image.rows) // up to 2:1 ratio
        {
            offsets.push_back(cv::Point(0, 0)); // left square
            offsets.push_back(cv::Point(image.cols - squareSize, 0)); // right square
        }
    }
    else if (image.cols < image.rows) // portrait format
    {
        offsets.push_back(cv::Point(0, (image.rows - squareSize) / 2)); // middle square

        if (image.rows <= 2 * image.cols) // up to 1:2 ratio
        {
            offsets.push_back(cv::Point(0, 0)); // top square
            offsets.push_back(cv::Point(0, image.rows - squareSize)); // bottom square
        }
    }
    else // square format
    {
        offsets.push_back(cv::Point(0, 0)); // only square
    }

    return offsets;
}

std::vector<cv::Rect> lookOnce(cv::dnn::Net& net,
                               const double objectnessThreshold, const double classThreshold,
                               const int width, const int height)
{
    std::vector<cv::Rect> seen;
    static bool verbose = true;

    std::vector<cv::Mat> outputs;
    net.forward(outputs, net.getUnconnectedOutLayersNames());

    for (int ii = 0; ii < outputs.size(); ++ii)
    {
        const cv::Mat& output = outputs[ii];

        if (verbose) std::cout << "outputs["<<ii<<"].size() = " << outputs[ii].size() << std::endl;

        for (int jj = 0; jj < output.rows; ++jj)
        {
            const float* detection = output.ptr<float>(jj);

            const int centerX = (int)(detection[0] * width);
            const int centerY = (int)(detection[1] * height);
            const int w       = (int)(detection[2] * width);
            const int h       = (int)(detection[3] * height);
            const float objectnessPrediction = detection[4];

            if (objectnessPrediction < objectnessThreshold) continue;

            const cv::Mat scores = output.row(jj).colRange(5, output.cols);
            double maxVal;
            cv::Point maxLoc;
            cv::minMaxLoc(scores, NULL /* minVal */, &maxVal, NULL /* minLoc */, &maxLoc);

            if (maxVal < classThreshold) continue;

            seen.push_back(cv::Rect(centerX - w/2, centerY - h/2, w, h));

            std::cout << "classId=" << maxLoc.x
                      << " center=(x=" << centerX << ",y=" << centerY << ")"
                      << " size=(w=" << w << ",h=" << h << ")"
                      << " objectnessPrediction=" << objectnessPrediction
                      << " classPrediction=" << maxVal
                      << std::endl;
        }
    }

    verbose = false;
    return seen;
}

class ObjectDetector
{
private:
    const cv::CommandLineParser parser;
    cv::dnn::Net net;

public:
    ObjectDetector(const cv::CommandLineParser& _parser)
        : parser(_parser)
        , net(cv::dnn::readNetFromDarknet(parser.get<cv::String>("cfgFile"),
                                          parser.get<cv::String>("weightsFile")))
    {
    }

    void apply(const cv::Mat& image, cv::Mat& annotatedImage,
               const cv::Scalar& annotationColor = cv::Scalar(255,255,255) /* white */,
               const int thickness = 1)
    {
        int squareSize;
        for (const cv::Point& offset : splitIntoSquares(image, squareSize))
        {
            const cv::Mat imageSquare(image,
                                      cv::Range(offset.y, offset.y + squareSize) /* rowRange */,
                                      cv::Range(offset.x, offset.x + squareSize) /* colRange */);

            const cv::Mat blob = cv::dnn::blobFromImage(imageSquare,
                                                        parser.get<double>("scaleFactor"),
                                                        cv::Size(parser.get<int>("sizeWidth"),
                                                                 parser.get<int>("sizeHeight")),
                                                        parser.get<cv::Scalar>("mean"),
                                                        parser.get<bool>("swapRB"),
                                                        parser.get<bool>("crop"));
            net.setInput(blob);

            const std::vector<cv::Rect> seen = lookOnce(net,
                                                        parser.get<double>("objectnessThreshold"),
                                                        parser.get<double>("classThreshold"),
                                                        imageSquare.cols /* width */,
                                                        imageSquare.rows /* height */);
            for (const cv::Rect& rect : seen)
            {
                if (annotatedImage.empty())
                {
                    image.copyTo(annotatedImage);
                }
                cv::rectangle(annotatedImage,
                              cv::Point(offset.x + rect.x,
                                        offset.y + rect.y),
                              cv::Point(offset.x + rect.x + rect.width,
                                        offset.y + rect.y + rect.height),
                              annotationColor,
                              thickness);
            }
        }
    }
};

int main(int argc, char* argv[])
{
    // compose command line parser
    std::stringstream keysStream;
    {
        keysStream
            << "{ help                |      | print this message }"

            << "{ inputVideo          |<none>| input video file name }"
            << "{ outputVideo         |      | output video file name (if empty then output is on-screen only) }"
            << "{ fourcc              |      | output video codec (if empty then the input video codec is used) }"

            << "{ outputImage         | frame%d.jpg | output image file name (optional) }"
            << "{ sampleFrameNumbers  |             | (up to 4) frames for output image }"

            << "{ minFrameNumber |         1 | first frame to use }"
            << "{ maxFrameNumber | 123456789 | last frame to use }"

            << "{ cfgFile     | yolov3.cfg     | darknet config file to use }"
            << "{ weightsFile | yolov3.weights | darknet weights file to use }"

            << "{ objectnessThreshold | 0.5 | }"
            << "{ classThreshold      | 0.5 | }"

            << "{ scaleFactor | " << (1.0/255) << " | to pass to blobFromImage }"
            << "{ sizeWidth   | 416                 | size (width) to pass to blobFromImage }"
            << "{ sizeHeight  | 416                 | size (height) to pass to blobFromImage }"
            << "{ mean        | 0 0 0               | to pass to blobFromImage }"
            << "{ swapRB      | true                | to pass to blobFromImage }"
            << "{ crop        | false               | to pass to blobFromImage }"

            << "{ rotateCode  |                     | optional cv::RotateFlags value }"
            ;
    }
    const cv::CommandLineParser parser(argc, argv, keysStream.str());

    if (parser.has("help"))
    {
        parser.printMessage();
        return 0;
    }

    ObjectDetector objectDetector(parser);

    const cv::String inputVideoFileName = parser.get<cv::String>("inputVideo");
    const cv::String outputVideoFileName = parser.get<cv::String>("outputVideo");

    const cv::String outputImageFileName = parser.get<cv::String>("outputImage");
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

    const cv::Size frameSize((int) videoReader.get(cv::CAP_PROP_FRAME_WIDTH),
                             (int) videoReader.get(cv::CAP_PROP_FRAME_HEIGHT));
    std::cout << "frameSize = " << frameSize << std::endl;

    const int frameCount = (int) videoReader.get(cv::CAP_PROP_FRAME_COUNT);
    std::cout << "frameCount = " << frameCount << std::endl;

    const double framesPerSecond = (double) videoReader.get(cv::CAP_PROP_FPS);
    std::cout << "framesPerSecond = " << framesPerSecond << std::endl;

    cv::Mat image;
    size_t frameNumber = 0;
    while (videoReader.read(image))
    {
        frameNumber++;

        if (frameNumber < parser.get<int>("minFrameNumber")) continue;
        if (frameNumber > parser.get<int>("maxFrameNumber")) break;

        if (!parser.get<cv::String>("rotateCode").empty())
        {
            cv::rotate(image, image, parser.get<int>("rotateCode"));
        }

        cv::Mat annotatedImage;
        objectDetector.apply(image /* src */, annotatedImage /* dst */);

        if (!annotatedImage.empty())
        {
            std::cout << "frameNumber="<<frameNumber << std::endl;

            putFrameNumber(annotatedImage, frameNumber);
            cv::imshow("annotatedImage", annotatedImage);

            if (!outputVideoFileName.empty())
            {
                if (!videoWriter.isOpened())
                {
                    if (!videoWriter.open(outputVideoFileName, fourccInt, 1 /* framesPerSecond */,
                                          annotatedImage.size()))
                    {
                        std::cerr << "could not open output file: " << outputVideoFileName << std::endl;
                        return 1;
                    }
                }
                videoWriter.write(annotatedImage);
            }

            if (!outputImageFileName.empty() && outputImageFrameNumbers.find(frameNumber) != outputImageFrameNumbers.end())
            {
                cv::imwrite(cv::format(outputImageFileName.c_str(), frameNumber), annotatedImage);
            }
        }

        putFrameNumber(image, frameNumber);
        cv::imshow("image", image);

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

