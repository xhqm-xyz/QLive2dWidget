#pragma once

/*
* 作者：星辉清梦
* 作用：实例
* 时间：乙巳己卯丁丑戊申
* QLive2dWidget：用于加载和渲染live2d精灵
* *    bool loadSprite(const QString&);     加载精灵模型，可以多次加载多个精灵，成功返回精灵句柄，失败返回空指针
* *    QVector<QLive2dSprite*> sprites();   返回由此窗口管理的精灵列表
* 注意：
* *    建议使用QT5，
* *    使用QT6进行测试无法使QLive2dWidget透明
* *    作者使用的版本是Qt5.12.6，测试的版本是Qt6.9.0bate
*/

#include <QApplication>
#include <QGridLayout>
#include "QLive2dWidget.h"

int main(int argc, char* argv[])
{
	QApplication App(argc, argv);
	QLive2dAdapter::Enable(); //启用Live2dSDK


	QLive2dWidget Widget;
	//加载模型
	QLive2dSprite* Sprite1 = Widget.loadSprite("../Resources/Mao/Mao.model3.json");
	QLive2dSprite* Sprite2 = Widget.loadSprite("../Resources/Haru/Haru.model3.json");
	QLive2dSprite* Sprite3 = Widget.loadSprite("../Resources/Hiyori/Hiyori.model3.json");
	//背景颜色
	Sprite1->setColor(0.0f, 0.3f, 0.7f, 0.1f);
	Sprite2->setColor(0.0f, 0.3f, 0.0f, 0.1f);
	Sprite3->setColor(0.0f, 0.0f, 0.3f, 0.1f);
	//切换表情
	Sprite2->setPriority(Csm::CubismPriority::PriorityNormal);
	QStringList Emoticons = Sprite2->spriteEmoticons();
	Sprite2->setMotion(Emoticons[0]);
	//切换动作
	QStringList Motions = Sprite3->spriteMotions();
	Sprite3->setEmoticon(Motions[0]);
	//布局
	QGridLayout gridLayout(&Widget);
	gridLayout.addWidget(Sprite1, 0, 0, 1, 9);
	gridLayout.addWidget(Sprite2, 1, 0, 1, 5);
	gridLayout.addWidget(Sprite3, 1, 4, 1, 5);
	

	int res = App.exec();
	QLive2dAdapter::Disable(); //关闭Live2dSDK
	return res;
}