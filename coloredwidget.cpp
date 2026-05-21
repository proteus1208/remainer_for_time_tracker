#include "coloredwidget.h"
#include <QPropertyAnimation>
#include <QPainter>
#include <QApplication>
#include <QEasingCurve>
#include <QTimer>

ColoredWidget::ColoredWidget(QScreen *screen, const QColor &color, QWidget *parent)
    : QWidget(parent)
    , m_opacity(0.0)
    , m_screen(screen)
    , m_color(color)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_TranslucentBackground);
    setStyleSheet("background: transparent;");

    if (m_screen) {
        QRect geometry = m_screen->geometry();
        setGeometry(geometry);
    }

    m_opacityAnimation = new QPropertyAnimation(this, "opacity", this);
    m_opacityAnimation->setDuration(500);
    m_opacityAnimation->setEasingCurve(QEasingCurve::InOutQuad);
    connect(m_opacityAnimation, &QPropertyAnimation::finished, this, [this]() {
        if (m_opacity <= 0.01) {
            // Animation finished hiding, delete widget
            QTimer::singleShot(50, this, &QWidget::deleteLater);
        }
    });
}

ColoredWidget::~ColoredWidget()
{
}

void ColoredWidget::setOpacity(qreal opacity)
{
    m_opacity = opacity;
    update();
}

void ColoredWidget::showAnimated()
{
    m_opacityAnimation->setStartValue(0.0);
    m_opacityAnimation->setEndValue(1.0);
    m_opacityAnimation->start();
    show();
}

void ColoredWidget::hideAnimated()
{
    if (m_opacityAnimation->state() == QPropertyAnimation::Running) {
        return; // Already animating
    }
    m_opacityAnimation->setStartValue(m_opacity);
    m_opacityAnimation->setEndValue(0.0);
    m_opacityAnimation->start();
    // Widget will delete itself when animation finishes (connected in constructor)
}

void ColoredWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setOpacity(m_opacity);
    painter.fillRect(rect(), m_color);
}
