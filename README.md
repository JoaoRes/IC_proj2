# IC_proj2

## Part A
### ex2
####**Compile:**
```console 
g++ -g -Wall x.cpp -o x
```
####**Execute:**
```console 
./x
```
### ex4
####**Compile:**
```console 
g++ -g -Wall x.cpp -o x
```
####**Execute:**
```console 
./x
```



## PartB
### Lossless
####**Compile:** 
```console 
g++ -g -Wall losslessCodec_audio.cpp -o lossless -lsndfile && ./remove.sh 
```
####**Execute:**
```console 
./lossless
```
### Lossy
####**Compile:**
```console 
g++ -g -Wall lossyCodec_audio.cpp -o lossless -lsndfile && ./remove.sh 
```
####**Execute:**
```console 
./lossy
```

