//
// Created by tony on 11/03/22.
//
#include <JoinAlgos/JoinAlgoTable.h>
#include "JoinAlgos/MSWJNestedLoopJoin.h"

using namespace OoOJoin;

JoinAlgoTable::JoinAlgoTable() {
//  NPJ npj;
    algos = {newAbstractJoinAlgo(), \
          newNestedLoopJoin(), \
          newNPJ(), \
         newNPJSingle(), \
         newMSWJNestedLoopJoin(), \
};
    algoMap["Null"] = newAbstractJoinAlgo();
    algoMap["NestedLoopJoin"] = newNestedLoopJoin();
    algoMap["NPJ"] = newNPJ();
    algoMap["NPJSingle"] = newNPJSingle();
    algoMap["MSWJNestedLoopJoin"] = newMSWJNestedLoopJoin();
    //cout<<algos[1]->getAlgoName()<<endl;
}
