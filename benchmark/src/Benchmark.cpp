// Copyright (C) 2021 by the IntelliStream team (https://github.com/intellistream)

/**
 * @brief This is the main entry point of the entire program.
 * We use this as the entry point for benchmarking.
 *
 */
#include <Utils/Logger.hpp>
#include <vector>
#include <OoOJoin.h>
using namespace std;
using namespace AllianceDB;
vector<tsType> genArrivalTime(vector<tsType> eventTime, vector<tsType> arrivalSkew) {
  vector<tsType> ru = vector<tsType>(eventTime.size());
  size_t len = (eventTime.size() > arrivalSkew.size()) ? arrivalSkew.size() : eventTime.size();
  for (size_t i = 0; i < len; i++) {
    ru[i] = eventTime[i] + arrivalSkew[i];
  }
  return ru;
}
template<class vecType=int>
void printVec(vector<vecType> in) {
  printf("hellow \r\n");
  size_t len = in.size();
  printf("len=%s", to_string(in.size()).data());
  for (size_t i = 0; i < len; i++) {
    printf("%s:%s\r\n", to_string(i).data(), to_string(in[i]).data());
  }
}
void bubble_sort(vector<AllianceDB::TrackTuplePtr> &arr) {
  size_t i, j;
  TrackTuplePtr temp;
  size_t len = arr.size();
  for (i = 0; i < len - 1; i++)
    for (j = 0; j < len - 1 - i; j++)
      if (arr[j]->arrivalTime > arr[j + 1]->arrivalTime) {
        temp = arr[j];
        arr[j] = arr[j + 1];
        arr[j + 1] = temp;
      }
}
vector<AllianceDB::TrackTuplePtr> genTuples(vector<keyType> keyS, vector<tsType> eventS, vector<tsType> arrivalS) {
  size_t len = keyS.size();
  vector<AllianceDB::TrackTuplePtr> ru = vector<AllianceDB::TrackTuplePtr>(len);
  for (size_t i = 0; i < len; i++) {
    ru[i] = newTrackTuple(keyS[i], 0, eventS[i], arrivalS[i]);
  }
  bubble_sort(ru);
  return ru;
}
void runTestBench()
{
  IAWJOperatorPtr iawj=newIAWJOperator();
  size_t testSize = 100;
  MicroDataSet ms(123456);
  //get config
  ConfigMapPtr cfg = newConfigMap();
  cfg->fromFile("config.csv");
  //gen the S
  vector<keyType> keyS = ms.genRandInt<keyType>(testSize, testSize / 10, 0);
  vector<tsType> eventS = ms.genSmoothTimeStamp<tsType>(testSize, 10, 10);
  vector<tsType> arrivalSkew = ms.genRandInt<tsType>(testSize, 10, 1);
  vector<tsType> arrivalS = genArrivalTime(eventS, arrivalSkew);
  vector<TrackTuplePtr> sTuple = genTuples(keyS, eventS, arrivalS);

  //gen R
  MicroDataSet mr = MicroDataSet(990);
  vector<keyType> keyR = mr.genRandInt<keyType>(testSize, testSize / 10, 0);
  vector<tsType> eventR = mr.genSmoothTimeStamp<tsType>(testSize, 10, 10);
  vector<tsType> arrivalRkew = mr.genRandInt<tsType>(testSize, 10, 1);
  vector<tsType> arrivalR = genArrivalTime(eventS, arrivalRkew);
  vector<TrackTuplePtr> rTuple = genTuples(keyR, eventR, arrivalR);
  // test bench
  TestBench tb;
  cfg->edit("rLen",(uint64_t)testSize);
  cfg->edit("sLen",(uint64_t)testSize);
  cfg->edit("windowLen",(uint64_t)10);
  tb.setOperator(iawj,cfg);
  tb.setDataSet(rTuple,sTuple);
  INTELLI_INFO("Joined." <<  tb.OoOTest(false) << "tuples");

}
int main(int argc, char **argv) {


  //Setup Logs.
  setLogLevel(getStringAsDebugLevel("LOG_TRACE"));

  setupLogging("benchmark.log", LOG_DEBUG);


  //Run the test here.
  INTELLI_INFO("Nothing to run." << argc << argv);

  //tempTest();
  runTestBench();
 // testConfig();
  //TuplePtrQueue tpq=
}

