#include "control_3dmachine.h"
#include <QApplication>
#include <QDesktopWidget>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Control_3DMachine w;
    //    Qt :: WindowFlags flags = 0;
    //    flags = flags | Qt :: WindowMinimizeButtonHint;
    //    flags = flags | Qt ::WindowMaximizeButtonHint;
    //    w.setWindowFlags(flags);
    w.show();
    w.move((QApplication::desktop()->width() - w.width())/2,
               (QApplication::desktop()->height() - w.height())/2);
    return a.exec();
}
