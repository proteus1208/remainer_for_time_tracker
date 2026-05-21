#include "blastwidget.h"
#include <QPainter>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QScreen>
#include <QApplication>
#include <cmath>

BlastWidget::BlastWidget(const QPoint &position, QWidget *parent)
    : QWidget(parent)
    , m_opacity(1.0)
    , m_scale(0.0)
    , m_blastPosition(position)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_TranslucentBackground);
    setStyleSheet("background: transparent;");

    // Set widget size to 200x200 and position at mouse location
    setGeometry(position.x() - 100, position.y() - 100, 200, 200);

    m_scaleAnimation = new QPropertyAnimation(this, "scale", this);
    m_scaleAnimation->setDuration(800);
    m_scaleAnimation->setStartValue(0.0);
    m_scaleAnimation->setEndValue(1.0);
    m_scaleAnimation->setEasingCurve(QEasingCurve::OutCubic);

    m_opacityAnimation = new QPropertyAnimation(this, "opacity", this);
    m_opacityAnimation->setDuration(1000);
    m_opacityAnimation->setStartValue(1.0);
    m_opacityAnimation->setEndValue(0.0);
    m_opacityAnimation->setEasingCurve(QEasingCurve::InQuad);

    connect(m_opacityAnimation, &QPropertyAnimation::finished, this, &BlastWidget::onAnimationFinished);
}

BlastWidget::~BlastWidget()
{
}

void BlastWidget::setOpacity(qreal opacity)
{
    m_opacity = opacity;
    update();
}

void BlastWidget::setScale(qreal scale)
{
    m_scale = scale;
    update();
}

void BlastWidget::startAnimation()
{
    m_scaleAnimation->start();
    m_opacityAnimation->start();
}

void BlastWidget::onAnimationFinished()
{
    deleteLater();
}

void BlastWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setOpacity(m_opacity);

    int centerX = width() / 2;
    int centerY = height() / 2;
    int maxRadius = 80;

    // Draw beautiful blast effect with multiple expanding circles
    for (int i = 0; i < 5; ++i) {
        qreal radius = (maxRadius * m_scale) * (1.0 - i * 0.15);
        qreal alpha = (1.0 - i * 0.2) * m_opacity;

        if (radius > 0 && alpha > 0) {
            QColor color;
            // Color gradient from center to edge
            if (i == 0) {
                color = QColor(255, 200, 100, int(alpha * 255));
            } else if (i == 1) {
                color = QColor(255, 150, 50, int(alpha * 200));
            } else if (i == 2) {
                color = QColor(255, 100, 0, int(alpha * 150));
            } else if (i == 3) {
                color = QColor(200, 50, 0, int(alpha * 100));
            } else {
                color = QColor(150, 0, 0, int(alpha * 50));
            }

            painter.setBrush(QBrush(color));
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(QPoint(centerX, centerY), int(radius), int(radius));
        }
    }

    // Draw particles
    int particleCount = 20;
    for (int i = 0; i < particleCount; ++i) {
        qreal angle = (i * 360.0 / particleCount) * M_PI / 180.0;
        qreal distance = maxRadius * m_scale * 0.8;
        int x = centerX + int(distance * qCos(angle));
        int y = centerY + int(distance * qSin(angle));

        qreal particleAlpha = m_opacity * (1.0 - m_scale * 0.5);
        painter.setBrush(QColor(255, 255, 200, int(particleAlpha * 200)));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(QPoint(x, y), 3, 3);
    }
}
