
if [[ $# -ne 2 ]]
then
  echo "Usage: $0 <input file name> <output file name>"
  exit 1
else
  inputFileName="$1"
  outputFileName="$2"
fi

if [[ ! -f "${inputFileName}" ]]
then
  echo "Input file '${inputFileName}' does not exist."
  exit 2
fi

if [[ -f "${outputFileName}" ]]
then
  echo "Output file '${outputFileName}' already exists."
  exit 3
fi

echo "Input file: ${inputFileName}"

# Input column 1 like
#   "2018-07-06 05:43:21.012345+00"
# is to become like
#   "2018-07-06T05:43:21.012345Z"
# in output column 1.

# Input columns 4 and 5 of 9
#   "1234,5678"
# are to become one joint
#   "0.1234 0.5678"
# output column 10.

sed 's/^\(....\-..\-..\) \(..:..:...*\)+00,\(.*\)$/\1T\2Z,\3/g' ${inputFileName} | sed 's/^\(....\-..\-..T..:..:...*Z,.*,.*\),\(.*\),\(.*\),\(.*,.*,.*,.*\)$/\1,\2,\3,\4,posX0000\2 posY0000\3/g' | sed 's/^\(.*\),posX.*\(....\) posY.*\(....\)$/\1,0\.\2 0\.\3/g' > ${outputFileName}

echo "Output file: ${outputFileName}"

exit 0

