
if [[ $# -lt 2 ]]
then

  echo "Usage: $0 <data file name> <collection name> [ <skipLines> [<commitWithin>]]"
  exit 1

else

  dataFileName="$1" ; shift
  if [[ ! -f "${dataFileName}" ]]
  then
    echo "Data file '${dataFileName}' does not exist."
    exit 2
  fi

  collectionName="$1" ; shift

  if [[ $# -gt 0 ]]
  then
    skipLines="$1" ; shift
  else
    skipLines=1 # skip the header line
  fi

  if [[ $# -gt 0 ]]
  then
    commitWithin="$1" ; shift
  else
    commitWithin=60000 # commit within one minute
  fi

  if [[ $# -gt 0 ]]
  then
    echo "Found unexpected extra arguments: ${*}"
  fi

fi

fieldNames="timestamp_dt,frame_id_s,track_id_s,x_pos_i,y_pos_i,orientation_f,bee_id_i,bee_id_confidence_f,cam_id_s,pos_srpt"

./solr-7.5.0/bin/post -c ${collectionName} -params "commitWithin=${commitWithin}&fieldnames=${fieldNames}&rowid=id&rowidOffset=${skipLines}&skipLines=${skipLines}" ${dataFileName}

