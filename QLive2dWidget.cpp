#include "QLive2dWidget.h"

QLive2dWidget::QLive2dWidget(QWidget* parent)
	: QWidget(parent)
{
	//此处的执行顺序不可以改动，否则无法透明窗口
	setStyleSheet("background-color:rgba(255, 255, 255, 0);");
	setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
	setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
	setWindowFlags(windowFlags() | Qt::Tool);

	setAttribute(Qt::WA_TranslucentBackground); // 背景透明
	setAttribute(Qt::WA_StyledBackground);
	resize(360, 540);
	show();
}

QLive2dWidget::~QLive2dWidget()
{
	for (auto& live2dSprite : m_live2dSprites)
	{
		delete live2dSprite;
	}
}

QLive2dSprite* QLive2dWidget::loadSprite(const QString& modelPath)
{
	QLive2dSprite* spriteModel = new QLive2dSprite(this);
	if (spriteModel->loadPath(modelPath))
	{
		m_live2dSprites.push_back(spriteModel);
	}
	else
	{
		delete spriteModel; spriteModel = nullptr;
	}
	return spriteModel;
}

void QLive2dWidget::wheelEvent(QWheelEvent* Event)
{
	QWidget::wheelEvent(Event);
	QSize tranSize = this->size();
	if (Event->delta() > 0)
	{
		this->resize(tranSize * (1.0 + 0.03));
	}
	else
	{
		this->resize(tranSize * (1.0 - 0.03));
	}
}

void QLive2dWidget::resizeEvent(QResizeEvent* Event)
{
	QWidget::resizeEvent(Event);
}

void QLive2dWidget::mousePressEvent(QMouseEvent* Event)
{
	QWidget::mousePressEvent(Event);
	if (Qt::RightButton == Event->button())
	{
		m_isPress = true;
		m_currPoint = Event->pos();
	}
}

void QLive2dWidget::mouseReleaseEvent(QMouseEvent* Event)
{
	QWidget::mouseReleaseEvent(Event);
	m_isPress = false;
}

void QLive2dWidget::mouseMoveEvent(QMouseEvent* Event)
{
	QWidget::mouseMoveEvent(Event);
	if (m_isPress)
	{
		move(Event->pos() - m_currPoint + this->pos());
	}
}
