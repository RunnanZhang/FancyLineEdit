#ifndef FANCYLINEEDIT_H
#define FANCYLINEEDIT_H

#include <QLineEdit>

class QLabel;
class QGraphicsOpacityEffect;

class FancyLineEdit : public QLineEdit
{
    Q_OBJECT

    Q_PROPERTY(ButtonMode buttonMode READ buttonMode WRITE setButtonMode)
    Q_PROPERTY(QPixmap buttonPixmap READ buttonPixmap WRITE setButtonPixmap)
    Q_PROPERTY(QColor buttonColor READ buttonColor WRITE setButtonColor)
    Q_PROPERTY(bool isAutoHide READ isAutoHideButton WRITE setAutoHideButton)

public:
    explicit FancyLineEdit(QWidget *parent = 0);

public:
    /// @brief 设置按钮的显示模式，自己绘制和加载pixmap.
    enum ButtonMode { Paint, Image };
    Q_ENUM(ButtonMode)
    ButtonMode buttonMode() const;
    void setButtonMode(ButtonMode);

    /// @brief 设置button的pixmap，仅在ButtonMode::Image模式下生效.
    QPixmap buttonPixmap() const;
    void setButtonPixmap(const QPixmap &pixmap);

    /// @brief 设置button的颜色，仅在ButtonMode::Paint模式下生效.
    QColor buttonColor() const;
    void setButtonColor(const QColor &color);

    /// @brief Set if button should be hidden when text is empty.
    void setAutoHideButton(bool isAutoHide);
    bool isAutoHideButton() const;

    /// @brief Under AutoHide mode, show/hide button with animation.
    void animateShow(bool visible);

    /// @brief Set default text, the default is empty.
    void setDefaultText(const QString &text);

    /// @brief set button's ToolTip.
    void setButtonToolTip(const QString &tip);

protected:
    virtual bool event(QEvent *e);
    virtual bool eventFilter(QObject *obj, QEvent *e);

private:
    bool m_bHovered;
    QString m_defaultText;
    QLabel *m_label;
    QColor m_backgroundColor;
    QColor m_buttonColor;
    ButtonMode m_buttonMode;
    QPixmap m_buttonPixmap;
    bool m_isAutoHide;
    QGraphicsOpacityEffect *m_opacityEffect;
};

#endif // FANCYLINEEDIT_H
