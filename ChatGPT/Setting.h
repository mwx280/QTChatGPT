﻿#pragma once

#include <QDialog>
#include "ui_Setting.h"

class Setting : public QDialog
{
	Q_OBJECT

public:
	Setting(QWidget* parent = nullptr, QString lineEditText = nullptr);
	~Setting();

public:
	void btnOKClicked();

signals:
	//语言切换信号
	void languageChanged(QString language);
	//发送apiKey信号
	void sendApiKey(QString API);

private:
	Ui::SettingClass ui;
};
