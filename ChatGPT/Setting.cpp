#include "Setting.h"

Setting::Setting(QWidget* parent, QString lineEditText)
	: QDialog(parent)
{
	ui.setupUi(this);
	//设置为模态对话框
	this->setModal(true);
	ui.lineEditAPI->setText(lineEditText);
	ui.lineEditAPI->setPlaceholderText("留空为使用默认秘钥");
	connect(ui.btnOK, &QPushButton::clicked, this, &Setting::btnOKClicked);
}

Setting::~Setting()
{}

void Setting::btnOKClicked()
{
	//发送API秘钥
	emit sendApiKey(ui.lineEditAPI->text());
	this->close();
}
