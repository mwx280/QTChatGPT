#include "ChatGPT.h"
#include <QtWidgets/QApplication>
#include <QTranslator>

class ChatGPTApp : public QApplication
{
public:
	ChatGPTApp(int& argc, char** argv) : QApplication(argc, argv) {}

	void switchLanguage(const QString& language)
	{
		if (language == "zh_CN")
		{
			removeTranslator(&tran);
		}
		else
		{
			installTranslator(&tran);
		}
	}

private:
	QTranslator tran;
};

int main(int argc, char* argv[])
{
	ChatGPTApp a(argc, argv);

	ChatGPT w;
	w.show();

	QObject::connect(&w, &ChatGPT::languageChanged, &a, [&](const QString& language)
		{
			a.switchLanguage(language);
			w.update();
		});

	return a.exec();
}
