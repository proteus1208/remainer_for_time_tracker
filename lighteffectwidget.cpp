#include "lighteffectwidget.h"
#include <QPainter>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QScreen>
#include <QApplication>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

LightEffectWidget::LightEffectWidget(QWidget *parent)
    : QWidget(parent)
    , m_opacity(0.0)
    , m_pulseScale(1.0)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_TranslucentBackground);
    setStyleSheet("background: transparent;");

    // Get screen size and position at center
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    setGeometry(screenGeometry.width() / 2 - 150, screenGeometry.height() / 2 - 150, 300, 300);

    m_opacityAnimation = new QPropertyAnimation(this, "opacity", this);
    m_opacityAnimation->setDuration(300);
    m_opacityAnimation->setEasingCurve(QEasingCurve::InOutQuad);

    m_pulseAnimation = new QPropertyAnimation(this, "pulseScale", this);
    m_pulseAnimation->setDuration(2000);
    m_pulseAnimation->setStartValue(0.8);
    m_pulseAnimation->setEndValue(1.2);
    m_pulseAnimation->setLoopCount(-1);
    m_pulseAnimation->setEasingCurve(QEasingCurve::InOutSine);
    m_pulseAnimation->start();

    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, QOverload<>::of(&QWidget::update));
    m_updateTimer->start(16); // ~60 FPS
}

LightEffectWidget::~LightEffectWidget()
{
}

void LightEffectWidget::setOpacity(qreal opacity)
{
    m_opacity = opacity;
    update();
}

void LightEffectWidget::setPulseScale(qreal scale)
{
    m_pulseScale = scale;
    update();
}

void LightEffectWidget::showAnimated()
{
    if (m_opacityAnimation->state() == QPropertyAnimation::Running) {
        return; // Already animating
    }
    m_opacityAnimation->setStartValue(0.0);
    m_opacityAnimation->setEndValue(0.15); // Very light, subtle effect
    m_opacityAnimation->start();
    show();
}

void LightEffectWidget::hideAnimated()
{
    if (m_opacityAnimation->state() == QPropertyAnimation::Running) {
        return; // Already animating
    }
    m_opacityAnimation->setStartValue(m_opacity);
    m_opacityAnimation->setEndValue(0.0);
    m_opacityAnimation->start();
    connect(m_opacityAnimation, &QPropertyAnimation::finished, this, &LightEffectWidget::onFadeOutFinished, Qt::UniqueConnection);
}

void LightEffectWidget::onFadeOutFinished()
{
    hide();
    emit animationFinished();
}

void LightEffectWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setOpacity(m_opacity);

    int centerX = width() / 2;
    int centerY = height() / 2;
    int baseRadius = 80;

    // Draw soft, pulsing light effect with multiple layers
    for (int i = 0; i < 3; ++i) {
        qreal radius = baseRadius * m_pulseScale * (1.0 - i * 0.3);
        qreal alpha = (0.3 - i * 0.1) * m_opacity;

        if (radius > 0 && alpha > 0) {
            QRadialGradient gradient(centerX, centerY, radius);
            gradient.setColorAt(0, QColor(200, 220, 255, int(alpha * 255)));
            gradient.setColorAt(0.5, QColor(150, 180, 255, int(alpha * 150)));
            gradient.setColorAt(1, QColor(100, 140, 255, 0));

            painter.setBrush(QBrush(gradient));
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(QPoint(centerX, centerY), int(radius), int(radius));
        }
    }

    // Draw small particles floating around
    int particleCount = 12;
    for (int i = 0; i < particleCount; ++i) {
        qreal angle = (i * 360.0 / particleCount + m_pulseScale * 30) * M_PI / 180.0;
        qreal distance = baseRadius * 0.6 * m_pulseScale;
        int x = centerX + int(distance * qCos(angle));
        int y = centerY + int(distance * qSin(angle));

        qreal particleAlpha = m_opacity * 0.4;
        painter.setBrush(QColor(255, 255, 255, int(particleAlpha * 200)));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(QPoint(x, y), 2, 2);
    }
}
