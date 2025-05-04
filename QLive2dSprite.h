#pragma once

/*
* 作者：星辉清梦
* 作用：精灵
* 时间：乙巳己卯丁丑戊申
* QSound：用于管理声音
* QShader：用于管理着色
* QTexture：用于管理纹理
* CubismPriority：官方的优先级的枚举实现
* QLive2dSprite：用于管理live2d精灵
* *    bool loadPath(const QString&);       加载精灵模型，只能在initializeGL后调用，仅能使用一次
* *    bool openVoice(const QString&);      播放外置声音，一般由用户手动先择路径
* *    bool setVoice(const QString&);       启动内置声音，一般由setMotion自动调用
* *    bool setMotion(const QString&);      启动模型动作
* *    bool setEmoticon(const QString&);    启动模型表情
*/

#ifndef QLIVE2DSPRITE
#define QLIVE2DSPRITE

#include <QRect>
#include <QFile>
#include <QImage>
#include <QTimer>
#include <QString>
#include <QFileInfo>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QOpenGLWidget>
#include <QOpenGLContext>
#include <QOpenGLFunctions>

#include "QLive2dAdapter.h"

class QVoice
{
private:
    QString m_SoundPath = "";
public:
    QVoice();
    ~QVoice();
public:
    bool LoadSound(const QString& SoundPath);
public:
    inline QString Path() const { return m_SoundPath; };
};

class QShader
{
private:
    GLuint m_programId = 0;
    QOpenGLContext* m_glContext = nullptr;
private:
    GLuint _CompileShader(const QString& shaderPath, GLenum shaderType);
public:
    //当glContext为nullptr时，将自动获取当前上下文
    QShader(QOpenGLContext* glContext = nullptr);
    ~QShader();
public:
    bool LoadShader(const QString& vertPath, const QString& fragPath);
public:
    inline GLuint Id() const { return m_programId; };
};

class QTexture
{
private:
    GLuint m_programId = 0;
    unsigned int m_width = 0;
    unsigned int m_height = 0;
    QOpenGLContext* m_glContext = nullptr;
public:
    //当glContext为nullptr时，将自动获取当前上下文
    QTexture(QOpenGLContext* glContext = nullptr);
    ~QTexture();
public:
    bool LoadTextures(const QString& filePath);
public:
    inline GLuint Id() const { return m_programId; };
    inline unsigned int Width() const { return m_width; };
    inline unsigned int Height() const { return m_height; };
};

namespace Live2D {
    namespace Cubism {
        namespace Framework {
            enum class CubismPriority : int
            {
                PriorityNone = 0,
                PriorityFirst = 1,
                PriorityNormal = 2,
                PriorityStrong = 3,
            };
        }
    }
};

class QLive2dSprite 
    : public QOpenGLWidget
    , public Csm::CubismUserModel
{
    Q_OBJECT
private:
    struct _impClass;
    _impClass* m_packet = nullptr;
private:
    static void __connectBeganCallback(Csm::ACubismMotion* self);
    static void __connectFinishedCallback(Csm::ACubismMotion* self);
public:
    explicit QLive2dSprite(QWidget* parent = nullptr);
    virtual ~QLive2dSprite();
public:
    QString spriteName() const;
    QString spriteVoice() const;              //当前声音
    QString spriteMotion() const;             //当前动画
    QString spriteExpression() const;         //当前动作
    QStringList spriteVoices() const;         //声音列表
    QStringList spriteMotions() const;        //动画列表
    QStringList spriteExpressions() const;    //动作列表
public:
    bool loadPath(const QString&);
    bool openVoice(const QString&);
    bool setVoice(const QString&);
    bool setMotion(const QString&);
    bool setExpression(const QString&);
    void setColor(float red, float green, float blue, float alph);      //填充颜色
    void setMatrix(const Csm::CubismMatrix44&);                         //变换矩阵
    void setPriority(const Csm::CubismPriority&);                       //优先等级

public:
    virtual void initializeGL() override;

    virtual void paintGL() override;
    virtual void resizeGL(int, int) override;

protected: //事件
    virtual void wheelEvent(QWheelEvent*) override; //滚轮
    virtual void paintEvent(QPaintEvent*) override; //绘制
    virtual void resizeEvent(QResizeEvent*) override; //大小
    virtual void mousePressEvent(QMouseEvent*) override; //按下
    virtual void mouseReleaseEvent(QMouseEvent*) override; //释放
    virtual void mouseMoveEvent(QMouseEvent*) override; //释放
private:
    void UpdateTime();
    void ReleaseVoices();
    void ReleaseMotions();
    void ReleaseExpressions();
    QString HitTest(float, float, float opa = 1.f);

signals: //信号
    void startLoaded();						//加载模型
    void loadModelFailed();					//加载失败
    void loadModelSucceeded();				//加载成功
    void voicesChanged(QString);			//切换声音
    void motionsChanged(QString);			//切换动画
    void expressionsChanged(QString);		//切换动作
    void motionsBegan(Csm::ACubismMotion*);		//动画开始
    void motionsFinished(Csm::ACubismMotion*);	//动画结束
};

#endif