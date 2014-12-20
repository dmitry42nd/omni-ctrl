#include <QDebug>
#include <QtGui/QApplication>
#include <QStringList>

#include "segway.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QString configPath = "./";
    QString startDirPath = "./";
    double pk = 25.0;
    double dk = 49.2;
    double ik = 4.0;
    double ck = 0.0048;
    double ofs = 3.4;

// pdi: 12.5 24.6 2.0 k: 0.00420    
// pdi: 13.4 25.1 2.1 k: 0.00400
// pdi: 13.5 17.7 1.9 k: 0.003

    int accGAxis = 0;
    int accOAxis = 2;
    int gyroAxis = 0;
    
    if (app.arguments().contains("-c")) {
      int const index = app.arguments().indexOf("-c");
      if (app.arguments().count() <= index + 1) {
        return 1;
      }

      configPath = app.arguments()[index + 1];
      if (configPath.right(1) != "/") {
        configPath += "/";
      }
    }
    
    if (app.arguments().contains("-d")) {
      int const index = app.arguments().indexOf("-d");
      if (app.arguments().count() <= index + 1) {
        return 1;
      }

      startDirPath = app.arguments()[index + 1];
      if (configPath.right(1) != "/") {
        configPath += "/";
      }
    }
    
    if (app.arguments().contains("-pk")) {
      int const index = app.arguments().indexOf("-pk");
      if (app.arguments().count() <= index + 1) {
        return 1;
      }

      pk = app.arguments()[index + 1].toDouble();
    }
    if (app.arguments().contains("-ik")) {
      int const index = app.arguments().indexOf("-ik");
      if (app.arguments().count() <= index + 1) {
        return 1;
      }

      ik = app.arguments()[index + 1].toDouble();
    }
    
    if (app.arguments().contains("-dk")) {
      int const index = app.arguments().indexOf("-dk");
      if (app.arguments().count() <= index + 1) {
        return 1;
      }

      dk = app.arguments()[index + 1].toDouble();
    }
    
    if (app.arguments().contains("-ck")) {
      int const index = app.arguments().indexOf("-ck");
      if (app.arguments().count() <= index + 1) {
        return 1;
      }

      ck = app.arguments()[index + 1].toDouble();
    }
    
    if (app.arguments().contains("-ofs")) {
      int const index = app.arguments().indexOf("-ofs");
      if (app.arguments().count() <= index + 1) {
        return 1;
      }

      ofs = app.arguments()[index + 1].toDouble();
    }
    
    if (app.arguments().contains("-h") || app.arguments().contains("--help")) {
      qDebug() << "Flags:";
      qDebug() << "-c   - set path to config file";
      qDebug() << "-d   - set path to start directory";
      qDebug() << "-pk";
      qDebug() << "-ik";
      qDebug() << "-dk  - PID coeffitients";
      qDebug() << "-ck  - complementary filter coeffitient";
      qDebug() << "-ofs - angle offset";

      return 0;
    }

    Segway robot(&app, configPath, startDirPath, pk, dk, ik, ck, ofs, accGAxis, accOAxis, gyroAxis);

    return app.exec();
}
