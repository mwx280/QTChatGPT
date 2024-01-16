#include "ChatGPT.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ChatGPT w;
    w.show();
    return a.exec();
}
