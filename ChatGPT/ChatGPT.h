#pragma once

#include <QtWidgets/QWidget>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include "ui_ChatGPT.h"
#include <QtNetwork>
#include <QKeyEvent>

class ChatGPT : public QWidget
{
	Q_OBJECT

public:
	ChatGPT(QWidget* parent = nullptr);
	~ChatGPT();

private:
	//初始化按钮控件
	void initButton(QToolButton*& btn, const QString& text, const QString& icon);
	//清空聊天槽函数
	void clearChat();
	//置顶聊天窗口槽函数
	void topWidget();
	//写入文件槽函数
	void wirteToFile();
	//询问GPT
	void sendToGPT();
	//事件过滤器
	bool eventFilter(QObject* obj, QEvent* event) override;
signals:
	void updateUI(QString msg);
protected:
	bool once = true;
	QString apiKey = "你的apiKry";
	QString apiUrl = "https://api.chatanywhere.tech/v1/chat/completions";
private:
	QThread* sendThread;//子线程
private:
	Ui::ChatGPTClass ui;
};
