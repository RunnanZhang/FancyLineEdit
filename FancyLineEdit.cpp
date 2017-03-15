#include "FancyLineEdit.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QEvent>
#include <QDebug>
#include <QTimer>
#include <QPainter>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

const quint32 FADE_TIME = 200;

FancyLineEdit::FancyLineEdit(QWidget *parent)
    : QLineEdit(parent),
      m_bHovered(false),
      m_backgroundColor(QColor(255, 255, 255)),
      m_buttonColor(Qt::gray),
      m_buttonMode(ButtonMode::Paint)
{
    m_label = new QLabel(this);
    m_label->installEventFilter(this);

    m_opacityEffect = new QGraphicsOpacityEffect(m_label);
    m_label->setGraphicsEffect(m_opacityEffect);
    setAutoHideButton(true);

    // 不可用如下方式，否则设置失败，此方式对有父指针的widget会失败.
    //m_label->setWindowOpacity(0.5);
    m_label->setAttribute(Qt::WA_Hover);
    m_label->setCursor(QCursor(Qt::PointingHandCursor));
    m_label->setToolTip("Clear up to default value");

    connect(this, &QLineEdit::textChanged, this, [=](){animateShow(!this->text().isEmpty());});

    // 我们目前利用resize里面不断更新label的位置，也可利用下面加布局，这样位置不需要更新，但是需要设置label的FixSize.否则会被挤压.
//    QHBoxLayout *buttonLayout = new QHBoxLayout();
//    buttonLayout->setContentsMargins(0, 1, 1, 1);
//    buttonLayout->addStretch();
//    buttonLayout->addWidget(m_label);
//    this->setLayout(buttonLayout);
}

void FancyLineEdit::setDefaultText(const QString &text)
{
    m_defaultText = text;
}

bool FancyLineEdit::event(QEvent *e)
{
    if (e->type() == QEvent::FocusIn)
    {
        // 此处直接调用会失败，在event里应该还有处理致使此方法不好用，利用QTimer实现，此时槽函数会在此函数完成时调用.
        QTimer::singleShot(0, this, SLOT(selectAll()));
    }
    else if (e->type() == QEvent::Resize)
    {
        int width = this->geometry().width();
        int height = this->geometry().height();
        int buttonSize = height - 2;

        m_label->setGeometry(width - height, 1, buttonSize, buttonSize);

        // 设置输入框中文件输入区，不让输入的文字在被隐藏在按钮下.
        this->setTextMargins(0, 1, buttonSize, 1);
    }

    return QLineEdit::event(e);
}

bool FancyLineEdit::eventFilter(QObject *obj, QEvent *e)
{
    if (e->type() == QEvent::HoverEnter)
    {
        m_bHovered = true;
        // 由于设置了Qt::WA_Hover属性，此处无需显示调用update().
        //update();
    }
    else if(e->type() == QEvent::HoverLeave)
    {
        m_bHovered = false;
    }
    else if(e->type() == QEvent::MouseButtonRelease)
    {
        this->setText(m_defaultText);
    }
    else if(e->type() == QEvent::Paint)
    {
        QPainter painter(m_label);

        if(m_buttonMode == FancyLineEdit::Image)
        {
            QRect backRect = m_label->rect();
            painter.drawPixmap(backRect.topLeft(),  m_buttonPixmap.scaled(backRect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        }
        else
        {
            // must use QRectF.
            QRectF backRect = m_label->rect();
            qreal rX = backRect.x() + backRect.width()/10;
            qreal rY = backRect.y() + backRect.height()/10;
            qreal rW = backRect.width()/5*4;
            qreal rH = backRect.height()/5*4;
            QRectF circleRect(rX, rY, rW, rH);
            painter.setRenderHint(QPainter::Antialiasing, true);

            painter.setBrush(m_backgroundColor);
            painter.setPen(Qt::NoPen);
            painter.drawRect(backRect);

            painter.setBrush(m_bHovered ? m_buttonColor.dark() : m_buttonColor);
            painter.drawEllipse(circleRect);

            painter.setPen(QPen(Qt::white, 2));
            qreal rR = backRect.width()/10*5;
            rX = backRect.width()/2 - rR/3;
            rY = backRect.width()/2 + rR/3;
            painter.drawLine(QPointF(rX, rX), QPointF(rY, rY));
            painter.drawLine(QPointF(rY, rX), QPointF(rX, rY));
        }
    }

    return QLineEdit::eventFilter(obj, e);
}

void FancyLineEdit::animateShow(bool visible)
{
    if(!m_isAutoHide)
    {
        // 如若不是自动隐藏，则一直显示，无需触发下面代码.
        return;
    }

    if(visible)
    {
        m_label->show();
    }
    else
    {
        QTimer::singleShot(FADE_TIME, m_label, SLOT(hide()));
    }

    QPropertyAnimation *animation = new QPropertyAnimation(m_label->graphicsEffect(), "opacity");
    animation->setDuration(FADE_TIME);
    animation->setEndValue(visible ? 1.0 : 0.0);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

FancyLineEdit::ButtonMode FancyLineEdit::buttonMode() const
{
    return m_buttonMode;
}

void FancyLineEdit::setButtonMode(ButtonMode mode)
{
    m_buttonMode = mode;
}

QPixmap FancyLineEdit::buttonPixmap() const
{
    return m_buttonPixmap;
}
void FancyLineEdit::setButtonPixmap(const QPixmap &pixmap)
{
    m_buttonPixmap = pixmap;
}

void FancyLineEdit::setButtonToolTip(const QString &tip)
{
    m_label->setToolTip(tip);
}

void FancyLineEdit::setAutoHideButton(bool isAutoHide)
{
    m_isAutoHide = isAutoHide;

    m_opacityEffect->setOpacity(1);
    m_label->show();

    if(m_isAutoHide && m_label->text().isEmpty())
    {
        // 此处虽然不透明度为0，也看不到，但是我们下面设置了QCursor样式，虽透明但仍可触发，所以此处有隐藏必要，而不是仅视觉上不可见.
        m_opacityEffect->setOpacity(0);
        m_label->hide();
    }
}

bool FancyLineEdit::isAutoHideButton() const
{
    return m_isAutoHide;
}

QColor FancyLineEdit::buttonColor() const
{
    return m_buttonColor;
}
void FancyLineEdit::setButtonColor(const QColor &color)
{
    m_buttonColor = color;
}
