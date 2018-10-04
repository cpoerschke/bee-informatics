
echo "Searching for bees home at midnight on the 8th"

curl --silent 'http://localhost:8983/solr/bee-hive/select?q=*:*&rows=1234567890&fl=bee_id_i&sort=bee_id_i+asc&group=true&group.field=bee_id_i&sort=bee_id_i+asc&group.ngroups=true&fq=timestamp_dt:\[2015-09-08T00:00:00Z%2D1MINUTE+TO+2015-09-08T00:00:00Z%2B1MINUTE\]&group.main=true&wt=csv' | sort > midnight8.log

echo "Searching for bees home at midday   on the 8th"

curl --silent 'http://localhost:8983/solr/bee-hive/select?q=*:*&rows=1234567890&fl=bee_id_i&sort=bee_id_i+asc&group=true&group.field=bee_id_i&sort=bee_id_i+asc&group.ngroups=true&fq=timestamp_dt:\[2015-09-08T12:00:00Z%2D10MINUTE+TO+2015-09-08T12:00:00Z%2B10MINUTE\]&group.main=true&wt=csv' | sort > midday8.log

echo "Searching for bees home at midnight on the 9th"

curl --silent 'http://localhost:8983/solr/bee-hive/select?q=*:*&rows=1234567890&fl=bee_id_i&sort=bee_id_i+asc&group=true&group.field=bee_id_i&sort=bee_id_i+asc&group.ngroups=true&fq=timestamp_dt:\[2015-09-09T00:00:00Z%2D1MINUTE+TO+2015-09-09T00:00:00Z%2B1MINUTE\]&group.main=true&wt=csv' | sort > midnight9.log

echo "Calculating bees home at midnight on the 8th and 9th"

# col1: home at midnight on 8th
# col2: home at midnight on 9th
# col3: home at midnight on 8th and 9th
comm -12 midnight8.log midnight9.log > midnight.log

echo "Calculating bees home at midnight but away at midday"

# col1: home at midnight
# col2: home at noon
# col3: home at midnight and at noon
# wanted: home at midnight, not home at noon
comm -23 midnight.log midday8.log > not-midday8.log

echo "Displaying number of bees per file"

wc -l midnight8.log
wc -l midday8.log
wc -l midnight9.log
wc -l midnight.log
wc -l not-midday8.log

divider=

echo "=== Bees home at midnight but away at midday ==="

for bee_id_i in `cat not-midday8.log`
do
  if [[ -n "${divider}" ]]
  then
    echo "${divider}"
  else
    divider="------------------------------------------------"
  fi
  echo "bee: ${bee_id_i}"
  curl --silent 'http://localhost:8983/solr/bee-hive/select?wt=csv&rows=1&q=bee_id_i:'${bee_id_i}'&fq=timestamp_dt:\[*+TO+2015-09-08T12:00:00Z\]&fl=prev_seen:timestamp_dt&sort=timestamp_dt+desc'
  curl --silent 'http://localhost:8983/solr/bee-hive/select?wt=csv&rows=1&q=bee_id_i:'${bee_id_i}'&fq=timestamp_dt:\[2015-09-08T12:00:00Z+TO+*\]&fl=next_seen:timestamp_dt&sort=timestamp_dt+asc'
done

echo "================================================"

