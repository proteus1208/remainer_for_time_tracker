#ifndef LIGHTEFFECTWIDGET_H
#define LIGHTEFFECTWIDGET_H

#include <QWidget>
#include <QPropertyAnimation>
#include <QTimer>

class LightEffectWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
    Q_PROPERTY(qreal pulseScale READ pulseScale WRITE setPulseScale)

public:
    explicit LightEffectWidget(QWidget *parent = nullptr);
    ~LightEffectWidget();

    qreal opacity() const { return m_opacity; }
    void setOpacity(qreal opacity);
    qreal pulseScale() const { return m_pulseScale; }
    void setPulseScale(qreal scale);

    void showAnimated();
    void hideAnimated();

signals:
    void animationFinished();

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void onFadeOutFinished();

private:
    qreal m_opacity;
    qreal m_pulseScale;
    QPropertyAnimation *m_opacityAnimation;
    QPropertyAnimation *m_pulseAnimation;
    QTimer *m_updateTimer;
};

#endif // LIGHTEFFECTWIDGET_H
