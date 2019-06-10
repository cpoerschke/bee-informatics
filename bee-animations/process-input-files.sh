
annotation_f()
{
  repo_url="https://github.com/cpoerschke/bee-informatics"

  data_title="BeesBook Recording Season 2015 Sample Release"
  data_url="https://doi.org/10.7303/syn11737848.1"

  echo "Generated by the ${1} script (${repo_url}) using part of the
'${data_title}' dataset (${data_url})."
}

image_annotation=$(annotation_f "draw-bees.py")
animation_annotation=$(annotation_f "animate-bees.py")

for step in 1 2 3 4 5
do
  ./draw-a-bee-step.py $step
done

./elliptical_bee_test.py --angle-degrees -30 --output-file-name elliptical-bee-minus-30-degrees.png
./elliptical_bee_test.py --angle-degrees   0 --output-file-name elliptical-bee-0-degrees.png
./elliptical_bee_test.py --angle-degrees +60 --output-file-name elliptical-bee-plus-60-degrees.png

./draw-bees.py --input-file-name ./2015-09-08_120000_097383.csv --output-file-name ./draw-a-frame.png --annotation "${image_annotation}"

./draw-bees.py --input-file-name ./2015-09-08_120000_097383.csv --output-file-name ./label-a-frame.png --label-all-bees --annotation "${image_annotation}"

./draw-bees.py --input-file-name ./2015-09-08_120000_097383.csv --output-file-name ./highlight-a-frame.png --spotlight-bee-ids 3258 229 263 259 296 344 3576 2811 --annotation "${image_annotation}"

mp4_file=./animate-a-bee.mp4
if [[ -e ${mp4_file} ]]
then
  echo "${mp4_file} already exists."
else
  ./animate-bees.py --input-file-name ./2015-09-08_1200_to_1214.csv --output-file-name ${mp4_file} \
  --title '"15 minutes with Bee 3258"' \
  --progress-report-interval 900 \
  --annotation "${animation_annotation}" --cam-id 1 \
  --focal-bee-id 3258
fi

mp4_file=./animate-bees-5x-trailer.mp4
if [[ -e ${mp4_file} ]]
then
  echo "${mp4_file} already exists."
else
  ./animate-bees.py --input-file-name ./2015-09-08_1200_to_1229.csv --output-file-name ${mp4_file} \
  --title '"Hello Bee 3258!" (5x time-lapse)' \
  --frame-timestamp-min "2015-09-08 12:03:30" \
  --frame-timestamp-max "2015-09-08 12:08:30" \
  --progress-report-interval 900 \
  --annotation "${animation_annotation}" --cam-id 1 \
  --focal-bee-id 3258 --focal-bee-meet-radius 166 \
  --frames-per-second 15
fi

mp4_file=./animate-bees-5x.mp4
if [[ -e ${mp4_file} ]]
then
  echo "${mp4_file} already exists."
else
  ./animate-bees.py --input-file-name ./2015-09-08_1200_to_1229.csv --output-file-name ${mp4_file} \
  --title '"Have you met Bee 3258?" (5x time-lapse)' \
  --progress-report-interval 900 \
  --annotation "${animation_annotation}" --cam-id 1 \
  --focal-bee-id 3258 --focal-bee-meet-radius 166 \
  --frames-per-second 15
fi

mp4_file=./animate-bees-10x.mp4
if [[ -e ${mp4_file} ]]
then
  echo "${mp4_file} already exists."
else
  ./animate-bees.py --input-file-name ./2015-09-08_1200_to_1259.csv --output-file-name ${mp4_file} \
  --title '"A hive hour after noon" (10x time-lapse)' \
  --progress-report-interval 900 \
  --annotation "${animation_annotation}" --dots-per-inch 150 --label-no-bees \
  --focal-bee-id 3258 --focal-bee-meet-radius 166 \
  --frames-per-second 30
fi
