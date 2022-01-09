
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

void putText2(cv::Mat& image, const cv::String& text1, const cv::String& text2,
              const cv::Scalar& fontColor = cv::Scalar(255,255,255) /* white */,
              const int thickness = 3, const int fontFace = cv::FONT_HERSHEY_SIMPLEX)
{
    for (int pixelHeight = image.rows; pixelHeight > 0; pixelHeight--)
    {
        const double fontScale = cv::getFontScaleFromHeight(fontFace, pixelHeight, thickness);

        int unusedBaseLine;
        const cv::Size text1Size = cv::getTextSize(text1, fontFace, fontScale, thickness, &unusedBaseLine);
        const cv::Size text2Size = cv::getTextSize(text2, fontFace, fontScale, thickness, &unusedBaseLine);

        if (std::max(text1Size.width, text2Size.width) <= 0.8 * image.cols)
        {
            cv::putText(image, text1,
                        cv::Point((image.cols - text1Size.width)/2, 1*(image.rows + text1Size.height)/3),
                        fontFace, fontScale, fontColor, thickness);
            cv::putText(image, text2,
                        cv::Point((image.cols - text2Size.width)/2, 2*(image.rows + text2Size.height)/3),
                        fontFace, fontScale, fontColor, thickness);
            return;
        }
    }
    std::cerr << "could not fit text(s) to " << image.size() << " image" << std::endl;
}

cv::Mat display(const cv::Mat& imageA, const cv::Mat& imageB, const int squareSize)
{
    cv::Mat images;

    if (imageA.rows <= imageA.cols)
    {
        cv::vconcat(imageA /* src1 */, imageB /* src2 */, images /* dst */);
    }
    else
    {
        cv::hconcat(imageA /* src1 */, imageB /* src2 */, images /* dst */);
    }

    if (squareSize != 0)
    {
        if (images.rows < images.cols)
        {
            cv::Mat padImage((images.cols - images.rows) / 2, images.cols, images.type(), cv::Scalar(255,255,255) /* white */);
            cv::vconcat(padImage /* src1 */, images /* src2 */, images /* dst */);
            cv::vconcat(images /* src1 */, padImage /* src2 */, images /* dst */);
        }

        if (images.cols < images.rows)
        {
            cv::Mat padImage(images.rows, (images.rows - images.cols) / 2, images.type(), cv::Scalar(255,255,255) /* white */);
            cv::hconcat(padImage /* src1 */, images /* src2 */, images /* dst */);
            cv::hconcat(images /* src1 */, padImage /* src2 */, images /* dst */);
        }

        if (images.cols != squareSize || images.rows != squareSize)
        {
            cv::Mat resizedImage;
            const int interpolation = (2 * squareSize < images.cols + images.rows ? cv::INTER_AREA : cv::INTER_CUBIC);
            cv::resize(images, resizedImage, cv::Size(squareSize, squareSize), 0 /* fx */, 0 /* fy */, interpolation);
            images = resizedImage;
        }
    }

    cv::imshow("images", images);

    return images;
}

int main(int argc, char* argv[])
{
    // compose command line parser
    std::stringstream keysStream;
    {
        const cv::Ptr<cv::BackgroundSubtractorKNN> knn = cv::createBackgroundSubtractorKNN();
        const cv::Ptr<cv::BackgroundSubtractorMOG2> mog2 = cv::createBackgroundSubtractorMOG2();

        const int history = (knn->getHistory() + mog2->getHistory()) / 2;
        const bool detectShadows = knn->getDetectShadows() && mog2->getDetectShadows();

        keysStream
            << "{ help                |      | print this message }"

            << "{ inputVideo          |<none>| input video file name }"
            << "{ outputVideo         |      | output video file name (if empty then output is on-screen only) }"
            << "{ fourcc              |      | output video codec (if empty then the input video codec is used) }"

            << "{ outputImage         |      | output image file name (optional) }"
            << "{ sampleFrameNumber   |      | frame for output image (if empty then maxFrameNumber is used) }"

            << "{ minFrameNumber |         1 | first frame to use }"
            << "{ maxFrameNumber | 123456789 | last frame to use }"

            << "{ knn            | false | use BackgroundSubtractorKNN class }"
            << "{ mog2           | false | use BackgroundSubtractorMOG2 class }"

            << "{ history        | " << history << " | integer }"
            << "{ detectShadows  | " << std::boolalpha << detectShadows << " | true or false }"
            << "{ dist2Threshold | " << knn->getDist2Threshold() << " | double (BackgroundSubtractorKNN only) }"
            << "{ varThreshold   | " << mog2->getVarThreshold()  << " | double (BackgroundSubtractorMOG2 only) }"

            << "{ rotateCode     |   | optional cv::RotateFlags value }"
            << "{ squareSize     | 0 | if non-zero the resized square size of the output image }"
            ;
    }
    const cv::CommandLineParser parser(argc, argv, keysStream.str());

    if (parser.has("help"))
    {
        parser.printMessage();
        return 0;
    }

    std::string backgroundSubtractorName;
    std::stringstream backgroundSubtractorParameters;
    cv::Ptr<cv::BackgroundSubtractor> backgroundSubtractor;

    if (parser.get<bool>("knn"))
    {
        cv::Ptr<cv::BackgroundSubtractorKNN> backgroundSubtractorKNN = cv::createBackgroundSubtractorKNN(
            parser.get<int>("history"),
            parser.get<double>("dist2Threshold"),
            parser.get<bool>("detectShadows"));
        backgroundSubtractorName = "cv::BackgroundSubtractorKNN";
        backgroundSubtractorParameters
            << "history=" << backgroundSubtractorKNN->getHistory()
            << " dist2Threshold=" << backgroundSubtractorKNN->getDist2Threshold()
            << " detectShadows=" << std::boolalpha << backgroundSubtractorKNN->getDetectShadows();
        backgroundSubtractor = backgroundSubtractorKNN;
    }

    if (parser.get<bool>("mog2"))
    {
        cv::Ptr<cv::BackgroundSubtractorMOG2> backgroundSubtractorMOG2 = cv::createBackgroundSubtractorMOG2(
            parser.get<int>("history"),
            parser.get<double>("varThreshold"),
            parser.get<bool>("detectShadows"));
        backgroundSubtractorName = "cv::BackgroundSubtractorMOG2";
        backgroundSubtractorParameters
            << "history=" << backgroundSubtractorMOG2->getHistory()
            << " varThreshold=" << backgroundSubtractorMOG2->getVarThreshold()
            << " detectShadows=" << std::boolalpha << backgroundSubtractorMOG2->getDetectShadows();
        backgroundSubtractor = backgroundSubtractorMOG2;
    }

    const cv::String inputVideoFileName = parser.get<cv::String>("inputVideo");
    const cv::String outputVideoFileName = parser.get<cv::String>("outputVideo");

    const cv::String outputImageFileName = parser.get<cv::String>("outputImage");
    const size_t outputImageFrameNumber = (parser.has("sampleFrameNumber")
        ? parser.get<int>("sampleFrameNumber") : parser.get<int>("maxFrameNumber"));

    if (!parser.check())
    {
        parser.printErrors();
        return 1;
    }

    // exactly one of KNN and MOG2 should have been specified
    if (parser.get<bool>("knn") == parser.get<bool>("mog2"))
    {
        parser.printMessage();
        return 2;
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

    cv::Mat image;
    size_t frameNumber = 0;
    while (videoReader.read(image))
    {
        frameNumber++;

        if (!parser.get<cv::String>("rotateCode").empty())
        {
            cv::rotate(image, image, parser.get<int>("rotateCode"));
        }

        cv::Mat foregroundMask;
        backgroundSubtractor->apply(image, foregroundMask);

        if (frameNumber < parser.get<int>("minFrameNumber")) continue;
        if (frameNumber > parser.get<int>("maxFrameNumber")) break;

        cv::Mat foregroundImage;
        image.copyTo(foregroundImage, foregroundMask);

        putFrameNumber(image, frameNumber);

        const cv::Mat images = display(image, foregroundImage, parser.get<int>("squareSize"));
        if (!outputVideoFileName.empty())
        {
             if (!videoWriter.isOpened())
             {
                 if (!videoWriter.open(outputVideoFileName, fourccInt, framesPerSecond, images.size()))
                 {
                    std::cerr << "could not open output file: " << outputVideoFileName << std::endl;
                    return 3;
                 }
                 std::cout << "imageSize = " << image.size() << std::endl;
            }
            videoWriter.write(images);
        }

        if (!outputImageFileName.empty() && outputImageFrameNumber == frameNumber)
        {
            cv::imwrite(outputImageFileName, images);
        }

        if ('q' == (char) cv::waitKey(1))
        {
            std::cerr << "user asked to (q)uit" << std::endl;
            return 4;
        }
    }

    backgroundSubtractor->getBackgroundImage(image);
    putText2(image,
             backgroundSubtractorName + " background image",
             backgroundSubtractorParameters.str());

    cv::Mat lastImage(image.size(), image.type(), cv::Scalar(0,0,0) /* black */);
    putText2(lastImage,
             "Background subtraction with bees",
             "https://github.com/cpoerschke/bee-informatics");

    const cv::Mat images = display(image, lastImage, parser.get<int>("squareSize"));
    if (!outputVideoFileName.empty())
    {
        videoWriter.write(images);
    }

    std::cout << "press any key to stop" << std::endl;
    cv::waitKey(10000); // wait up to 10 seconds

    return 0;
}

