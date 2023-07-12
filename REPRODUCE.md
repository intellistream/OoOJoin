# How to Reproduce Our Experimental Results?

1. Everything will be automatically built and evaluated by calling the following command under **a sudo user** (You maybe asked to enter the password):

```shell
./onKeyReproduce.sh
```
This will also copy figures and results to `figures` and `results` folder at the root directory of this project for your convenience to find them.

## Prerequisite

1. System specification:

   | Component          | Description                            |
   | ------------------ |----------------------------------------|
   | Processor (w/o HT) | Intel(R) Xeon(R) Gold 6252 CPU, 24*2   |
   | L3 cache size      | 35.75MB                                |
   | Memory             | 384 GB                                 |
   | OS & Compiler      | Ubuntu22.04 (Physical or Docker) g++11 |

2. To make the scalability evaluation work as expected, please make sure that the core id 0~23 represent 24 physical cores at the same NUMA node (if there is some NUMA platform).

3. The torch installation command is designed for X64, and also works for ARM64, but may need some changes on other architectures like RISCV.

4. Docker scripts which only set up os and dependencies:
We provide docker scripts to install Ubuntu22.04 environment, please do the following to set it up
```shell
cd docker
./docker_est.sh
```
Upon seeing the docker command line, you may cd to home, proceed to manually clone this repo and then run the `onKeyReproduce.sh`.

5. Hands-free docker with precompiled AllianceDB_OoOJoin and its source:
We have merged the docker into AllianceDB docker repo, so far support X64 and ARM64(v8).
In the docker command line, you will find everything at /home/OoOJoin/projects/OoOJoin
You are also able to run `git pull` under /home/OoOJoin/projects/OoOJoin to make it updated
- X64 version: docker tag is `alliancedbadmin/alliancedb_ooo:latest`
A template setup command: 
```shell
cd docker/AllianceDB_X64
./docker_est.sh
```
- ARM64 version (EXPERIMENTAL): docker tag is `alliancedbadmin/alliancedb_ooo:arm64`
A template setup command: 
```shell
cd docker/AllianceDB_ARM64
./docker_est.sh
```
:warning: The ARM64 version can be built successfully, and basically able to run, but we have not yet
placed it into ARM64 servers with more than 24 core.

## Third-party Lib

Third-party Libs will be automatically installed in scripts, which contains:

1. CMake install, if CMake already installed, make sure the CMake version > 3.10.0.

```shell
sudo apt install -y cmake
```

2. Tex font rendering (Optional):

```shell
sudo apt install -y texlive-fonts-recommended texlive-fonts-extra
sudo apt install -y dvipng
sudo apt install -y font-manager
sudo apt install -y cm-super
```

3. Python3:

```shell
sudo apt install -y python3
sudo apt install -y python3-pip
pip3 install numpy
pip3 install matplotlib pandas
```

4. Libtorch:

```shell
pip3 install torch==1.13.0 torchvision torchaudio --index-url https://download.pytorch.org/whl/cpu
```

### Configurations
Please refer to the `config*.csv` under each child directory of `scripts` folder, 
such as scripts/sec6_2StockQ1
it assigns the operator, dataset and other system configurations

## Real World Datasets
Please refer to `benchmark/datasets`
We have already make the datasets fitted into readable csv format for both human and machine, with <key, value, event time, arrival time, processed time>.

Logistic: 

​	benchmark/datasets/logistics/tr.csv

​	benchmark/datasets/logistics/ts.csv

Retail:

​	benchmark/datasets/retail/tr.csv

​	benchmark/datasets/retail/ts.csv

Rovio:

​	benchmark/datasets/rovio/rovio.csv (For both R and S)

Stock: 

​	benchmark/datasets/stock/cj_1000ms_1tLowDelayData_short.csv

​	benchmark/datasets/stock/sb_1000ms_1tHighDelayData_short.csv

## Results

All results are in `build/benchmark/results/`.

All figures are in `build/benchmark/figures`.

You can run the following to restart an evaluation round, all old data will be deleted
```shell
cd build/benchmark/scripts
./rerunAll.sh
```