#ifndef SPINNINGCIRCLESWIDGET_H
#define SPINNINGCIRCLESWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QPropertyAnimation>

class SpinningCirclesWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal rotationAngle READ rotationAngle WRITE setRotationAngle)
    Q_PROPERTY(qreal colorRotationAngle READ colorRotationAngle WRITE setColorRotationAngle)
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)

public:
    explicit SpinningCirclesWidget(QWidget *parent = nullptr);
    ~SpinningCirclesWidget();

    qreal rotationAngle() const { return m_rotationAngle; }
    void setRotationAngle(qreal angle);
    qreal colorRotationAngle() const { return m_colorRotationAngle; }
    void setColorRotationAngle(qreal angle);
    qreal opacity() const { return m_opacity; }
    void setOpacity(qreal opacity);

    void hideAnimated();

signals:
    void animationFinished();

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void onFadeOutFinished();

private:
    qreal m_rotationAngle;
    qreal m_colorRotationAngle;
    qreal m_opacity;
    QPropertyAnimation *m_rotationAnimation;
    QPropertyAnimation *m_colorRotationAnimation;
    QPropertyAnimation *m_fadeOutAnimation;
    QTimer *m_updateTimer;
};

#endif // SPINNINGCIRCLESWIDGET_H
