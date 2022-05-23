
stem=VID_20220409_142117

./intelligentScissors-demo \
  --inputVideoFileName=./$stem.mp4

rotateCode=0
topLeftBottomRight="0 0 1088 1088"
topLeftBottomRight="0 0 544 544"
topLeftBottomRight="0 136 544 680"

./intelligentScissors-demo \
  --inputVideoFileName=./$stem.mp4 --rotateCode=$rotateCode --topLeftBottomRight="$topLeftBottomRight" \
  --outputImageFileName=../${stem}-scissors.jpg --interactive=false

topLeftBottomRight="0 272 544 544"

./intelligentScissors-demo \
  --inputVideoFileName=./$stem.mp4 --rotateCode=$rotateCode --topLeftBottomRight="$topLeftBottomRight"

mousePoint_A="275 115"
mousePoint_B="456 138"
mousePoint_C="472 195"
mousePoint_D="392 180"
mousePoint_E="299 236"
mousePoint_F="387 166"
mousePoint_G="329 152"
mousePoint_H="$mousePoint_A"

./intelligentScissors-demo \
  --inputVideoFileName=./$stem.mp4 --rotateCode=$rotateCode --topLeftBottomRight="$topLeftBottomRight" \
  --mousePoint_A="$mousePoint_A" --mousePoint_B="$mousePoint_B" \
  --mousePoint_C="$mousePoint_C" --mousePoint_D="$mousePoint_D" \
  --mousePoint_E="$mousePoint_E" --mousePoint_F="$mousePoint_F" \
  --mousePoint_G="$mousePoint_G" --mousePoint_H="$mousePoint_H" \
  --outputImageFileName=../${stem}-scissors-'click%d'.jpg --interactive=false

histogramImage=../${stem}-scissors-click0.jpg
histogramImageMask=../${stem}-scissors-click8.jpg

./calcHist-demo --numBins=2 --histogramNorm=100 --blockSize=50 --outputImageFileName=../eightBlocks.jpg

./calcHist-demo --numBins=2 --histogramNorm=100 --histogramImage=${histogramImage}
./calcHist-demo --numBins=2 --histogramNorm=100 --histogramImage=${histogramImage} --histogramImageMask=${histogramImageMask}

./calcHist-demo --numBins=3 --histogramNorm=100 --histogramImage=${histogramImage}
./calcHist-demo --numBins=3 --histogramNorm=100 --histogramImage=${histogramImage} --histogramImageMask=${histogramImageMask}

./calcBackProject-demo \
  --inputImageFileName=../${stem}-scissors.jpg --outputImageFileName=../${stem}-shades.jpg \
  --histogramImage=${histogramImage} --histogramImageMask=${histogramImageMask}

topLeftBottomRight="0 200 1088 744"
topLeftBottomRight="0 200  544 744"

cursorPoint="353 197"

minFrameNumber=1
maxFrameNumber=580
fourcc=HEVC # try H264 or HEVC

sampleFrameNumbers="1 492 555 580"

./meanShift-demo \
  --inputVideoFileName=./$stem.mp4 --rotateCode=$rotateCode --topLeftBottomRight="$topLeftBottomRight" \
  --minFrameNumber=$minFrameNumber --maxFrameNumber=$maxFrameNumber \
  --outputVideoFileName=../${stem}-shades-${fourcc}.mp4 --fourcc=$fourcc \
  --sampleFrameNumbers="$sampleFrameNumbers" --outputImageFileName=../${stem}-shades-'frame%d'.jpg \
  --histogramImage=${histogramImage} --histogramImageMask=${histogramImageMask} --cursorPoint="${cursorPoint}"

