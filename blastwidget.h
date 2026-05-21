#ifndef BLASTWIDGET_H
#define BLASTWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QPropertyAnimation>
#include <QPoint>

class BlastWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
    Q_PROPERTY(qreal scale READ scale WRITE setScale)

public:
    explicit BlastWidget(const QPoint &position, QWidget *parent = nullptr);
    ~BlastWidget();

    qreal opacity() const { return m_opacity; }
    void setOpacity(qreal opacity);
    qreal scale() const { return m_scale; }
    void setScale(qreal scale);

    void startAnimation();

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void onAnimationFinished();

private:
    qreal m_opacity;
    qreal m_scale;
    QPropertyAnimation *m_opacityAnimation;
    QPropertyAnimation *m_scaleAnimation;
    QPoint m_blastPosition;
};

#endif // BLASTWIDGET_H
