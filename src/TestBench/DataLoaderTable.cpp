//
// Created by tony on 29/12/22.
//

#include <TestBench/DataLoaderTable.h>
//#include <TestBench/FileDataLoader.h>
//#include <TestBench/ZipfDataLoader.h>
#include <TestBench/RandomDataLoader.h>
OoOJoin::DataLoaderTable::DataLoaderTable() {
    loaderMap["random"] = newRandomDataLoader();
}
