
baseUrl="http://localhost:8984/solr/bee-steps"
q="*:*"
posDeltaField="pos_delta_f"

echo "### Total number of data points"

curl --silent "${baseUrl}/select?rows=0&echoParams=none&q=${q}"

echo "### Number of data points without step lengths (e.g. first sighting after return to the hive)"

curl --silent "${baseUrl}/select?rows=0&echoParams=none&q=${q}&fq=${posDeltaField}:\[-1+TO+-1\]"

echo "### Number of data points with ambiguous step length (e.g. one bee in two places at the same time)"

curl --silent "${baseUrl}/select?rows=0&echoParams=none&q=${q}&fq=${posDeltaField}:\[*+TO+-2\]"

echo "### Interpretation: e.g. compared to the 1.9% incorrect IDs figure mentioned section 3.1 of https://doi.org/10.3389/frobt.2018.00035"

