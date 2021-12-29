# IC_proj2

## Part A
### ex2
####**Compile:**
```console 
g++ -g x.cpp -o x
```
####**Execute:**
```console 
./x
```
### ex4
####**Compile:**
```console 
g++ -g x.cpp -o x
```
####**Execute:**
```console 
./x
```



## PartB
### Lossless
####**Compile:** 
```console 
g++ -g losslessCodec_audio.cpp -o lossless -lsndfile && ./remove.sh 
```
####**Execute:**
You need to pass as argument to the program the path to the wav song, in our case to test we 
are going to use `AudioSampleFiles/sample01.wav` as wav file
```console 
./lossless AudioSampleFiles/sample01.wav
```
### Lossy
####**Compile:**
```console 
g++ -g lossyCodec_audio.cpp -o lossless -lsndfile && ./remove.sh 
```
####**Execute:**
```console 
./lossy
```

#Compilation arguments

* `-Wall` Show all warnings and errors
* `-D _DEBUG` Show Debug Prints
* `-D _TIMES` Calculate execution times

