#include <QDebug>
#include <QtGui/QApplication>
#include <QStringList>

#include "rover.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QString configPath = "./";
    QString soundPath = "/home/root/music/1.wav";
    QString speech = "Привет, я - роообот";
    if (app.arguments().contains("-c")) 
    {
      int const index = app.arguments().indexOf("-c");
      if (app.arguments().count() <= index + 1) 
      {
        return 1;
      }

      configPath = app.arguments()[index + 1];
      if (configPath.right(1) != "/") 
      {
        configPath += "/";
      }
    }
    if (app.arguments().contains("--music")) 
    {
      int const index = app.arguments().indexOf("--music");
      if (app.arguments().count() <= index + 1) 
      {
        return 1;
      }

      soundPath = app.arguments()[index + 1];
    }
    if (app.arguments().contains("--say")) 
    {
      int const index = app.arguments().indexOf("--say");
      if (app.arguments().count() <= index + 1) 
      {
        return 1;
      }

      speech = app.arguments()[index + 1];
    }
    if (app.arguments().contains("-h") || app.arguments().contains("--help")) 
    {
      qDebug() << "Flags:";
      qDebug() << "--music  -  set path to music file";
      qDebug() << "--say    -  set phrase. Ex: --say \"Хэллоу, ворлд!\"";

      return 0;
    }


    Rover robot(app.thread(), configPath, soundPath, speech);

    return app.exec();
}
