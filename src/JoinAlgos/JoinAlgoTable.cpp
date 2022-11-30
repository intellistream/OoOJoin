//
// Created by tony on 11/03/22.
//
#include <JoinAlgos/JoinAlgoTable.h>

using namespace AllianceDB;
JoinAlgoTable::JoinAlgoTable() {
//  NPJ npj;
  algos = {newAbstractJoinAlgo(), \
          newNestedLoopJoin(), \
          newNPJ(), \
         newNPJSingle()
  };
  algoMap["Null"] = newAbstractJoinAlgo();
  algoMap["NestedLoopJoin"] = newNestedLoopJoin();
  algoMap["NPJ"] = newNPJ();
  algoMap["NPJSingle"] = newNPJSingle();
  //cout<<algos[1]->getAlgoName()<<endl;
}
