#!/bin/bash
# check for required packages
if ! command -v gcc &> /dev/null
then
    echo "gcc not found, installing..."
    sudo apt-get install gcc -y
fi

if ! command -v g++ &> /dev/null
then
    echo "g++ not found, installing..."
    sudo apt-get install g++ -y
fi

# check for g++11 and install it if not found
if ! command -v g++-11 &> /dev/null
then
    echo "g++-11 not found, installing..."
    sudo apt-get install g++-11 -y
fi

if ! command -v python3 &> /dev/null
then
    echo "python3 not found, installing..."
    sudo apt-get install python3 -y
fi

if ! command -v pip3 &> /dev/null
then
    echo "pip3 not found, installing..."
    sudo apt-get install python3-pip -y
fi

# install doxygen
if ! command -v doxygen &> /dev/null
then
    echo "doxygen not found, installing..."
    sudo apt-get install doxygen -y
fi

if ! command -v graphviz &> /dev/null
then
    echo "graphviz not found, installing..."
    sudo apt-get install graphviz -y
fi

# install required Python packages

if [[ "$(lsb_release -rs)" == "22.04" ]]; then
    pip3 install torch==1.13.0 torchvision torchaudio --index-url https://download.pytorch.org/whl/cpu
else
    pip3 install torch torchvision torchaudio --index-url https://download.pytorch.org/whl/cpu
fi

pip3 install matplotlib pandas numpy


# build the project
if [ ! -d "build" ]
then
    mkdir build
fi
cd build
    cmake -DCMAKE_PREFIX_PATH=`python3 -c 'import torch;print(torch.utils.cmake_prefix_path)'` -DCMAKE_CXX_COMPILER=g++-11 ..
make -j24

# run the benchmark and copy the results
cd ../benchmark
if [ ! -d "figures" ]
then
    mkdir figures
fi
if [ ! -d "results" ]
then
    mkdir results
fi
cd scripts
./rerunAll.sh
cd ../../..

# copy the results and figures to the current directory
rm -rf results
rm -rf figures
cp -r build/benchmark/results .
cp -r build/benchmark/figures .

# generate documentation
doxygen Doxyfile