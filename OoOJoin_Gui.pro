QT       += core gui
QT += printsupport
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++20
QMAKE_CXXFLAGS += -std=c++20
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    qcustomplot.cpp \
    src/Common/StateOfKey.cpp \
    src/Common/Tuples.cpp \
    src/Common/Window.cpp \
    src/JoinAlgos/AbstractJoinAlgo.cpp \
    src/JoinAlgos/JoinAlgoTable.cpp \
    src/JoinAlgos/NestedLoopJoin.cpp \
    src/Operator/AbstractOperator.cpp \
    src/Operator/IAWJOperator.cpp \
    src/Operator/IMAIAWJOperator.cpp \
    src/Operator/MeanAQPIAWJOperator.cpp \
    src/Operator/OperatorTable.cpp \
    src/TestBench/AbstractDataLoader.cpp \
    src/TestBench/DataLoaderTable.cpp \
    src/TestBench/RandomDataLoader.cpp \
    src/TestBench/TestBench.cpp \
    src/Utils/IntelliLog.cpp \
    src/Utils/UtilityFunctions.cpp \
    src/WaterMarker/AbstractWaterMarker.cpp \
    src/WaterMarker/ArrivalWM.cpp \
    src/WaterMarker/LatenessWM.cpp \
    src/WaterMarker/WMTable.cpp

HEADERS += \
    mainwindow.h \
    qcustomplot.h
FORMS += \
    mainwindow.ui
INCLUDEPATH += include/
# Default rules for deployment.
#QMAKE_WASM_PTHREAD_POOL_SIZE=navigator.hardwareConcurrency
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
RESOURCES += \
    font.qrc

