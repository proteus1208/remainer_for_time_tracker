#ifndef COLOREDWIDGET_H
#define COLOREDWIDGET_H

#include <QWidget>
#include <QPropertyAnimation>
#include <QScreen>
#include <QColor>

class ColoredWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)

public:
    explicit ColoredWidget(QScreen *screen, const QColor &color, QWidget *parent = nullptr);
    ~ColoredWidget();

    qreal opacity() const { return m_opacity; }
    void setOpacity(qreal opacity);

    void showAnimated();
    void hideAnimated();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    qreal m_opacity;
    QPropertyAnimation *m_opacityAnimation;
    QScreen *m_screen;
    QColor m_color;
};

#endif // COLOREDWIDGET_H
