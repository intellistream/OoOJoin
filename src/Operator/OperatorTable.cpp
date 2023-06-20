//
// Created by tony on 06/12/22.
//

#include <Operator/OperatorTable.h>
#include <Operator/MeanAQPIAWJOperator.h>
#include <Operator/IMAIAWJOperator.h>
#include <Operator/MSWJOperator.h>
#include <Operator/IAWJOperator.h>
namespace OoOJoin {
    OperatorTable::OperatorTable() {
        operatorMap["IAWJ"] = newIAWJOperator();
        operatorMap["IMA"] = newIMAIAWJOperator();
    }
} // OoOJoin
