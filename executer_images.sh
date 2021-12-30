echo "Compiling code"
g++ -Wall test_Lossless_Image.cpp -D _NOSHOW -D _TIMES -o test_Lossless_Image -std=c++11 `pkg-config --cflags --libs opencv4` 
echo "Code Compiled"
images="airplane baboon boat girl house lena monarch peppers tulips"
for i in $images
do
   echo "\nDoing iteration $i.ppm"
   ./remove.sh &&
   ./test_Lossless_Image imagens_PPM/$i.ppm $i.prop >/dev/null 2>&1 &&   
   echo "Execution $i Done" && 
   size_original=$(ls -l imagens_PPM/$i.ppm | awk '{print $5}') &&
   size_encoded=$(ls -l img.bin| awk '{print $5}') &&
   echo "writing file size properties" && 
   echo $size_original >> $i.prop &&
   echo $size_encoded >> $i.prop
done
