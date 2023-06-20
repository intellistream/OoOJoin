#include "mainwindow.h"

#include <QApplication>
#include <QFontDatabase>

#include <QTextCodec>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    int fontId = QFontDatabase::addApplicationFont(":/font.ttc");
    QString fontname = QFontDatabase::applicationFontFamilies (fontId).at(0); QFont font; font.setFamily(fontname);
    font.setPixelSize(13); font.setWeight(QFont::Normal); font.setItalic(false); a.setFont(font);
    QTextCodec *codec = QTextCodec::codecForName("UTF-8"); QTextCodec::setCodecForLocale(codec);


    MainWindow w;
    w.show();
    return a.exec();
}
