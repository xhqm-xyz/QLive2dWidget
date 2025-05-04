#pragma once
#include <QApplication>
#include <QGridLayout>
#include "QLive2dWidget.h"

int main(int argc, char* argv[])
{
	QApplication App(argc, argv);
	QLive2dAdapter::Enable(); //启用Live2dSDK

	QLive2dWidget Widget;
	Widget.resize(360, 540);
	QGridLayout gridLayout(&Widget);
	//加载模型

	QString modelPath = App.applicationDirPath();
	QLive2dSprite* MioSprite = Widget.loadSprite(modelPath + u8"/Mio/Mio.model3.json");
	if (MioSprite)
	{
		//背景颜色
		MioSprite->setColor(0.0f, 0.0f, 0.0f, 0.0f);
		//增加文字流投影屏
		gridLayout.addWidget(MioSprite, 0, 0, 100, 150);
	}


	int res = App.exec();
	QLive2dAdapter::Disable(); //关闭Live2dSDK
	return res;
}