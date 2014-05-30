#include "control_3dmachine.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QSplashScreen>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QPixmap pixmap(":/new/prefix1/slogo.jpg");
    QSplashScreen *splash=new QSplashScreen;
    splash->setPixmap(pixmap);
    splash->show();
    a.processEvents();
    QTime time;
    time.start();
    while(time.elapsed() < 1000)
      {
        QCoreApplication::processEvents();
      }
    Control_3DMachine w;
    w.show();
    w.move((QApplication::desktop()->width() - w.width())/2,
               (QApplication::desktop()->height() - w.height())/2);
    splash->finish(&w);
    delete splash;
    return a.exec();
}
