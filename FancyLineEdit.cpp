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

class FancyLineEditPrivate
{
    Q_DECLARE_PUBLIC(FancyLineEdit)

public:
    FancyLineEditPrivate(FancyLineEdit *parent)
        : q_ptr(parent),
          m_bHovered(false),
          m_backgroundColor(QColor(255, 255, 255)),
          m_buttonColor(Qt::gray),
          m_buttonMode(FancyLineEdit::ButtonMode::Paint)
    {}

    bool m_bHovered;
    QString m_defaultText;
    QLabel *m_label;
    QColor m_backgroundColor;
    QColor m_buttonColor;
    FancyLineEdit::ButtonMode m_buttonMode;
    QPixmap m_buttonPixmap;
    bool m_isAutoHide;
    QGraphicsOpacityEffect *m_opacityEffect;

    /// @brief Under AutoHide mode, show/hide button with animation.
    void animateShow(bool visible);

    FancyLineEdit * const q_ptr;
};

FancyLineEdit::FancyLineEdit(QWidget *parent)
    : QLineEdit(parent),
      d_ptr(new FancyLineEditPrivate(this))
{
    Q_D(FancyLineEdit);

    d->m_label = new QLabel(this);
    d->m_label->installEventFilter(this);

    d->m_opacityEffect = new QGraphicsOpacityEffect(d->m_label);
    d->m_label->setGraphicsEffect(d->m_opacityEffect);
    setAutoHideButton(true);

    // 不可用如下方式，否则设置失败，此方式对有父指针的widget会失败.
    //m_label->setWindowOpacity(0.5);
    d->m_label->setAttribute(Qt::WA_Hover);
    d->m_label->setCursor(QCursor(Qt::PointingHandCursor));
    d->m_label->setToolTip("Clear up to default value");

    connect(this, &QLineEdit::textChanged, this, [=](){d->animateShow(!this->text().isEmpty());});

    // 我们目前利用resize里面不断更新label的位置，也可利用下面加布局，这样位置不需要更新，但是需要设置label的FixSize.否则会被挤压.
//    QHBoxLayout *buttonLayout = new QHBoxLayout();
//    buttonLayout->setContentsMargins(0, 1, 1, 1);
//    buttonLayout->addStretch();
//    buttonLayout->addWidget(m_label);
//    this->setLayout(buttonLayout);
}

FancyLineEdit::~FancyLineEdit()
{

}

void FancyLineEdit::setDefaultText(const QString &text)
{
    Q_D(FancyLineEdit);
    d->m_defaultText = text;
}

bool FancyLineEdit::event(QEvent *e)
{
    Q_D(FancyLineEdit);

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

        d->m_label->setGeometry(width - height, 1, buttonSize, buttonSize);

        // 设置输入框中文件输入区，不让输入的文字在被隐藏在按钮下.
        this->setTextMargins(0, 1, buttonSize, 1);
    }

    return QLineEdit::event(e);
}

bool FancyLineEdit::eventFilter(QObject *obj, QEvent *e)
{
    Q_D(FancyLineEdit);

    if (e->type() == QEvent::HoverEnter)
    {
        d->m_bHovered = true;
        // 由于设置了Qt::WA_Hover属性，此处无需显示调用update().
        //update();
    }
    else if(e->type() == QEvent::HoverLeave)
    {
        d->m_bHovered = false;
    }
    else if(e->type() == QEvent::MouseButtonRelease)
    {
        this->setText(d->m_defaultText);
    }
    else if(e->type() == QEvent::Paint)
    {
        QPainter painter(d->m_label);

        if(d->m_buttonMode == FancyLineEdit::Image)
        {
            QRect backRect = d->m_label->rect();
            painter.drawPixmap(backRect.topLeft(),  d->m_buttonPixmap.scaled(backRect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        }
        else
        {
            // must use QRectF.
            QRectF backRect = d->m_label->rect();
            qreal rX = backRect.x() + backRect.width()/5;
            qreal rY = backRect.y() + backRect.height()/5;
            qreal rW = backRect.width()/5*3;
            qreal rH = backRect.height()/5*3;
            QRectF circleRect(rX, rY, rW, rH);
            painter.setRenderHint(QPainter::Antialiasing, true);

            painter.setBrush(d->m_backgroundColor);
            painter.setPen(Qt::NoPen);
            painter.drawRect(backRect);

            painter.setBrush(d->m_bHovered ? d->m_buttonColor.dark() : d->m_buttonColor);
            painter.drawEllipse(circleRect);

            painter.setPen(QPen(Qt::white, 2));
            rX = backRect.width()*0.4;
            rY = backRect.width()*0.6;
            painter.drawLine(QPointF(rX, rX), QPointF(rY, rY));
            painter.drawLine(QPointF(rY, rX), QPointF(rX, rY));
        }
    }

    return QLineEdit::eventFilter(obj, e);
}

void FancyLineEditPrivate::animateShow(bool visible)
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
    Q_D(const FancyLineEdit);
    return d->m_buttonMode;
}

void FancyLineEdit::setButtonMode(ButtonMode mode)
{
    Q_D(FancyLineEdit);
    d->m_buttonMode = mode;
}

QPixmap FancyLineEdit::buttonPixmap() const
{
    Q_D(const FancyLineEdit);
    return d->m_buttonPixmap;
}

void FancyLineEdit::setButtonPixmap(const QPixmap &pixmap)
{
    Q_D(FancyLineEdit);
    d->m_buttonPixmap = pixmap;
}

void FancyLineEdit::setButtonToolTip(const QString &tip)
{
    Q_D(FancyLineEdit);
    d->m_label->setToolTip(tip);
}

void FancyLineEdit::setAutoHideButton(bool isAutoHide)
{
    Q_D(FancyLineEdit);

    d->m_isAutoHide = isAutoHide;

    d->m_opacityEffect->setOpacity(1);
    d->m_label->show();

    if(d->m_isAutoHide && d->m_label->text().isEmpty())
    {
        // 此处虽然不透明度为0，也看不到，但是我们下面设置了QCursor样式，虽透明但仍可触发，所以此处有隐藏必要，而不是仅视觉上不可见.
        d->m_opacityEffect->setOpacity(0);
        d->m_label->hide();
    }
}

bool FancyLineEdit::isAutoHideButton() const
{
    Q_D(const FancyLineEdit);
    return d->m_isAutoHide;
}

QColor FancyLineEdit::buttonColor() const
{
    Q_D(const FancyLineEdit);
    return d->m_buttonColor;
}

void FancyLineEdit::setButtonColor(const QColor &color)
{
    Q_D(FancyLineEdit);
    d->m_buttonColor = color;
}
