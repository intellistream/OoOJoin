#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <OoOJoin.h>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void testOoO();
    void reportOoO(uint64_t windowLen,uint64_t tc,uint64_t *lat,double *err);
    void webLog(std::string log);
    void scanSolutionSpace(uint64_t windowLen);
    void readOperatorTag();
    void testPlot();
private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::MainWindow *ui;
    std::string g_opTag;
    int graphs=0;
};
#endif // MAINWINDOW_H
