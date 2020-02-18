# imgcvt
This is simply just a png to raw image converter for MCU GUI.


## compile
On ubuntu:
```
git clone https://github.com/singds/imgcvt.git
cd imgcvt
make
```
After that you have a `../imgcvt/build` directory with the executable **imgcvt**.


## usage
To see all the available options run:
```
imgcvt -h
```
This is an example supposing you have an `example.png` image inside your current directory:
```
imgcvt example.png -frgba8888 -o example.raw
```
This produces the file `example.raw`.