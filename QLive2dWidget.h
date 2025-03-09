#pragma once

/*
* 作者：星辉清梦
* 作用：渲染窗口
* 时间：乙巳己卯丁丑戊申
* QLive2dWidget：用于加载和渲染live2d精灵
* *    bool loadSprite(const QString&);     加载精灵模型，可以多次加载多个精灵，成功返回精灵句柄，失败返回空指针
* *    QVector<QLive2dSprite*> sprites();   返回由此窗口管理的精灵列表
*/

#ifndef QLIVE2DWIDGET
#define QLIVE2DWIDGET

#include "QLive2dSprite.h"

class QLive2dWidget : public QWidget
{
    Q_OBJECT
private:
    bool m_isPress = false;
    QPoint m_currPoint = {};
    QVector<QLive2dSprite*> m_live2dSprites;
public:
    explicit QLive2dWidget(QWidget* parent = nullptr);
	virtual ~QLive2dWidget();
public:
    QLive2dSprite* loadSprite(const QString& modelPath);
    inline QVector<QLive2dSprite*> sprites() { return m_live2dSprites; };
protected: //事件
    virtual void wheelEvent(QWheelEvent*) override; //滚轮
    virtual void resizeEvent(QResizeEvent*) override; //大小
    virtual void mousePressEvent(QMouseEvent*) override; //按下
    virtual void mouseReleaseEvent(QMouseEvent*) override; //释放
    virtual void mouseMoveEvent(QMouseEvent*) override; //释放
};

#endif //QLIVE2DWIDGET