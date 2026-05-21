#ifndef OPACITYOVERLAYWIDGET_H
#define OPACITYOVERLAYWIDGET_H

#include <QWidget>
#include <QPropertyAnimation>
#include <QScreen>

class OpacityOverlayWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)

public:
    explicit OpacityOverlayWidget(QScreen *screen, QWidget *parent = nullptr);
    ~OpacityOverlayWidget();

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
};

#endif // OPACITYOVERLAYWIDGET_H
