
*作者：星辉清梦*
* QLive2dAdapter
* QLive2dSprite
* QLive2dWidget


**QLive2dAdapter**
* 适配器：用于管理Live2d SDK，兼容String
* 触摸适配器：复制粘贴官方代码，就换了个Qt风格的名
* 内存适配器：复制粘贴官方代码，就换了个Qt风格的名
  

**QLive2dSprite**
* QSound：用于管理声音
* QShader：用于管理着色
* QTexture：用于管理纹理
* CubismPriority：官方的优先级的枚举实现
* 模型精灵：用于管理live2d精灵

  
**QLive2dWidget**
* 渲染窗口：用于加载、渲染和管理live2d精灵


***注意：***
* *    建议使用QT5，
* *    使用QT6进行测试无法使QLive2dWidget透明
* *    作者使用的版本是Qt5.12.6，测试的版本是Qt6.9.0bate
* *	20250504更新
* *    建议使用QT5.15.2，
* *    使用QT5.12进行测试无法捕获音频帧

***示例：***

#include "QLive2dWidget.h"


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
	//播放声音
	Sprite1->openVoice("/voice.wav");
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