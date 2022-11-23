//
// Created by tony on 23/11/22.
//

#include <TestBench/TestBench.h>
#include <Utils/UtilityFunctions.hpp>
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
  for (i = 0; i < len - 1; i++){
    arr[i]->arrivalTime=arr[i]->eventTime;
  }
}
void AllianceDB::TestBench::inOrderSort(std::vector<TrackTuplePtr> &arr) {
  size_t i, j;
  TrackTuplePtr temp;
  size_t len = arr.size();
  for (i = 0; i < len - 1; i++) {
    arr[i]->arrivalTime=arr[i]->eventTime;
    for (j = 0; j < len - 1 - i; j++)
      if (arr[j]->eventTime > arr[j + 1]->eventTime) {
        temp = arr[j];
        arr[j] = arr[j + 1];
        arr[j + 1] = temp;
      }
  }
}
void AllianceDB::TestBench::setDataSet(std::vector<TrackTuplePtr> _r, std::vector<TrackTuplePtr> _s) {
  rTuple=_r;
  sTuple=_s;
}
void AllianceDB::TestBench::setOperator(AllianceDB::AbstractOperatorPtr op,ConfigMapPtr cfg) {
  testOp=op;
  opConfig =cfg;
}
void AllianceDB::TestBench::inlineTest() {
  struct timeval timeStart;
  size_t testSize = (rTuple.size()>sTuple.size())?sTuple.size():rTuple.size();
  size_t rPos = 0, sPos = 0;
  size_t tNow = 0;
  size_t tMaxS = sTuple[testSize - 1]->arrivalTime;
  size_t tMaxR = rTuple[testSize - 1]->arrivalTime;
  size_t tMax = (tMaxS > tMaxR) ? tMaxS : tMaxR;
  size_t tNextS = 0, tNextR = 0;
  testOp->setConfig(opConfig);
  gettimeofday(&timeStart, NULL);
  testOp->syncTimeStruct(timeStart);
  testOp->start();
  while (tNow < tMax) {
    tNow = UtilityFunctions::timeLastUs(timeStart) / 40;
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
   if(additionalSort)
   {
     OoOSort(rTuple);
     OoOSort(sTuple);
   }
   inlineTest();
  return testOp->getResult();
}

size_t AllianceDB::TestBench::inOrderTest(bool additionalSort) {
  forceInOrder(rTuple);
  forceInOrder(sTuple);
  if(additionalSort)
  {
    inOrderSort(rTuple);
    inOrderSort(sTuple);
  }
  inlineTest();
  return testOp->getResult();
}