//
// Created by tony on 06/12/22.
//

#include <Operator/OperatorTable.h>

namespace OoOJoin {
    OperatorTable::OperatorTable() {
        operatorMap["IAWJ"] = newIAWJOperator();
        operatorMap["MeanAQP"] = newMeanAQPIAWJOperator();
        operatorMap["IMA"] = newIMAIAWJOperator();
        operatorMap["MSWJ"] = newMSWJOperator();
    }
} // OoOJoin