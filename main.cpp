#include <QtGui/QApplication>
#include <QStringList>
#include "linetracer.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QString configPath = "./";
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

    Linetracer robot(app.thread(), configPath);

    return app.exec();
}
