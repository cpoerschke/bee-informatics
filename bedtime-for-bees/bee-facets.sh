
csvFile="./bee-facets.csv"
script="./bee-facets.py"

echo "
### The ${script} script divides a day (e.g. 2015-09-08) into 'day' (6am to 6pm) and 'night' (before 6am and after 6pm)
### and then calculates day and night step speed percentiles as well as day/night percentile ratios for individual bees.
"


if [[ ! -e "${csvFile}" ]]
then
  echo "### Extracting ${csvFile} data -- this likely will take a few minutes."
  ${script}
else
  echo "### Found existing ${csvFile} data file."
fi


#
# Checking data file header assumptions
#

actual_headers="$(head -1 ./bee-facets.csv)"

expected_headers="bee.count,bee.val,bee.p25_pos_delta_f,bee.p50_pos_delta_f,bee.p75_pos_delta_f,day.count,day.count/night.count,day.p25_pos_delta_f,day.p25_pos_delta_f/night.p25_pos_delta_f,day.p50_pos_delta_f,day.p50_pos_delta_f/night.p50_pos_delta_f,day.p75_pos_delta_f,day.p75_pos_delta_f/night.p75_pos_delta_f,night.count,night.count/day.count,night.p25_pos_delta_f,night.p25_pos_delta_f/day.p25_pos_delta_f,night.p50_pos_delta_f,night.p50_pos_delta_f/day.p50_pos_delta_f,night.p75_pos_delta_f,night.p75_pos_delta_f/day.p75_pos_delta_f"

if [[ "${actual_headers}" != "${expected_headers}" ]]
then
  echo "
*** ERROR: actual headers differ from expected headers:

actual_headers   = ${actual_headers}

vs.

expected_headers = ${expected_headers}
"
  exit 0
fi


echo "
### Note the metrics available for each bee:
"
head -1 ./bee-facets.csv


echo "
### Observe selected metrics for Bee 104:
"
cut -d',' -f1,2,13 ./bee-facets.csv | head -1
cut -d',' -f1,2,13 ./bee-facets.csv | grep ",104,"


echo "
### Identify other bees for whom there are similar or more data points:
"
cut -d',' -f1,2,13 ./bee-facets.csv | head -1
cut -d',' -f1,2,13 ./bee-facets.csv | grep ^"[1-9][0-9][0-9][0-9][0-9][0-9]" | grep -v ^"1[0-5][0-9][0-9][0-9][0-9]," | grep -v ","$ | sort --numeric-sort --field-separator=, --key=1


echo "
### Identify similar bees (with a high day/night 75th percentile ratio):
"
cut -d',' -f1,2,13 ./bee-facets.csv | head -1
cut -d',' -f1,2,13 ./bee-facets.csv | grep ^"[1-9][0-9][0-9][0-9][0-9][0-9]" | grep -v ^"1[0-5][0-9][0-9][0-9][0-9]," | grep -v ","$ | sort --numeric-sort --field-separator=, --key=3 --key=2 | tail


echo "
### Identify dissimilar bees (with a low day/night 75th percentile ratio):
"
cut -d',' -f1,2,13 ./bee-facets.csv | head -1
cut -d',' -f1,2,13 ./bee-facets.csv | grep ^"[1-9][0-9][0-9][0-9][0-9][0-9]" | grep -v ^"1[0-5][0-9][0-9][0-9][0-9]," | grep -v ","$ | sort --numeric-sort --field-separator=, --key=3 --key=2 | head


echo "
### Compose 'similar bees' query:
"
beeList=
for beeId in `
cut -d',' -f1,2,13 ./bee-facets.csv | grep ^"[1-9][0-9][0-9][0-9][0-9][0-9]" | grep -v ^"1[0-5][0-9][0-9][0-9][0-9]," | grep -v ","$ | sort --numeric-sort --field-separator=, --key=3 --key=2 | tail | cut -d',' -f2 | sort -n`
do
  if [[ -n "${beeList}" ]]
  then
    beeList="${beeList}+"
  fi
  beeList="${beeList}${beeId}"
done

Q_LIKE_BEE_104="${beeList}"

echo "Q_LIKE_BEE_104='${Q_LIKE_BEE_104}'"


echo "
### Compose 'dissimilar bees' query:
"
beeList=
for beeId in `
cut -d',' -f1,2,13 ./bee-facets.csv | grep ^"[1-9][0-9][0-9][0-9][0-9][0-9]" | grep -v ^"1[0-5][0-9][0-9][0-9][0-9]," | grep -v ","$ | sort --numeric-sort --field-separator=, --key=3 --key=2 | head | cut -d',' -f2 | sort -n`
do
  if [[ -n "${beeList}" ]]
  then
    beeList="${beeList}+"
  fi
  beeList="${beeList}${beeId}"
done

Q_UNLIKE_BEE_104="${beeList}"

echo "Q_UNLIKE_BEE_104='${Q_UNLIKE_BEE_104}'"


echo "
### Export commands for use by the bee-step-stats-sum.sh script:
"

echo "export Q_LIKE_BEE_104='${Q_LIKE_BEE_104}'"
echo "export Q_UNLIKE_BEE_104='${Q_UNLIKE_BEE_104}'"

echo

