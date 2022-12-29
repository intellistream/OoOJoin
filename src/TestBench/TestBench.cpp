//
// Created by tony on 23/11/22.
//

#include <TestBench/TestBench.h>
#include <Utils/UtilityFunctions.hpp>
#include <algorithm>
#include <Utils/IntelliLog.h>
using namespace INTELLI;
using namespace OoOJoin;
using namespace std;
void OoOJoin::TestBench::OoOSort(std::vector<TrackTuplePtr> &arr) {
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
void OoOJoin::TestBench::forceInOrder(std::vector<TrackTuplePtr> &arr) {
  size_t len = arr.size();
  size_t i;
  for (i = 0; i < len; i++) {
    arr[i]->arrivalTime = arr[i]->eventTime;
  }
}
void OoOJoin::TestBench::inOrderSort(std::vector<TrackTuplePtr> &arr) {
  size_t i, j;
  TrackTuplePtr temp;
  size_t len = arr.size();
  for (i = 0; i < len - 1; i++) {
    arr[i]->arrivalTime = arr[i]->eventTime;
    for (j = 0; j < len - 1 - i; j++)
      if (arr[j]->eventTime > arr[j + 1]->eventTime) {
        temp = arr[j];
        arr[j] = arr[j + 1];
        arr[j + 1] = temp;
      }
  }
}
void OoOJoin::TestBench::setDataSet(std::vector<TrackTuplePtr> _r, std::vector<TrackTuplePtr> _s) {
  rTuple = _r;
  sTuple = _s;
}
bool OoOJoin::TestBench::setOperator(OoOJoin::AbstractOperatorPtr op, ConfigMapPtr cfg) {
  testOp = op;
  opConfig = cfg;
  if (opConfig == nullptr) {
    return false;
  }
  if (opConfig->existU64("timeStep")) {
    timeStep = opConfig->getU64("timeStep");
    // TB_INFO("Feeding time step=" + to_string(timeStep) + "us");
  } else {
    //  TB_WARNNING("No setting of timeStep, use 1\n");
    timeStep = 1;
  }
  return true;
}
void OoOJoin::TestBench::inlineTest() {
  struct timeval timeStart;
  size_t testSize = (rTuple.size() > sTuple.size()) ? sTuple.size() : rTuple.size();
  size_t rPos = 0, sPos = 0;
  size_t tNow = 0;
  size_t tMaxS = sTuple[testSize - 1]->arrivalTime;
  size_t tMaxR = rTuple[testSize - 1]->arrivalTime;
  size_t tMax = (tMaxS > tMaxR) ? tMaxS : tMaxR;
  size_t tNextS = 0, tNextR = 0;
  /*for(size_t i=0;i<testSize;i++)
  {
   TB_INFO(sTuple[i]->toString());
  }*/
  testOp->setConfig(opConfig);
  gettimeofday(&timeStart, NULL);
  testOp->syncTimeStruct(timeStart);
  testOp->start();
  while (tNow < tMax) {
    tNow = UtilityFunctions::timeLastUs(timeStart);
    //INTELLI_INFO("T=" << tNow);
    while (tNow >= tNextS) {
      if (sPos <= testSize - 1) {
        testOp->feedTupleS(sTuple[sPos]);
        sPos++;
        if (sPos <= testSize - 1) {
          tNextS = sTuple[sPos]->arrivalTime;
        } else {
          tNextS = -1;
          //  INTELLI_INFO("NO MORE S");
          break;
        }

      }

    }
    //INTELLI_INFO("detect R");
    while (tNow >= tNextR) {
      if (rPos <= testSize - 1) {
        testOp->feedTupleR(rTuple[rPos]);
        //INTELLI_INFO("feed"+rTuple[rPos]->toString()+"at "+ to_string(tNow));
        rPos++;
        if (rPos <= testSize - 1) {
          tNextR = rTuple[rPos]->arrivalTime;

        } else {
          tNextR = -1;
          //  INTELLI_INFO("NO MORE R");
          break;
        }
      }
    }
    //usleep(20);
  }
  testOp->stop();
}
size_t OoOJoin::TestBench::OoOTest(bool additionalSort) {
  if (additionalSort) {
    OoOSort(rTuple);
    OoOSort(sTuple);
  }
  inlineTest();
  AQPResult = testOp->getAQPResult();
  return testOp->getResult();
}

size_t OoOJoin::TestBench::inOrderTest(bool additionalSort) {
  forceInOrder(rTuple);
  forceInOrder(sTuple);
  if (additionalSort) {
    inOrderSort(rTuple);
    inOrderSort(sTuple);
  }
  inlineTest();
  return testOp->getResult();
}
void OoOJoin::TestBench::logRTuples(bool skipZero) {
  //   TB_INFO("/***Printing the rTuples in the following***/");
  size_t rLen = rTuple.size();
  for (size_t i = 0; i < rLen; i++) {
    if (skipZero && rTuple[i]->processedTime == 0) {

    } else {
      //TB_INFO(rTuple[i]->toString());
    }

  }
  // TB_INFO("/***Done***/");
}
bool OoOJoin::TestBench::saveRTuplesToFile(std::string fname, bool skipZero) {
  ofstream of;
  of.open(fname);
  if (of.fail()) {
    return false;
  }
  of << "key,value,eventTime,arrivalTime,processedTime\n";
  size_t rLen = rTuple.size();
  for (size_t i = 0; i < rLen; i++) {
    if (skipZero && rTuple[i]->processedTime == 0) {

    } else {
      TrackTuplePtr tp = rTuple[i];
      string line = to_string(tp->key) + "," + to_string(tp->payload) + "," + to_string(tp->eventTime) + ","
          + to_string(tp->arrivalTime) + "," + to_string(tp->processedTime) + "\n";
      of << line;
    }

  }
  of.close();
  return true;
}
double OoOJoin::TestBench::getAvgLatency() {
  size_t rLen = rTuple.size();
  size_t nonZeroCnt = 0;
  double sum = 0;
  for (size_t i = 0; i < rLen; i++) {
    if (rTuple[i]->processedTime >= rTuple[i]->arrivalTime && rTuple[i]->processedTime != 0) {
      double temp = rTuple[i]->processedTime - rTuple[i]->arrivalTime;
      sum += temp;
      nonZeroCnt++;
    }
  }
  return sum / nonZeroCnt;
}
double OoOJoin::TestBench::getThroughput() {
  size_t rLen = rTuple.size();
  tsType minArrival = rTuple[0]->arrivalTime;
  tsType maxProcessed = 0;
  for (size_t i = 0; i < rLen; i++) {
    if (rTuple[i]->processedTime >= maxProcessed) {
      maxProcessed = rTuple[i]->processedTime;
    }
    if (rTuple[i]->arrivalTime <= minArrival) {
      minArrival = rTuple[i]->arrivalTime;
    }
  }
  double elapsedTime = (maxProcessed - minArrival);
  if (elapsedTime <= 0) {
    TB_WARNNING("No valid elapsed time, maybe there is no joined result?");
    return 0;
  }
  double thr = rLen;
  thr = thr * 1e6 / elapsedTime;
  return thr;
}
double OoOJoin::TestBench::getLatencyPercentage(double fraction) {
  size_t rLen = rTuple.size();
  size_t nonZeroCnt = 0;
  vector<tsType> validLatency;
  for (size_t i = 0; i < rLen; i++) {
    if (rTuple[i]->processedTime >= rTuple[i]->arrivalTime && rTuple[i]->processedTime != 0) {
      validLatency.push_back(rTuple[i]->processedTime - rTuple[i]->arrivalTime);
      nonZeroCnt++;
    }
  }
  if (nonZeroCnt == 0) {
    TB_WARNNING("No valid latency, maybe there is no joined result?");
    return 0;
  }
  std::sort(validLatency.begin(), validLatency.end());
  double t = nonZeroCnt;
  t = t * fraction;
  size_t idx = (size_t) t + 1;
  if (idx >= validLatency.size()) {
    idx = validLatency.size() - 1;
  }
  return validLatency[idx];

}
ConfigMapPtr OoOJoin::TestBench::getTimeBreakDown() {
  if (testOp != nullptr) {
    return testOp->getTimeBreakDown();
  }
  return nullptr;
}
std::vector<TrackTuplePtr> OoOJoin::TestBench::loadDataFromCsv(std::string fname,std::string separator , std::string newLine)
{
  std::vector<TrackTuplePtr> ru;
  ifstream ins;
  ins.open(fname);
  assert(separator.data());
  assert(newLine.data());
  if (ins.fail()) {
    return ru;
  }
  std::string readStr;
  /**
   * header is of << "key,value,eventTime,arrivalTime,processedTime\n";
   * should read the first line
   */
  std::getline(ins, readStr, newLine.data()[0]);
  vector<std::string> cols;
  // readStr.erase(readStr.size()-1);
  spilt(readStr, separator, cols);
  size_t idxKey=0,idxValue=0,idxEventTime=0,idxArrivalTime=0;
  size_t validCols=4;
  size_t validRows=0;
  if(cols.size()<validCols)
  {
    INTELLI_ERROR("Invalid csv header, return nothing");
    return ru;
  }

  /**
   * @note parase the first row here
   */
   for(size_t i=0;i<cols.size();i++)
   {
     if(cols[i]=="key")
     {
       idxKey=i;
     }
     if(cols[i]=="value")
     {
       idxValue=i;
     }
     if(cols[i]=="eventTime")
     {
       idxEventTime=i;
     }
     if(cols[i]=="arrivalTime"||cols[i]=="arriveTime")
     {
       idxArrivalTime=i;
     }
   }
   /**
    * @note deciding the valid rows
    */
  while (std::getline(ins, readStr, newLine.data()[0])){
    if(cols.size()>=validCols)
    {
      validRows++;
    }
  }
  cout<<"valid rows="<<validRows<<endl;
  ru= std::vector<TrackTuplePtr>(validRows);
  /**
   * re-open file
   */
  ins.close();
  ins.open(fname);
  /**
   * jump the header,
   */
  size_t loadRow=0;
  std::getline(ins, readStr, newLine.data()[0]);
  while (std::getline(ins, readStr, newLine.data()[0])){
    vector<std::string> cols2;
    // readStr.erase(readStr.size()-1);
    spilt(readStr, separator, cols2);
    if(cols.size()>=validCols)
    { keyType k;
      valueType v;
      tsType et,at;
      istringstream k_ss(cols2[idxKey]); k_ss>>k;
      istringstream v_ss(cols2[idxValue]); v_ss>>v;
      istringstream et_ss(cols2[idxEventTime]); et_ss>>et;
      istringstream at_ss(cols2[idxArrivalTime]); at_ss>>at;
      TrackTuplePtr tp=newTrackTuple(k,v,et,at);
      tp->processedTime=0;
      ru[loadRow]=tp;
      //cout<<ru[loadRow]->toString()<<endl;
      loadRow++;
    }
  }
  ins.close();
  return ru;
}
