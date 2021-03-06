echo "Compiling code"
g++ -g -Wall -D _TIMES  lossyCodec_audio.cpp -o lossy -lsndfile
echo "Code Compiled"
for i in $(seq 1 7)
do
   echo "Doing iteration $i"
   ./remove.sh &&
   ./lossy AudioSampleFiles/sample0$i.wav  $i.prop >/dev/null 2>&1 &&   
   echo "Execution $i Done" && 
   size_original=$(ls -l AudioSampleFiles/sample0$i.wav | awk '{print $5}') &&
   size_encoded=$(ls -l lossy_encoded_output.txt | awk '{print $5}') &&
   echo "writing file size properties" && 
   echo $size_original >> $i.prop &&
   echo $size_encoded >> $i.prop
done