#include "ChatGPT.h"
#include "Setting.h"
#include <QDesktopServices>

ChatGPT::ChatGPT(QWidget* parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	//判断是否有用户秘钥
	QFile* file = new QFile(this);
	file->setFileName("data.dat");
	if (file->open(QIODevice::ReadOnly))
	{
		QByteArray byteData = file->readAll();
		QString api = QString(byteData);
		apiKey = api;
		file->close();
	}

	ui.textEditUser->installEventFilter(this);//安装事件过滤器
	ui.textEditGPT->setReadOnly(true);//禁止用户编辑
	//this->showMaximized();
	this->setWindowIcon(QIcon(":/img/gpt.png"));
	//初始化按钮控件
	initButton(ui.btnTop, "置顶窗口", ":/img/top.png");
	initButton(ui.btnSetting, "设      置", ":/img/setting.png");
	initButton(ui.btnWriteFile, "写入文件", ":/img/file.png");
	initButton(ui.btnSend, "发      送", ":/img/send.png");
	initButton(ui.btnClear, "清      空", ":/img/clear.png");
	initButton(ui.btnGitHub, "关于软件", ":/img/github.png");

	//设置用户输入框为默认焦点
	ui.textEditUser->setFocus();
	//设置textedit提示
	ui.textEditUser->setPlaceholderText("在这里输入你要询问ChatGPT的内容");
	ui.textEditGPT->setPlaceholderText("ChatGPT的回答会显示在这里");

	//连接按钮信号和槽
	connect(ui.btnClear, &QToolButton::clicked, this, &ChatGPT::clearChat);
	connect(ui.btnTop, &QToolButton::clicked, this, &ChatGPT::topWidget);
	connect(ui.btnWriteFile, &QToolButton::clicked, this, &ChatGPT::wirteToFile);
	connect(ui.btnSend, &QToolButton::clicked, this, &ChatGPT::sendToGPT);
	connect(ui.btnSetting, &QToolButton::clicked, this, &ChatGPT::chatGPTSetting);
	connect(ui.btnGitHub, &QToolButton::clicked, this, &ChatGPT::GitHubPage);
	//GPT消息处理
	connect(this, &ChatGPT::sendGPTMsg, this, &ChatGPT::receiveGPTMsg);
	//连接错误
	connect(this, &ChatGPT::sendPostErrorMsg, this, &ChatGPT::receiveErrorMsg);
}

ChatGPT::~ChatGPT()
{}

void ChatGPT::initButton(QToolButton*& btn, const QString& text, const QString& icon)
{
	btn->setIconSize(QSize(20, 20));//设置图标大小
	btn->setIcon(QIcon(icon));		//设置图标
	btn->setText(text);				//设置按钮文本
	btn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);//设置按钮风格为文本+标签
}

void ChatGPT::clearChat()
{
	//清空输出框
	ui.textEditGPT->clear();
}

void ChatGPT::topWidget()
{
	//通过文字判断窗口是否置顶
	if (ui.btnTop->text() == "置顶窗口")
	{
		this->setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);//窗口置顶
		this->show();//重新绘制窗口
		ui.btnTop->setText("取消置顶");
	}
	else
	{
		this->setWindowFlags(windowFlags() & ~Qt::WindowStaysOnTopHint);//取消置顶
		this->show();
		ui.btnTop->setText("置顶窗口");
	}
}

void ChatGPT::wirteToFile()
{
	//弹出对话框，让用户选择保存位置
	QString fileName = QFileDialog::getSaveFileName(this, "保存文件", "ChatGPT", "文本文件 (*.txt)");
	//判断用户选项
	if (!fileName.isEmpty())
	{
		QFile file(fileName);//创建文件
		//判断是否打开成功
		if (file.open(QIODevice::WriteOnly | QIODevice::Text))
		{
			QTextStream out(&file);//文件操作流写入文件
			QString textData = ui.textEditGPT->toPlainText();//获取输入框文本
			out << textData;//写入文件
			file.close();//关闭文件
			QMessageBox::about(this, "保存完毕", QString("文件已保存到%1").arg(fileName));
		}
		else
		{
			QMessageBox::about(this, "写入文件错误", "无法写入到指定位置！");
		}
	}
}

void ChatGPT::sendToGPT()
{
	//判断输入框是否为空
	if (ui.textEditUser->toPlainText().isEmpty())
	{
		ui.textEditUser->setPlaceholderText("输入框不能为空哦！");
		return;
	}
	ui.textEditUser->setPlaceholderText("在这里输入你要询问ChatGPT的内容");
	ui.btnSend->setEnabled(false);					//设置按钮不可用
	ui.textEditGPT->moveCursor(QTextCursor::End);	//移动光标到最后一行
	//组合文本
	QString msg = ui.textEditUser->toPlainText();
	//输出html使文本框变色
	ui.textEditGPT->append(QString("<span style=\"color: blue;\"><pre>%1</pre></span>").arg(msg.toHtmlEscaped()));
	ui.textEditUser->clear();       //清空用户输入框
	QString GPTMsg = ui.textEditGPT->toPlainText(); //给gpt发送的消息

	//创建一个QThread对象
	QThread* thread = new QThread(this);
	//将当前对象移动到新线程中
	this->moveToThread(thread);

	//连接线程启动信号和槽
	connect(thread, &QThread::started, [=]()
		{
			//创建一个QJsonObject用于存储消息
			QJsonObject message;
			//将角色字段插入到message对象中，值为"user"
			message.insert("role", "user");
			//将用户输入的消息内容插入到message对象中
			message.insert("content", GPTMsg);

			//创建一个QJsonArray用于存储所有消息
			QJsonArray messages;
			//将上一步创建的message对象插入到messages数组中
			messages.append(message);

			//创建一个QJsonObject用于存储数据
			QJsonObject data;
			//将模型名称插入到data对象中
			data.insert("model", "gpt-3.5-turbo");
			//将消息数组插入到data对象中
			data.insert("messages", messages);

			//创建一个QNetworkAccessManager对象来发送网络请求
			QNetworkAccessManager* manager = new QNetworkAccessManager();
			//创建一个QNetworkRequest对象，并设置apiUrl为请求的URL
			QNetworkRequest request(QUrl::fromUserInput(apiUrl));

			//设置请求头的内容类型为"application/json"
			request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
			//设置请求头的Authorization字段，值为"Bearer [apiKey]"
			request.setRawHeader("Authorization", QString("Bearer %1").arg(apiKey).toUtf8());

			//发送POST请求，并将data对象转化为JSON格式的数据作为请求体
			QNetworkReply* reply = manager->post(request, QJsonDocument(data).toJson());

			//连接请求结束的信号finished，准备处理响应结果
			connect(reply, &QNetworkReply::finished, [=]()
				{
					//如果没有发生错误
					if (reply->error() == QNetworkReply::NoError)
					{
						//读取响应数据
						QByteArray response = reply->readAll();
						//将响应数据解析为QJsonDocument对象
						QJsonDocument doc = QJsonDocument::fromJson(response);
						//将QJsonDocument对象转化为QJsonObject对象
						QJsonObject obj = doc.object();

						//处理响应对象
						//从响应对象中获取choices数组
						QJsonArray choices = obj.value("choices").toArray();
						//如果choices数组不为空
						if (!choices.isEmpty())
						{
							//取出第一个choice对象，并从其中获取message字段的值
							QJsonObject message = choices.at(0).toObject().value("message").toObject();
							//从message对象中获取content字段的值
							QString content = message.value("content").toString() + "\n";
							emit sendGPTMsg(content);//发送消息
						}
					}
					else
					{
						//发送给主线程错误信息
						emit sendPostErrorMsg(reply->errorString());
					}

					//释放reply对象和manager对象
					reply->deleteLater();
					manager->deleteLater();
					//退出线程
					thread->quit();
				});
		});

	//连接线程结束信号和槽
	connect(thread, &QThread::finished, thread, &QThread::deleteLater);

	//启动线程
	thread->start();
}

void ChatGPT::chatGPTSetting()
{
	QString str;
	//判断秘钥是否一致，不一致则写入秘钥到文件
	if (defaultApiKey == apiKey)
	{
		str = "";
	}
	else
	{
		str = apiKey;
	}
	Setting* setting = new Setting(this, str);
	setting->setWindowTitle("ChatGPT设置");
	//从设置页面获取apiKey
	connect(setting, &Setting::sendApiKey, this, &ChatGPT::getApiKey);

	setting->show();
}

void ChatGPT::GitHubPage()
{
	//创建对话框
	QDialog* GitHub = new QDialog(this);
	GitHub->setWindowTitle("关于软件");
	GitHub->setModal(true);
	GitHub->setFixedSize(QSize());

	QLabel* label = new QLabel("项目开源地址:", GitHub);
	//创建跳转按钮
	QPushButton* btnGitHub = new QPushButton("点击跳转到GItHub", GitHub);
	btnGitHub->setStyleSheet("QPushButton { color: red; }"
		"QPushButton:hover { background-color: #e6e6e6; }"
		"QPushButton:pressed { background-color: #d9d9d9; }");
	//连接按钮信号，点击后跳转到GItHub页面
	connect(btnGitHub, &QPushButton::clicked, this, [=]()
		{
			QDesktopServices::openUrl(QUrl("https://github.com/mwx280/QTChatGPT"));
			GitHub->close();
		});
	//添加控件到垂直布局
	QHBoxLayout* layout = new QHBoxLayout(GitHub);
	layout->addWidget(label);
	layout->addWidget(btnGitHub);

	GitHub->show();
}

void ChatGPT::receiveGPTMsg(const QString& GPTMsg)
{
	ui.btnSend->setEnabled(true);					//设置按钮可用
	//输出html使文本框变色
	ui.textEditGPT->append(QString("<span style=\"color: red;\"><pre>%1</pre></span>").arg(GPTMsg.toHtmlEscaped()));

	ui.textEditGPT->moveCursor(QTextCursor::End);	//移动光标到最后一行
}

void ChatGPT::receiveErrorMsg(const QString& errorMsg)
{
	//提示用户请求错误
	QMessageBox::about(this, "请求错误", QString("请检查设置中的秘钥是否正确！\n错误原因：%1").arg(errorMsg));
	//设置按钮可用
	ui.btnSend->setEnabled(true);
}

bool ChatGPT::eventFilter(QObject* obj, QEvent* event)
{
	//判断事件源是否为 ui.textEditUser，并且事件类型为按键事件
	if (obj == ui.textEditUser && event->type() == QEvent::KeyPress)
	{
		//将事件转换为按键事件
		QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);

		//判断按下的键是否为回车键（Enter键）
		if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)
		{
			//获取按钮点击状态
			if (ui.btnSend->isEnabled() == true)
			{
				//触发发送按钮的点击事件
				ui.btnSend->clicked();
				return true; //返回true表示事件已处理
			}
		}
	}

	//没有合适事件，继续处理事件
	return QObject::eventFilter(obj, event);
}

void ChatGPT::getApiKey(const QString& newApiKey)
{
	if (newApiKey.isEmpty())
	{
		//设置为默认apiKey
		apiKey = defaultApiKey;
		//清空写入的秘钥信息
		wirteApiKeyFile("");
		return;
	}
	else
	{
		apiKey = newApiKey;
		//写入秘钥到文件
		wirteApiKeyFile(newApiKey);
	}
}

void ChatGPT::wirteApiKeyFile(const QString& apiKey)
{
	//写入apiKey到文件
	QFile* file = new QFile(this);
	file->setFileName("data.dat");
	if (file->open(QIODevice::WriteOnly))
	{
		file->write(apiKey.toUtf8());
		file->close();
	}
}