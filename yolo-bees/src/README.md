
## Building OpenCV from source code

Based on the [OpenCV installation overview](https://docs.opencv.org/4.5.5/d0/d3d/tutorial_general_install.html) tutorial steps.

```
# download and unpack the release archive from https://github.com/opencv/opencv/releases
tar xf 4.5.5.tar.gz

# create a separate build directory
mkdir opencv-4.5.5-build
cd opencv-4.5.5-build

# build it
cmake -DCMAKE_BUILD_TYPE=Release ../opencv-4.5.5

make
```

## Building the demo program

Based on the [Using OpenCV with gcc and CMake](https://docs.opencv.org/4.5.5/db/df5/tutorial_linux_gcc_cmake.html) tutorial steps.

```
# ensure the OpenCV_DIR environment variable is set
export OpenCV_DIR=/Users/cpoerschke/opencv-4.5.5-build

# build the demo in the src sub-directory
cd /Users/cpoerschke/bee-informatics/yolo-bees/src

cmake .

make
```

## Downloading the YOLO model files

Based on https://github.com/opencv/opencv/blob/4.5.5/samples/dnn/models.yml#L24 and/or https://pjreddie.com/darknet/yolo/ information:

* https://pjreddie.com/media/files/yolov3.cfg
* https://pjreddie.com/media/files/yolov3.weights

## Running the demo program

```
# to check usage details
./yolo-bees-demo --help

# determine variable parameters
stem=VID_20220101_145249

minFrameNumber=3109
maxFrameNumber=3399

sampleFrameNumbers="3139 3197 3205 3351"

fourcc=HEVC # try H264 or HEVC

# run the demo
./yolo-bees-demo \
  --inputVideo=$stem.mp4 \
  --minFrameNumber=$minFrameNumber --maxFrameNumber=$maxFrameNumber \
  --outputVideo=../$stem-yolo-$minFrameNumber-to-$maxFrameNumber.mp4 \
  --sampleFrameNumbers="$sampleFrameNumbers" \
  --outputImage=../$stem-yolo-'%d'.jpg \
  --fourcc=$fourcc
```

