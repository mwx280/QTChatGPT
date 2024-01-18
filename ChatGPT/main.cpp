#include "ChatGPT.h"
#include <QtWidgets/QApplication>

int main(int argc, char* argv[])
{
	QApplication a(argc, argv);

	QTranslator tran;
	if (tran.load(QString("./translations/qt_zh_CN.qm")))
	{
		a.installTranslator(&tran);
	}

	ChatGPT w;
	w.show();
	return a.exec();
}
