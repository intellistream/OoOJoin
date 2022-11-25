//
// Created by tony on 23/11/22.
//

#include <TestBench/TestBench.h>
#include <Utils/UtilityFunctions.hpp>
#include <algorithm>
using namespace INTELLI;
using namespace AllianceDB;
using namespace std;
void AllianceDB::TestBench::OoOSort(std::vector<TrackTuplePtr> &arr) {
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
void AllianceDB::TestBench::forceInOrder(std::vector<TrackTuplePtr> &arr) {
  size_t len = arr.size();
  size_t i;
  for (i = 0; i < len; i++) {
    arr[i]->arrivalTime = arr[i]->eventTime;
  }
}
void AllianceDB::TestBench::inOrderSort(std::vector<TrackTuplePtr> &arr) {
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
void AllianceDB::TestBench::setDataSet(std::vector<TrackTuplePtr> _r, std::vector<TrackTuplePtr> _s) {
  rTuple = _r;
  sTuple = _s;
}
bool AllianceDB::TestBench::setOperator(AllianceDB::AbstractOperatorPtr op, ConfigMapPtr cfg) {
  testOp = op;
  opConfig = cfg;
  if (opConfig == nullptr) {
    return false;
  }
  if (opConfig->existU64("timeStep")) {
    timeStep = opConfig->getU64("timeStep");
    TB_INFO("Feeding time step="+ to_string(timeStep)+"us");
  }
  else
  {
    TB_WARNNING("No setting of timeStep, use 1\n");
    timeStep=1;
  }
  return true;
}
void AllianceDB::TestBench::inlineTest() {
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
    tNow = UtilityFunctions::timeLastUs(timeStart)/timeStep ;
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
size_t AllianceDB::TestBench::OoOTest(bool additionalSort) {
  if (additionalSort) {
    OoOSort(rTuple);
    OoOSort(sTuple);
  }
  inlineTest();
  return testOp->getResult();
}

size_t AllianceDB::TestBench::inOrderTest(bool additionalSort) {
  forceInOrder(rTuple);
  forceInOrder(sTuple);
  if (additionalSort) {
    inOrderSort(rTuple);
    inOrderSort(sTuple);
  }
  inlineTest();
  return testOp->getResult();
}
void AllianceDB::TestBench::logRTuples(bool skipZero) {
  TB_INFO("/***Printing the rTuples in the following***/");
  size_t rLen=rTuple.size();
  for(size_t i=0;i<rLen;i++)
  {
    if(skipZero&&rTuple[i]->processedTime==0)
    {

    }
    else
    {
      TB_INFO(rTuple[i]->toString());
    }

  }
  TB_INFO("/***Done***/");
}
bool AllianceDB::TestBench::saveRTuplesToFile(std::string fname, bool skipZero) {
  ofstream of;
  of.open(fname);
  if (of.fail()) {
    return false;
  }
  of<<"key,value,eventTime,arrivalTime,processedTime\n";
  size_t rLen=rTuple.size();
  for(size_t i=0;i<rLen;i++)
  {
    if(skipZero&&rTuple[i]->processedTime==0)
    {

    }
    else
    {
      TrackTuplePtr tp=rTuple[i];
      string line= to_string(tp->key)+","+to_string(tp->payload)+","+to_string(tp->eventTime)+","+to_string(tp->arrivalTime)+","+to_string(tp->processedTime)+"\n";
      of<<line;
    }

  }
  of.close();
  return true;
}
double AllianceDB::TestBench::getAvgLatency() {
  size_t rLen=rTuple.size();
  size_t nonZeroCnt=0;
  double sum=0;
  for(size_t i=0;i<rLen;i++)
  {
    if(rTuple[i]->processedTime>=rTuple[i]->arrivalTime&&rTuple[i]->processedTime!=0) {
      double temp = rTuple[i]->processedTime - rTuple[i]->arrivalTime;
      sum += temp;
      nonZeroCnt++;
    }
  }
  return sum*timeStep/nonZeroCnt;
}
double AllianceDB::TestBench::getThroughput() {
  size_t rLen=rTuple.size();
  tsType minArrival=rTuple[0]->arrivalTime;
  tsType maxProcessed=0;
  for(size_t i=0;i<rLen;i++)
  {
    if(rTuple[i]->processedTime>=maxProcessed) {
      maxProcessed=rTuple[i]->processedTime;
    }
    if(rTuple[i]->arrivalTime<=minArrival) {
      minArrival=rTuple[i]->arrivalTime;
    }
  }
  double elapsedTime=(maxProcessed-minArrival)*timeStep;
  double thr=rLen;
  thr=thr*1e6/elapsedTime;
  return thr;
}
double AllianceDB::TestBench::getLatencyPercentage(double fraction) {
  size_t rLen=rTuple.size();
  size_t nonZeroCnt=0;
  vector<tsType>validLatency;
  for(size_t i=0;i<rLen;i++)
  {
    if(rTuple[i]->processedTime>=rTuple[i]->arrivalTime&&rTuple[i]->processedTime!=0) {
      validLatency.push_back(rTuple[i]->processedTime-rTuple[i]->arrivalTime);
      nonZeroCnt++;
    }
  }
  std::sort(validLatency.begin(),validLatency.end());
  double t=nonZeroCnt;
  t=t*fraction;
  size_t idx=(size_t)t+1;
  if(idx>=validLatency.size())
  {
    idx=validLatency.size()-1;
  }
  return validLatency[idx]*timeStep;

}