# OoOJoin [![CMake](https://github.com/intellistream/ModernCPlusProjectTemplate/actions/workflows/cmake.yml/badge.svg?branch=main)](https://github.com/intellistream/ModernCPlusProjectTemplate/actions/workflows/cmake.yml)

Out of order join operator

## Requires G++11

For ubuntu older than 21.10, run following first

```shell
sudo add-apt-repository 'deb http://mirrors.kernel.org/ubuntu jammy main universe'
sudo apt-get update
```

Then, install the default gcc/g++ of ubuntu22.04

```shell
sudo apt-get install gcc g++ cmake python3 python3-pip
```

## How to build
### Create a docker for clion
Please refer to the .md in docker folder
The docker has minmal gcc/g++/cmake and libtorch, which is good enough.
### Build in shell

```shell

mkdir build && cd build
make 
```

## Code Structure

- benchmark -- application code to use the generated shared library
- cmake -- cmake configuration files
- docsrc -- the source pictures to build docs
- include -- all the header files
- scripts -- python scripts to run automatic tests
- src -- corresponding source files, will generate a shared library
- test -- test code based on google test

## Local generation of the documents

You can also re-generate them locally, if you have the doxygen and graphviz. Following are how to install them in ubuntu
21.10/22.04

```shell
sudo apt-get install doxygen
sudo apt-get install graphviz
```

Then, you can do

```shell
mkdir doc
doxygen Doxyfile
```

to get the documents in doc/html folder, and start at index.html

## Automatic scripts

### Python3 dependencies

```shell
pip3 install matplotlib pandas numpy
```

1. Place the scripts folder in the same folder of benchmerk program
2. Create a folder named ``results`` and another named ``figures``
3. cd the folder you want to test
4. run

```shell
  python3 drawTogether.py
```

5. You will get the figures in ``figures``, and csv results in ``results``

## Known issues

1. If you use Torch with cuda, the nvcc will refuse to work as it doesn't support c++20 yet. Therefore, we disabled the
   global requirement check of C++ 20, and only leave an "-std=c++20" option for g++. This will be fixed after nvidia
   can support c++20 in cuda.
2. The pytorch headers also conflict with gtest, so gtest is now excluded, we switch the test framework to header-only
   catch2