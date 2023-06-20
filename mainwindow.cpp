#include "mainwindow.h"
#include "ui_mainwindow.h"
using namespace OoOJoin;
 ConfigMapPtr cfg;
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->progressBar->setMaximum(100);
    ui->progressBar->setMinimum(0);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked()
{
    ui->label->setText("Running");
    testOoO();
    ui->label->setText("Done");
}
void MainWindow::webLog(std::string log)
{
    QString q_str = QString::fromStdString(log);
    QString rawStr=ui->textEdit->toPlainText();
    rawStr+="\r\n";
    rawStr+=q_str;
    ui->textEdit->setText(rawStr);
}
void MainWindow::readOperatorTag()
{  int idx=ui->comboBox->currentIndex();
    if(idx==0)
    {
        g_opTag="IAWJ";
    }
    else
    {
        g_opTag="IMA";
    }
}
void MainWindow::testPlot()
{

    QVector<double> x(101), y(101); //初始化向量x和y
    for (int i=0; i<101; ++i)
    {
      x[i] = i/50.0 - 1; // x范围[-1,1]
      y[i] = x[i]*x[i]; // y=x*x
    }
    ui->myPlot->addGraph();//添加数据曲线（一个图像可以有多个数据曲线）

    // graph(0);可以获取某个数据曲线（按添加先后排序）
    // setData();为数据曲线关联数据
    ui->myPlot->graph(0)->setData(x, y);
    ui->myPlot->graph(0)->setName("第一个示例");// 设置图例名称
    // 为坐标轴添加标签
    ui->myPlot->xAxis->setLabel("x");
    ui->myPlot->yAxis->setLabel("y");
    // 设置坐标轴的范围，以看到所有数据
    ui->myPlot->xAxis->setRange(-1, 1);
    ui->myPlot->yAxis->setRange(0, 1);
    ui->myPlot->legend->setVisible(true); // 显示图例
    // 重画图像
    ui->myPlot->replot();

}
void MainWindow::reportOoO(uint64_t windowLen, uint64_t tc,uint64_t *lat, double *error)
{
    OperatorTablePtr opTable = newOperatorTable();
    readOperatorTag();
    cfg= newConfigMap();
     cfg->edit("timeStepUs", (uint64_t)200);
    cfg->edit("windowLenMs", (uint64_t)windowLen);
     cfg->edit("watermarkTimeMs", (uint64_t)tc);
     cfg->edit("operator", (std::string)g_opTag);
      cfg->edit("eventRateKTps", (uint64_t)10);
     //finish init phase
    string operatorTag;
    string loaderTag = "random";
    operatorTag = cfg->tryString("operator", "IAWJ");

    //size_t testSize = 0;
    size_t OoORu = 0, realRu = 0;
       //load global configs
    tsType windowLenMs, timeStepUs, maxArrivalSkewMs;
    //uint64_t keyRange;
    windowLenMs = cfg->tryU64("windowLenMs", 10, true);
    timeStepUs = cfg->tryU64("timeStepUs", 40, true);
        //watermarkTimeMs = cfg->tryU64("watermarkTimeMs", 10,true);

    maxArrivalSkewMs = cfg->tryU64("maxArrivalSkewMs", 100 / 2);

    operatorTag = cfg->tryString("operator", "IAWJ");
     loaderTag = cfg->tryString("dataLoader", "random");

     cfg->edit("windowLen", (uint64_t) windowLenMs * 1000);
         //cfg->edit("watermarkTime", (uint64_t) watermarkTimeMs * 1000);
      cfg->edit("timeStep", (uint64_t) timeStepUs);

         TestBench tb, tbOoO;
         //set data
         tbOoO.setDataLoader(loaderTag, cfg);

         cfg->edit("rLen", (uint64_t) tbOoO.sizeOfS());
         cfg->edit("sLen", (uint64_t) tbOoO.sizeOfR());
         //set operator as iawj
         AbstractOperatorPtr iawj = opTable->findOperator(operatorTag);
         tbOoO.setOperator(iawj, cfg);
         webLog("Try use " + operatorTag + " operator");
         webLog("/****run OoO test of  tuples***/");

          OoORu = tbOoO.OoOTest(false);
          //return;
          webLog("OoO Confirmed joined " + to_string(OoORu));
          webLog("OoO AQP joined " + to_string(tbOoO.AQPResult));
         // return;
          ConfigMap generalStatistics;
          generalStatistics.edit("AvgLatency", (double) tbOoO.getAvgLatency());
          generalStatistics.edit("95%Latency", (double) tbOoO.getLatencyPercentage(0.95));
          generalStatistics.edit("Throughput", (double) tbOoO.getThroughput());
          // tbOoO.logRTuples();
          // INTELLI_DEBUG("Average latency (us)=" << tbOoO.getAvgLatency());
          webLog("95% latency (us)=" + to_string(tbOoO.getLatencyPercentage(0.95)));
          webLog("Throughput (TPs/s)=" + to_string(tbOoO.getThroughput()));

          ConfigMapPtr resultBreakDown = tbOoO.getTimeBreakDown();
          if (resultBreakDown != nullptr) {
              resultBreakDown->toFile( "_breakdown.csv");
          }
          /**
           * disable all possible OoO related settings, as we will test the expected in-order results
           */
          cfg->edit("watermarkTimeMs", (uint64_t) (windowLenMs + maxArrivalSkewMs));
          cfg->edit("latenessMs", (uint64_t) 0);
          cfg->edit("earlierEmitMs", (uint64_t) 0);
          tb.setDataLoader(loaderTag, cfg);
          tb.setOperator(iawj, cfg);

          realRu = tb.inOrderTest(true);

          webLog("Expect " + to_string(realRu));

         double err = OoORu;
          err = (err - realRu) / realRu;


          webLog("OoO AQP joined " + to_string(tbOoO.AQPResult));

          err = tbOoO.AQPResult;
          err = (err - realRu) / realRu;
          generalStatistics.edit("AQPError", (double) err);

          webLog("Error = " + to_string(err));
          *error=err;
          *lat=tbOoO.getLatencyPercentage(0.95);

         // ui->textEdit->setText("Error="+QString::number(err));
          printf("done\r\n");
          srand(time(NULL));
}
void MainWindow::testOoO()
{
    OperatorTablePtr opTable = newOperatorTable();
    readOperatorTag();
    cfg= newConfigMap();
     cfg->edit("timeStepUs", (uint64_t)200);
    cfg->edit("windowLenMs", (uint64_t)100);
     cfg->edit("watermarkTimeMs", (uint64_t)100);
     cfg->edit("operator", (std::string)g_opTag);
      cfg->edit("eventRateKTps", (uint64_t)10);
     //finish init phase
    string operatorTag;
    string loaderTag = "random";
    operatorTag = cfg->tryString("operator", "IAWJ");

    //size_t testSize = 0;
    size_t OoORu = 0, realRu = 0;
       //load global configs
    tsType windowLenMs, timeStepUs, maxArrivalSkewMs;
    //uint64_t keyRange;
    windowLenMs = cfg->tryU64("windowLenMs", 10, true);
    timeStepUs = cfg->tryU64("timeStepUs", 40, true);
        //watermarkTimeMs = cfg->tryU64("watermarkTimeMs", 10,true);

    maxArrivalSkewMs = cfg->tryU64("maxArrivalSkewMs", 100 / 2);

    operatorTag = cfg->tryString("operator", "IAWJ");
     loaderTag = cfg->tryString("dataLoader", "random");

     cfg->edit("windowLen", (uint64_t) windowLenMs * 1000);
         //cfg->edit("watermarkTime", (uint64_t) watermarkTimeMs * 1000);
      cfg->edit("timeStep", (uint64_t) timeStepUs);

         TestBench tb, tbOoO;
         //set data
         tbOoO.setDataLoader(loaderTag, cfg);

         cfg->edit("rLen", (uint64_t) tbOoO.sizeOfS());
         cfg->edit("sLen", (uint64_t) tbOoO.sizeOfR());
         //set operator as iawj
         AbstractOperatorPtr iawj = opTable->findOperator(operatorTag);
         tbOoO.setOperator(iawj, cfg);
       //  webLog("Try use " + operatorTag + " operator");
      //   webLog("/****run OoO test of  tuples***/");

          OoORu = tbOoO.OoOTest(false);
          //return;
        //  webLog("OoO Confirmed joined " + to_string(OoORu));
        //  webLog("OoO AQP joined " + to_string(tbOoO.AQPResult));
         // return;
          ConfigMap generalStatistics;
          generalStatistics.edit("AvgLatency", (double) tbOoO.getAvgLatency());
          generalStatistics.edit("95%Latency", (double) tbOoO.getLatencyPercentage(0.95));
          generalStatistics.edit("Throughput", (double) tbOoO.getThroughput());
          // tbOoO.logRTuples();
          // INTELLI_DEBUG("Average latency (us)=" << tbOoO.getAvgLatency());
          webLog("95% latency (us)=" + to_string(tbOoO.getLatencyPercentage(0.95)));
          webLog("Throughput (TPs/s)=" + to_string(tbOoO.getThroughput()));

          tbOoO.saveRTuplesToFile( "_tuples.csv", true);
          tbOoO.saveRTuplesToFile( "_arrived_tuplesR.csv", false);
          tbOoO.saveSTuplesToFile( "_arrived_tuplesS.csv", false);

          ConfigMapPtr resultBreakDown = tbOoO.getTimeBreakDown();
          if (resultBreakDown != nullptr) {
              resultBreakDown->toFile( "_breakdown.csv");
          }
          /**
           * disable all possible OoO related settings, as we will test the expected in-order results
           */
          cfg->edit("watermarkTimeMs", (uint64_t) (windowLenMs + maxArrivalSkewMs));
          cfg->edit("latenessMs", (uint64_t) 0);
          cfg->edit("earlierEmitMs", (uint64_t) 0);
          tb.setDataLoader(loaderTag, cfg);
          tb.setOperator(iawj, cfg);

          realRu = tb.inOrderTest(true);

          webLog("Expect " + to_string(realRu));

          double err = OoORu;
          err = (err - realRu) / realRu;
          generalStatistics.edit("Error", (double) err);

          webLog("OoO AQP joined " + to_string(tbOoO.AQPResult));

          err = tbOoO.AQPResult;
          err = (err - realRu) / realRu;
          generalStatistics.edit("AQPError", (double) err);

          webLog("Error = " + to_string(err));

          generalStatistics.toFile( "_general.csv");
         // ui->textEdit->setText("Error="+QString::number(err));
          printf("done\r\n");

}

void MainWindow::on_pushButton_2_clicked()
{
    //testPlot();
    scanSolutionSpace(100);
}
void MainWindow::scanSolutionSpace(uint64_t windowLen)
{
   // ui->myPlot->clearGraphs();
   // ui->myPlot_2->clearGraphs();
    ui->label->setText("Running "+ ui->comboBox->currentText());
     QPen pen;
    QVector<double> tc(10), lat(10),err(10);
    QVector<uint64_t> tc2(10);
    double wLen=windowLen;
    double step=wLen/10;
    double startPos=wLen/2;
    for (int i=0; i<10; ++i)
    {
      tc[i]=startPos+step*i;
      tc2[i]=tc[i];
    }
    ui->progressBar->setValue(0);
    uint64_t latency;
    double error;
    for (int i=0; i<10; ++i)
    {
      reportOoO(windowLen,tc2[i],&latency,&error);
      lat[i]=latency/1000.0;
      err[i]=abs(error);
      ui->progressBar->setValue(10+10*i);
    }
    QColor clr;

    clr.setRgb(rand()%255,rand()%255,rand()%255,255);
     pen.setColor(clr);
    //finally draw
    ui->myPlot->addGraph();//添加数据曲线（一个图像可以有多个数据曲线）
    ui->myPlot->graph(graphs)->setPen(pen);
    // graph(0);可以获取某个数据曲线（按添加先后排序）
    // setData();为数据曲线关联数据
    ui->myPlot->graph(graphs)->setData(tc, err);
    ui->myPlot->graph(graphs)->setName(ui->comboBox->currentText());// 设置图例名称
    // 为坐标轴添加标签
    ui->myPlot->xAxis->setLabel("Tc (ms)");
    ui->myPlot->yAxis->setLabel("error");
    // 设置坐标轴的范围，以看到所有数据
    ui->myPlot->xAxis->setRange(tc[0]*0.8,tc[9]*1.2);
    ui->myPlot->yAxis->setRange(0, 1);
    ui->myPlot->legend->setVisible(true); // 显示图例
    // 重画图像
    ui->myPlot->replot();


    ui->myPlot_2->addGraph();//添加数据曲线（一个图像可以有多个数据曲线）
    ui->myPlot_2->graph(graphs)->setPen(pen);
    // graph(0);可以获取某个数据曲线（按添加先后排序）
    // setData();为数据曲线关联数据
    ui->myPlot_2->graph(graphs)->setData(tc, lat);
    ui->myPlot_2->graph(graphs)->setName(ui->comboBox->currentText());// 设置图例名称
    // 为坐标轴添加标签
    ui->myPlot_2->xAxis->setLabel("Tc (ms)");
    ui->myPlot_2->yAxis->setLabel("95% latency (ms)");
    // 设置坐标轴的范围，以看到所有数据
    ui->myPlot_2->xAxis->setRange(tc[0]*0.8,tc[9]*1.2);
    ui->myPlot_2->yAxis->setRange(tc[0]*0.8,tc[9]*1.5);
    ui->myPlot_2->legend->setVisible(true); // 显示图例
    // 重画图像
    ui->myPlot_2->replot();
    graphs++;
    ui->label->setText("Done");
}
