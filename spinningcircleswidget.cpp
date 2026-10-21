#include "spinningcircleswidget.h"
#include <QPainter>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QScreen>
#include <QApplication>
#include <QtGlobal>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

SpinningCirclesWidget::SpinningCirclesWidget(QScreen *screen, QWidget *parent)
    : QWidget(parent)
    , m_rotationAngle(0)
    , m_colorRotationAngle(0)
    , m_opacity(1.0)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_TranslucentBackground);
    setStyleSheet("background: transparent;");

    centerOn(screen);

    // Circles rotate clockwise (CW) - positive rotation, slower for better visibility
    m_rotationAnimation = new QPropertyAnimation(this, "rotationAngle", this);
    m_rotationAnimation->setDuration(3000);
    m_rotationAnimation->setStartValue(0);
    m_rotationAnimation->setEndValue(360);
    m_rotationAnimation->setLoopCount(-1);
    m_rotationAnimation->setEasingCurve(QEasingCurve::Linear);
    m_rotationAnimation->start();

    // Colors rotate counter-clockwise (CCW) - negative rotation, faster for moving color effect
    m_colorRotationAnimation = new QPropertyAnimation(this, "colorRotationAngle", this);
    m_colorRotationAnimation->setDuration(2000);
    m_colorRotationAnimation->setStartValue(0);
    m_colorRotationAnimation->setEndValue(-360);
    m_colorRotationAnimation->setLoopCount(-1);
    m_colorRotationAnimation->setEasingCurve(QEasingCurve::Linear);
    m_colorRotationAnimation->start();

    m_fadeOutAnimation = new QPropertyAnimation(this, "opacity", this);
    m_fadeOutAnimation->setDuration(300);
    m_fadeOutAnimation->setStartValue(1.0);
    m_fadeOutAnimation->setEndValue(0.0);
    m_fadeOutAnimation->setEasingCurve(QEasingCurve::OutQuad);
    connect(m_fadeOutAnimation, &QPropertyAnimation::finished, this, &SpinningCirclesWidget::onFadeOutFinished);

    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, QOverload<>::of(&QWidget::update));
    m_updateTimer->start(16); // ~60 FPS
}

SpinningCirclesWidget::~SpinningCirclesWidget()
{
}

void SpinningCirclesWidget::setRotationAngle(qreal angle)
{
    m_rotationAngle = angle;
    update();
}

void SpinningCirclesWidget::setColorRotationAngle(qreal angle)
{
    m_colorRotationAngle = angle;
    update();
}

void SpinningCirclesWidget::setOpacity(qreal opacity)
{
    m_opacity = opacity;
    update();
}

void SpinningCirclesWidget::centerOn(QScreen *screen)
{
    if (!screen)
        screen = QApplication::primaryScreen();
    if (!screen)
        return;

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    setScreen(screen);
#endif

    // Center a 400x400 widget on this screen, including its virtual-desktop origin.
    const int size = 400;
    const QRect screenGeometry = screen->geometry();
    setGeometry(screenGeometry.x() + (screenGeometry.width() - size) / 2,
                screenGeometry.y() + (screenGeometry.height() - size) / 2,
                size,
                size);
}

void SpinningCirclesWidget::hideAnimated()
{
    if (m_fadeOutAnimation->state() != QPropertyAnimation::Running) {
        m_fadeOutAnimation->start();
    }
}

void SpinningCirclesWidget::onFadeOutFinished()
{
    hide();
    emit animationFinished();
}

void SpinningCirclesWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setOpacity(m_opacity);

    int centerX = width() / 2;
    int centerY = height() / 2;
    int radius = 120; // Scaled for 400x400 (was 30 for 200x200)
    int circleRadius = 32; // Bigger circles - doubled from 16 to 32

    painter.translate(centerX, centerY);
    
    // Rotate circles clockwise (CW) - positive rotation
    painter.rotate(m_rotationAngle);

    // Draw 8 circles in a circle pattern
    for (int i = 0; i < 8; ++i) {
        qreal circleAngle = (i * 360.0 / 8.0) * M_PI / 180.0;
        int x = radius * qCos(circleAngle);
        int y = radius * qSin(circleAngle);

        // Calculate color based on position with CCW rotation
        // Colors rotate counter-clockwise (CCW) - negative rotation
        // Convert colorRotationAngle (which goes from 0 to -360) to positive for calculation
        qreal colorAngle = (i * 360.0 / 8.0) - m_colorRotationAngle;
        if (colorAngle < 0) colorAngle += 360;
        if (colorAngle >= 360) colorAngle -= 360;
        
        // Create color based on angle (hue rotation) - colors rotate CCW
        // Use full saturation and brightness for vibrant moving colors
        int hue = int(colorAngle) % 360;
        
        // Add opacity to the circles (semi-transparent)
        int alpha = 180; // Opacity for circles (0-255, 180 = ~70% opacity)
        QColor circleColor = QColor::fromHsv(hue, 255, 255, alpha);
        
        // Draw circle with gradient for better visual effect
        QRadialGradient gradient(x, y, circleRadius);
        gradient.setColorAt(0, circleColor);
        QColor edgeColor = circleColor;
        edgeColor.setAlpha(alpha / 2); // More transparent at edges
        gradient.setColorAt(1, edgeColor);
        
        painter.setBrush(QBrush(gradient));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(QPoint(x, y), circleRadius, circleRadius);
    }
}
