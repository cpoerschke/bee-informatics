
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

## Building the demo programs

Based on the [Using OpenCV with gcc and CMake](https://docs.opencv.org/4.5.5/db/df5/tutorial_linux_gcc_cmake.html) tutorial steps.

```
# ensure the OpenCV_DIR environment variable is set
export OpenCV_DIR=/Users/cpoerschke/opencv-4.5.5-build

# build the demos in the src sub-directory
cd /Users/cpoerschke/bee-informatics/shades-of-bee/src

cmake .

make
```

## Running the demo programs

```
./run-demos.sh
```

