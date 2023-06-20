//
// Created by tony on 11/03/22.
//
#include <JoinAlgos/JoinAlgoTable.h>

using namespace OoOJoin;

JoinAlgoTable::JoinAlgoTable() {
//  NPJ npj;
    algos = {newAbstractJoinAlgo(), \
          newNestedLoopJoin(), \
    };
    algoMap["Null"] = newAbstractJoinAlgo();
    algoMap["NestedLoopJoin"] = newNestedLoopJoin();
    //cout<<algos[1]->getAlgoName()<<endl;
}
