#include "FancyLineEdit.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    FancyLineEdit w;
    //w.setAutoHideButton(false);
    //w.setButtonColor(Qt::red);
    //w.setButtonMode(FancyLineEdit::Image);
    //w.setButtonPixmap(QPixmap(":/images/Clear.png"));
    w.show();

    return a.exec();
}
