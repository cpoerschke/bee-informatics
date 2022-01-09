
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
cd /Users/cpoerschke/bee-informatics/bg-sub/src

cmake .

make
```

## Running the demo program

```
# to check usage details
./background-subtractor-demo --help

# determine variable parameters
stem=VID_20220101_145249

minFrameNumber=3109
maxFrameNumber=3399

sampleFrameNumber=3208

fourcc=H264 # try H264 or HEVC
squareSize=720 # shrink to limit file sizes

# run the demo for both algorithms
for algo in knn mog2
do
  ./background-subtractor-demo \
    --inputVideo=$stem.mp4 --${algo} \
    --minFrameNumber=$minFrameNumber --maxFrameNumber=$maxFrameNumber \
    --outputVideo=../$stem-$algo-background-$minFrameNumber-to-$maxFrameNumber.mp4 \
    --sampleFrameNumber=$sampleFrameNumber \
    --outputImage=../$stem-$algo-background-$sampleFrameNumber.jpg \
    --fourcc=$fourcc --squareSize=$squareSize
done
```

