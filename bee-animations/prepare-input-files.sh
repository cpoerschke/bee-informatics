
full_data_file=../data-sets/data_sample_release.csv
if [[ ! -e ${full_data_file} ]]
then
  echo "Full data file not found:"
  ls -l ${full_data_file}
  exit 1
fi

csv_file=./2015-09-08_1200_to_1259.csv
if [[ ! -e ${csv_file} ]]
then
                        head -1 ${full_data_file} >  ${csv_file}
  grep ^"2015-09-08 12:"        ${full_data_file} >> ${csv_file}
fi
wc -l ${csv_file}

csv_file=./2015-09-08_1200_to_1229.csv
if [[ ! -e ${csv_file} ]]
then
                      head -1 ${full_data_file} >  ${csv_file}
  grep ^"2015-09-08 12:[012]" ${full_data_file} >> ${csv_file}
fi
wc -l ${csv_file}

csv_file=./2015-09-08_1200_to_1214.csv
if [[ ! -e ${csv_file} ]]
then
                         head -1 ${full_data_file} >  ${csv_file}
  grep ^"2015-09-08 12:0"        ${full_data_file} >> ${csv_file}
  grep ^"2015-09-08 12:1[01234]" ${full_data_file} >> ${csv_file}
fi
wc -l ${csv_file}

csv_file=./2015-09-08_120000_097383.csv
if [[ ! -e ${csv_file} ]]
then
                             head -1 ${full_data_file} >  ${csv_file}
  grep ^"2015-09-08 12:00:00.097383" ${full_data_file} >> ${csv_file}
fi
wc -l ${csv_file}

