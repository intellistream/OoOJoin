//
// Created by tony on 27/06/23.
//

#include <Parallelization/KeyPartitionRunner.h>
#include <string>
void OoOJoin::KeyPartitionWorker::setDataSet(std::vector<TrackTuplePtr> _r, std::vector<TrackTuplePtr> _s) {
  rTuple = std::move(_r);
  sTuple = std::move(_s);
}
void OoOJoin::KeyPartitionWorker::setConfig(INTELLI::ConfigMapPtr _cfg) {
  OoOJoin::OperatorTable opt;
  cfg = _cfg;
  std::string opTag = cfg->tryString("operator", "IMA");
  testOp = opt.findOperator(opTag);
  if (testOp == nullptr) {
    INTELLI_ERROR("can not find the operator");
    exit(-1);
  }
  testOp->setConfig(_cfg);

}
void OoOJoin::KeyPartitionWorker::prepareRunning() {

  // testOp->start();
}
bool OoOJoin::KeyPartitionWorker::isMySTuple(OoOJoin::TrackTuplePtr ts) {
  if (ts->key % workers == myId) {
    return true;
  }
  return false;
}
bool OoOJoin::KeyPartitionWorker::isMyRTuple(OoOJoin::TrackTuplePtr tr) {
  if (tr->key % workers == myId) {
    return true;
  }
  return false;
}
void OoOJoin::KeyPartitionWorker::decentralizedMain() {

  INTELLI_INFO("Start tid" + to_string(myId) + " run");
  INTELLI::UtilityFunctions::bind2Core(myId);
  size_t testSize = (rTuple.size() > sTuple.size()) ? sTuple.size() : rTuple.size();
  size_t rPos = 0, sPos = 0;
  size_t tNow = 0;
  size_t tMaxS = sTuple[testSize - 1]->arrivalTime;
  size_t tMaxR = rTuple[testSize - 1]->arrivalTime;
  size_t tMax = (tMaxS > tMaxR) ? tMaxS : tMaxR;
  size_t tNextS = 0, tNextR = 0;
  gettimeofday(&tSystem, nullptr);
  testOp->syncTimeStruct(tSystem);
  testOp->start();
  //testOp->start();
  while (tNow < tMax) {
    tNow = UtilityFunctions::timeLastUs(tSystem);

    while (tNow >= tNextS) {
      if (sPos <= testSize - 1) {
        auto newTs = sTuple[sPos];
        if (isMySTuple(newTs)) {
          testOp->feedTupleS(newTs);

        }
        sPos++;
        if (sPos <= testSize - 1) {
          tNextS = sTuple[sPos]->arrivalTime;
        } else {
          tNextS = -1;
          break;
        }

      }

    }
    tNow = UtilityFunctions::timeLastUs(tSystem);
    while (tNow >= tNextR) {

      if (rPos <= testSize - 1) {
        auto newTr = rTuple[rPos];
        if (isMyRTuple(newTr))
          //if(rPos%workers==myId)
        {
          testOp->feedTupleR(newTr);
        }
        rPos++;
        if (rPos <= testSize - 1) {
          tNextR = rTuple[rPos]->arrivalTime;
        } else {
          tNextR = -1;
          break;
        }
      }
    }
    //usleep(20);
  }
  testOp->stop();
  INTELLI_INFO("End tid" + to_string(myId) + " run");
}

double OoOJoin::KeyPartitionWorker::getThroughput() {
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
    INTELLI_WARNING("No valid elapsed time, maybe there is no joined result?");
    return 0;
  }
  double thr = rLen;
  thr = thr * 1e6 / elapsedTime;
  return thr;
}

double OoOJoin::KeyPartitionWorker::getLatencyPercentage(double fraction) {
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
    INTELLI_WARNING("No valid latency, maybe there is no joined result?");
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
size_t OoOJoin::KeyPartitionWorker::getResult() {
  if (testOp == nullptr) {
    return 0;
  }
  return testOp->getResult();
}
size_t OoOJoin::KeyPartitionWorker::getAQPResult() {
  if (testOp == nullptr) {
    return 0;
  }
  return testOp->getAQPResult();
}
void OoOJoin::KeyPartitionWorker::inlineMain() {
  decentralizedMain();
}

void OoOJoin::KeyPartitionRunner::setConfig(INTELLI::ConfigMapPtr _cfg) {
  cfg = _cfg;
  threads = cfg->tryU64("threads", 1, true);
  myWorker = std::vector<OoOJoin::KeyPartitionWorkerPtr>(threads);
  for (uint64_t i = 0; i < threads; i++) {
    myWorker[i] = newKeyPartitionWorker();
    myWorker[i]->setConfig(cfg);
    myWorker[i]->setId(i, threads);
  }
}
void OoOJoin::KeyPartitionRunner::setDataSet(std::vector<TrackTuplePtr> _r, std::vector<TrackTuplePtr> _s) {
  for (uint64_t i = 0; i < threads; i++) {
    myWorker[i]->setDataSet(_r, _s);
  }
}
void OoOJoin::KeyPartitionRunner::runStreaming() {
  for (uint64_t i = 0; i < threads; i++) {
    myWorker[i]->prepareRunning();
  }
  for (uint64_t i = 0; i < threads; i++) {
    myWorker[i]->startThread();
  }
  INTELLI_INFO("Start " + to_string(threads) + " threads");
  for (uint64_t i = 0; i < threads; i++) {
    myWorker[i]->joinThread();
  }
  INTELLI_INFO("Finish " + to_string(threads) + " threads");
}
size_t OoOJoin::KeyPartitionRunner::getResult() {
  size_t ru = 0;
  for (uint64_t i = 0; i < threads; i++) {
    ru += myWorker[i]->getResult();
  }
  return ru;
}

size_t OoOJoin::KeyPartitionRunner::getAQPResult() {
  size_t ru = 0;
  for (uint64_t i = 0; i < threads; i++) {
    ru += myWorker[i]->getAQPResult();
  }
  return ru;
}

double OoOJoin::KeyPartitionRunner::getThroughput() {
  double ru = 0;
  for (uint64_t i = 0; i < threads; i++) {
    ru += myWorker[i]->getThroughput();
  }
  return ru / threads;
}
double OoOJoin::KeyPartitionRunner::getLatencyPercentage(double fraction) {
  double ru = 0;
  for (uint64_t i = 0; i < threads; i++) {
    ru += myWorker[i]->getLatencyPercentage(fraction);
  }
  return ru / threads;
}