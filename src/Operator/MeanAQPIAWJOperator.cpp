
#include <Operator/MeanAQPIAWJOperator.h>
#include <JoinAlgos/JoinAlgoTable.h>
bool OoOJoin::MeanAQPIAWJOperator::setConfig(INTELLI::ConfigMapPtr cfg) {
  if (!OoOJoin::AbstractOperator::setConfig(cfg)) {
    return false;
  }
  if (config->existDouble("aqpScale")) {
    aqpScale = config->getDouble("aqpScale");
  }
      OP_INFO("set aqp scale into=" + to_string(aqpScale));
  return true;
}
bool OoOJoin::MeanAQPIAWJOperator::start() {
  /**
  * @brief set watermark generator
  */
  wmGen = newPeriodicalWM();
  wmGen->setConfig(config);
  wmGen->syncTimeStruct(timeBaseStruct);
  /**
   * @note:
  */
  wmGen->creatWindow(0, windowLen);
 // wmGen->creatWindow(0, windowLen);
  /**
   * @brief set window
   */
  stateOfKeyTableR=newStateOfKeyHashTable(4096,4);
  stateOfKeyTableS=newStateOfKeyHashTable(4096,4);
  myWindow.setRange(0, windowLen);
  windowBound =windowLen;
  myWindow.init(sLen, rLen, 1);

  intermediateResult = 0;
  confirmedResult =0;
  lockedByWaterMark = false;
  return true;
}
void OoOJoin::MeanAQPIAWJOperator::conductComputation() {

}
void  OoOJoin::MeanAQPIAWJOperator::updateStateOfKeyS(MeanStateOfKeyPtr sk,TrackTuplePtr tp)
{
    double skewTp=tp->arrivalTime-tp->eventTime;
    double rateTp=tp->arrivalTime;
   // size_t prevUnarrived=0,currentUnarrived=0;
    rateTp=rateTp/(sk->arrivedTupleCnt+1);
    sk->arrivalSkew=(1-alphaArrivalSkew)*sk->arrivalSkew+alphaArrivalSkew*skewTp;
    sk->arrivalRate=(1-alphaArrivalRate)*sk->arrivalRate+alphaArrivalRate*rateTp;
    sk->sigmaArrivalSkew=(1-betaArrivalSkew)*sk->sigmaArrivalSkew+betaArrivalSkew*abs(sk->arrivalSkew-skewTp);
    //double futureTime=windowBound+sk->arrivalSkew-tp->arrivalTime;

   // double  futureTuples=futureTime*sk->arrivalRate;
    sk->arrivedTupleCnt++;
   /* prevUnarrived=sk->unArrivedTupleCnt;
    sk->unArrivedTupleCnt=futureTuples;
    currentUnarrived=  sk->unArrivedTupleCnt;
    AbstractStateOfKeyPtr skrf=stateOfKeyTableR->getByKey(tp->key);
    lastTimeS=tp->arrivalTime;
    if(skrf)
    {
      MeanStateOfKeyPtr skr=ImproveStateOfKeyTo(MeanStateOfKey,skrf);
      intermediateResult+=(skr->arrivedTupleCnt+skr->unArrivedTupleCnt)*currentUnarrived;
      intermediateResult-=(skr->arrivedTupleCnt+skr->unArrivedTupleCnt)*prevUnarrived;
      //cout<<"In S key " +to_string(tp->key)+"previous "+ to_string(prevUnarrived)+" not arrived, now"+ to_string(currentUnarrived)+" opposize="+ to_string(skr->unArrivedTupleCnt)+"\r\n";
      confirmedResult+=(skr->arrivedTupleCnt);
    }*/

}
void  OoOJoin::MeanAQPIAWJOperator::updateStateOfKeyR(MeanStateOfKeyPtr sk,TrackTuplePtr tp)
{
  double skewTp=tp->arrivalTime-tp->eventTime;
  double rateTp=tp->arrivalTime;
  rateTp=rateTp/(sk->arrivedTupleCnt+1);
  //size_t prevUnarrived=0,currentUnarrived=0;
  sk->arrivalSkew=(1-alphaArrivalSkew)*sk->arrivalSkew+alphaArrivalSkew*skewTp;
  sk->arrivalRate=(1-alphaArrivalRate)*sk->arrivalRate+alphaArrivalRate*rateTp;
  sk->sigmaArrivalSkew=(1-betaArrivalSkew)*sk->sigmaArrivalSkew+betaArrivalSkew*abs(sk->arrivalSkew-skewTp);
  sk->arrivedTupleCnt++;
  /*double futureTime=windowBound+sk->arrivalSkew-tp->arrivalTime;
  if(futureTime<0)
  {
    futureTime=0;
  }*/
  /*double  futureTuples=futureTime*sk->arrivalRate/1000.0;
  sk->arrivedTupleCnt++;
  prevUnarrived=sk->unArrivedTupleCnt;
  sk->unArrivedTupleCnt=futureTuples;
  currentUnarrived=  sk->unArrivedTupleCnt;
  AbstractStateOfKeyPtr skrf=stateOfKeyTableS->getByKey(tp->key);
  lastTimeR=tp->arrivalTime;
  size_t oppoSizeCnt=0;
  if(skrf)
  {
    MeanStateOfKeyPtr sks=ImproveStateOfKeyTo(MeanStateOfKey,skrf);
    oppoSizeCnt=sks->arrivedTupleCnt+sks->unArrivedTupleCnt;
    intermediateResult-=(sks->arrivedTupleCnt+sks->unArrivedTupleCnt)*prevUnarrived;
    intermediateResult+=(sks->arrivedTupleCnt+sks->unArrivedTupleCnt)*currentUnarrived;
    confirmedResult+=(sks->arrivedTupleCnt);
   // cout<<"IR="+ to_string(intermediateResult)+"\n";
   // cout<<"In R key " +to_string(tp->key)+"previous "+ to_string(prevUnarrived)+" not arrived, now"+ to_string(currentUnarrived)+" opposize="+ to_string(sks->unArrivedTupleCnt)+"\r\n";
  //   cout<<"key "+ to_string(tp->key)+ "arrive skew"+to_string( sk->arrivalSkew)+" future time ="+ to_string(futureTime)+"rate = "+ to_string(sk->arrivalRate)+" expect "+ to_string(futureTuples)+"\n";
  }*/
}
bool OoOJoin::MeanAQPIAWJOperator::stop() {
  /**
   */
  if (lockedByWaterMark) {
        WM_INFO("early terminate by watermark, already have results");
  }
  if (!lockedByWaterMark) {
        WM_INFO("No watermark encountered, compute now");
    //force to flush, if no watermark is given
    //conductComputation();
  }
  lazyComputeOfAQP();
  size_t rLen=myWindow.windowR.size();
  NPJTuplePtr *tr=myWindow.windowR.data();
  for(size_t i=0;i<rLen;i++)
  {
    tr[i]->processedTime=UtilityFunctions::timeLastUs(timeBaseStruct);
  }
  return true;
}
bool OoOJoin::MeanAQPIAWJOperator::feedTupleS(OoOJoin::TrackTuplePtr ts) {
  bool shouldGenWM,isInWindow;
  if (lockedByWaterMark) {
    return false;
  }
  isInWindow=myWindow.feedTupleS(ts);
  shouldGenWM = wmGen->reportTupleS(ts, 1);
  if (shouldGenWM) {
    lockedByWaterMark = true;
  //  return false;

  }
 // bool shouldGenWM;
  if(isInWindow)
  { MeanStateOfKeyPtr sk;
    AbstractStateOfKeyPtr skrf=stateOfKeyTableS->getByKey(ts->key);
    lastTimeS=ts->arrivalTime;
    if(skrf== nullptr) // this key does'nt exist
    {
      sk=newMeanStateOfKey();
      sk->key=ts->key;
      stateOfKeyTableS->insert(sk);
    }
    else
    {
      sk=ImproveStateOfKeyTo(MeanStateOfKey,skrf);
    }
    updateStateOfKeyS(sk,ts);
  }
  return true;
}

bool OoOJoin::MeanAQPIAWJOperator::feedTupleR(OoOJoin::TrackTuplePtr tr) {
  bool shouldGenWM,isInWindow;
  if (lockedByWaterMark) {
    return false;
  }
  isInWindow=myWindow.feedTupleR(tr);
  shouldGenWM = wmGen->reportTupleR(tr, 1);
  if (shouldGenWM) {
    lockedByWaterMark = true;
    //return false;

  }
  // bool shouldGenWM;
  if(isInWindow)
  { MeanStateOfKeyPtr sk;
    AbstractStateOfKeyPtr skrf=stateOfKeyTableR->getByKey(tr->key);
    lastTimeR=tr->arrivalTime;
    if(skrf== nullptr) // this key does'nt exist
    {
      sk=newMeanStateOfKey();
      sk->key=tr->key;
      stateOfKeyTableR->insert(sk);
    }
    else
    {
      sk=ImproveStateOfKeyTo(MeanStateOfKey,skrf);
    }
    updateStateOfKeyR(sk,tr);
  }
  return true;
}
size_t OoOJoin::MeanAQPIAWJOperator::getResult() {
  /*confirmedResult=0;
  AbstractStateOfKeyPtr probrPtr= nullptr;
  for (size_t i=0;i<stateOfKeyTableR->buckets.size();i++)
  {
    for(auto iter:stateOfKeyTableR->buckets[i])
    { if(iter!= nullptr) {
        MeanStateOfKeyPtr px = ImproveStateOfKeyTo(MeanStateOfKey, iter);
        probrPtr=stateOfKeyTableS->getByKey(px->key);
        if(probrPtr!= nullptr)
        {
          MeanStateOfKeyPtr py=ImproveStateOfKeyTo(MeanStateOfKey, probrPtr);
          confirmedResult+=px->arrivedTupleCnt*py->arrivedTupleCnt;
        }
      }
    }
  }*/
  size_t rLen=myWindow.windowR.size();
  NPJTuplePtr *tr=myWindow.windowR.data();
  for(size_t i=0;i<rLen;i++)
  {
   tr[i]->processedTime=UtilityFunctions::timeLastUs(timeBaseStruct);
  }
  return confirmedResult;
  // return confirmedResult;
}
void OoOJoin::MeanAQPIAWJOperator::lazyComputeOfAQP() {
  AbstractStateOfKeyPtr probrPtr= nullptr;
  intermediateResult=0;
  for (size_t i=0;i<stateOfKeyTableR->buckets.size();i++)
  {
    for(auto iter:stateOfKeyTableR->buckets[i])
    { if(iter!= nullptr) {
        MeanStateOfKeyPtr px = ImproveStateOfKeyTo(MeanStateOfKey, iter);
        probrPtr=stateOfKeyTableS->getByKey(px->key);
        if(probrPtr!= nullptr)
        {  double futureTimeS=windowBound+px->arrivalSkew-lastTimeS;

          double  futureTupleS=futureTimeS*px->arrivalRate/1000.0*aqpScale;
          size_t unarrivedS=futureTupleS;
          MeanStateOfKeyPtr py=ImproveStateOfKeyTo(MeanStateOfKey, probrPtr);

         double futureTimeR=windowBound+py->arrivalSkew-lastTimeR;
          double  futureTupleR=futureTimeR*py->arrivalRate/1000.0*aqpScale;
          size_t unarrivedR=futureTupleR;
          intermediateResult+=(px->arrivedTupleCnt+unarrivedS)*(py->arrivedTupleCnt+unarrivedR);
          /*cout<<"S: a="+ to_string(px->arrivedTupleCnt)+", u="+ to_string(futureTupleS)+"last seen at"+ to_string(lastTimeS)+"arrive rate"+ to_string(px->arrivalRate)+ ", future time="+ to_string(futureTimeS)+ ",sigma="+ to_string(px->sigmaArrivalSkew)+\
          ";R: a="+ to_string(py->arrivedTupleCnt)+", u="+ to_string(futureTupleR)+"future time ="+ to_string(futureTimeR)+"\n";*/
          confirmedResult+=(px->arrivedTupleCnt)*(py->arrivedTupleCnt);
        }
      }
    }
  }
}
size_t OoOJoin::MeanAQPIAWJOperator::getAQPResult() {


  return intermediateResult;
}

