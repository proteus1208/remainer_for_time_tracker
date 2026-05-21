#include "remainerapp.h"
#include "eventmonitor.h"
#include "lighteffectwidget.h"
#include "spinningcircleswidget.h"
#include "blastwidget.h"
#include "opacityoverlaywidget.h"
#include "coloredwidget.h"
#include <QApplication>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QTimer>
#include <QElapsedTimer>
#include <QDateTime>
#include <QScreen>
#include <QMessageBox>
#include <QIcon>
#include <QPainter>
#include <QPixmap>
#include <QAction>
#include <QSettings>
#include <QDir>
#include <QStandardPaths>
#include <QCoreApplication>

// ============================================================================
// TIME CONFIGURATION - Adjust these values to change inactivity thresholds
// ============================================================================
// All times are in milliseconds
static const qint64 TIME_TEN_SECONDS = 7000;        // 7 seconds - Spinning circles start (normal state ends)
static const qint64 TIME_TWENTY_SECONDS = 30000;     // 20 seconds - Blast effect (DISABLED)
static const qint64 TIME_ONE_MINUTE = 60000;         // 1 minute - Opacity overlay (DISABLED)
static const qint64 TIME_FIVE_MINUTES = 37000;      // 37 seconds - Colored widgets start (spinning state ends after 30s)
static const int BLAST_INTERVAL_MS = 1000;           // Blast effect interval (1 second) - DISABLED


// All times are in milliseconds
// static const qint64 TIME_TEN_SECONDS = 5000;        // 10 seconds - Spinning circles
// static const qint64 TIME_TWENTY_SECONDS = 30000;     // 20 seconds - Blast effect
// static const qint64 TIME_ONE_MINUTE = 60000;         // 1 minute - Opacity overlay
// static const qint64 TIME_FIVE_MINUTES = 120000;      // 5 minutes - Colored widgets
// static const int BLAST_INTERVAL_MS = 1000;           // Blast effect interval (1 second)
// ============================================================================

RemainerApp::RemainerApp(QObject *parent)
    : QObject(parent)
    , m_eventMonitor(nullptr)
    , m_trayIcon(nullptr)
    , m_trayMenu(nullptr)
    , m_stopAction(nullptr)
    , m_stopOneHourAction(nullptr)
    , m_activeAction(nullptr)
    , m_inactivityTimer(nullptr)
    , m_blastTimer(nullptr)
    , m_currentState(Normal)
    , m_isActive(true)
    , m_isStopped(false)
    , m_lightEffect(nullptr)
    , m_spinningCircles(nullptr)
{
    m_inactivityTimer = new QTimer(this);
    m_inactivityTimer->setSingleShot(false);
    m_inactivityTimer->setInterval(100); // Check every 100ms
    connect(m_inactivityTimer, &QTimer::timeout, this, &RemainerApp::onInactivityTimer);

    m_blastTimer = new QTimer(this);
    m_blastTimer->setSingleShot(false);
    m_blastTimer->setInterval(BLAST_INTERVAL_MS);
    connect(m_blastTimer, &QTimer::timeout, this, &RemainerApp::onBlastTimer);

    m_elapsedTimer.start();
}

RemainerApp::~RemainerApp()
{
    hideAllEffects();
}

void RemainerApp::initialize()
{
    // Create event monitor
    m_eventMonitor = new EventMonitor(this);
    connect(m_eventMonitor, &EventMonitor::activityDetected, this, &RemainerApp::onActivityDetected);
    connect(m_eventMonitor, &EventMonitor::mouseMoved, this, &RemainerApp::onMouseMoved);

    if (!m_eventMonitor->startMonitoring()) {
        QMessageBox::critical(nullptr, "Error", "Failed to start event monitoring!");
        return;
    }

    // Create system tray
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        QMessageBox::critical(nullptr, "Error", "System tray is not available!");
        return;
    }

    createTrayMenu();

    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setContextMenu(m_trayMenu);
    m_trayIcon->setToolTip("Remainer - Activity Monitor");
    connect(m_trayIcon, &QSystemTrayIcon::activated, this, &RemainerApp::onTrayIconActivated);
    
    // Try to load icon from file
    QString iconPath = QCoreApplication::applicationDirPath() + "/icon.ico";
    QIcon trayIcon(iconPath);
    
    // If not found in app dir, try current directory (for development)
    if (trayIcon.isNull() || trayIcon.availableSizes().isEmpty()) {
        trayIcon = QIcon("icon.ico");
    }
    
    // If still not found, try resource path
    if (trayIcon.isNull() || trayIcon.availableSizes().isEmpty()) {
        trayIcon = QIcon(":/icon.ico");
    }
    
    // If icon file found, use it; otherwise use generated icon
    if (!trayIcon.isNull() && !trayIcon.availableSizes().isEmpty()) {
        m_trayIcon->setIcon(trayIcon);
        QApplication::setWindowIcon(trayIcon);
    } else {
        // Fallback to generated icon
        updateTrayIcon();
    }
    
    m_trayIcon->show();

    // Start inactivity timer
    m_inactivityTimer->start();
    m_elapsedTimer.restart();
}

void RemainerApp::onActivityDetected()
{
    if (!m_isActive || m_isStopped)
        return;

    m_elapsedTimer.restart();
    hideAllEffects();
    m_currentState = Normal;
    m_blastTimer->stop();
}

void RemainerApp::onMouseMoved(const QPoint &position)
{
    m_lastMousePosition = position;
}

void RemainerApp::onInactivityTimer()
{
    if (!m_isActive || m_isStopped) {
        if (m_isStopped && !m_stopUntilTime.isNull() && QDateTime::currentDateTime() >= m_stopUntilTime) {
            m_isStopped = false;
            m_stopUntilTime = QDateTime();
            updateTrayIcon();
        }
        return;
    }

    qint64 elapsedMs = m_elapsedTimer.elapsed();
    InactivityState newState = Normal;

    if (elapsedMs >= TIME_FIVE_MINUTES) {
        newState = FiveMinutes;
    } else if (elapsedMs >= TIME_TEN_SECONDS) {
        newState = TenSeconds;
    }
    // DISABLED: Methods 3
    // } else if (elapsedMs >= TIME_ONE_MINUTE) {
    //     newState = OneMinute;
    // } else if (elapsedMs >= TIME_TWENTY_SECONDS) {
    //     newState = TwentySeconds;
    // }

    if (newState != m_currentState) {
        m_currentState = newState;
        updateInactivityState();
    }
}

void RemainerApp::onBlastTimer()
{
    if (m_currentState == TwentySeconds && m_isActive && !m_isStopped) {
        // Create new blast widget at last mouse position
        BlastWidget *blast = new BlastWidget(m_lastMousePosition);
        blast->startAnimation();
        m_blastWidgets.append(blast);
        blast->show();
        
        // Connect to remove from list when widget is destroyed
        // Note: We can't use the pointer in the lambda since it's destroyed,
        // so we'll clean up the list periodically or when transitioning states
    } else {
        // If we're not in the right state, stop the timer
        if (m_currentState != TwentySeconds) {
            m_blastTimer->stop();
        }
    }
}

void RemainerApp::updateInactivityState()
{
    hideAllEffects();

    if (!m_isActive || m_isStopped)
        return;

    showEffectForState(m_currentState);
}

void RemainerApp::showEffectForState(InactivityState state)
{
    switch (state) {
    case Normal:
        // Show light effect for normal state
        if (!m_lightEffect) {
            m_lightEffect = new LightEffectWidget();
            m_lightEffect->showAnimated();
        } else if (!m_lightEffect->isVisible()) {
            m_lightEffect->showAnimated();
        }
        break;

    case TenSeconds:
        // Hide light effect smoothly
        if (m_lightEffect) {
            LightEffectWidget *light = m_lightEffect;
            m_lightEffect = nullptr;
            connect(light, &LightEffectWidget::animationFinished, 
                    light, &QWidget::deleteLater, Qt::QueuedConnection);
            light->hideAnimated();
        }
        
        if (!m_spinningCircles) {
            m_spinningCircles = new SpinningCirclesWidget();
            m_spinningCircles->show();
        } else if (!m_spinningCircles->isVisible()) {
            // Widget exists but was hidden, show it again
            m_spinningCircles->show();
        }
        break;

    // DISABLED: Method 3 - TwentySeconds (Blast effect)
    // case TwentySeconds:
    //     if (m_spinningCircles) {
    //         SpinningCirclesWidget *spinning = m_spinningCircles;
    //         m_spinningCircles = nullptr;
    //         connect(spinning, &SpinningCirclesWidget::animationFinished, 
    //                 spinning, &QWidget::deleteLater, Qt::QueuedConnection);
    //         spinning->hideAnimated();
    //     }
    //     m_blastTimer->start();
    //     onBlastTimer(); // Show first blast immediately
    //     break;

    // DISABLED: Method 3 - OneMinute (Opacity overlay)
    // case OneMinute: {
    //     m_blastTimer->stop();
    //     // Blast widgets will fade out on their own, just clear the list
    //     // They delete themselves when animation completes
    //     m_blastWidgets.clear();
    //
    //     // Create opacity overlays for all screens
    //     QList<QScreen*> screens = QApplication::screens();
    //     for (QScreen *screen : screens) {
    //         OpacityOverlayWidget *overlay = new OpacityOverlayWidget(screen);
    //         overlay->showAnimated();
    //         m_opacityOverlays.append(overlay);
    //     }
    //     break;
    // }

    case FiveMinutes: {
        // Hide light effect when transitioning to 5 minutes
        if (m_lightEffect) {
            LightEffectWidget *light = m_lightEffect;
            m_lightEffect = nullptr;
            connect(light, &LightEffectWidget::animationFinished, 
                    light, &QWidget::deleteLater, Qt::QueuedConnection);
            light->hideAnimated();
        }

        // Hide spinning circles when transitioning to 5 minutes
        if (m_spinningCircles) {
            SpinningCirclesWidget *spinning = m_spinningCircles;
            m_spinningCircles = nullptr;
            connect(spinning, &SpinningCirclesWidget::animationFinished, 
                    spinning, &QWidget::deleteLater, Qt::QueuedConnection);
            spinning->hideAnimated();
        }

        // DISABLED: Method 3 - Opacity overlays cleanup (not used anymore)
        // // Hide opacity overlays
        // for (OpacityOverlayWidget *overlay : m_opacityOverlays) {
        //     if (overlay) {
        //         overlay->hideAnimated();
        //         overlay->deleteLater();
        //     }
        // }
        // m_opacityOverlays.clear();

        // Create colored widgets for all screens
        QList<QScreen*> screens2 = QApplication::screens();
        QList<QColor> colors = {QColor(255, 100, 100), QColor(100, 255, 100), 
                                QColor(100, 100, 255), QColor(255, 255, 100)};
        int colorIndex = 0;
        for (QScreen *screen : screens2) {
            QColor color = colors[colorIndex % colors.size()];
            ColoredWidget *colored = new ColoredWidget(screen, color);
            colored->showAnimated();
            m_coloredWidgets.append(colored);
            colorIndex++;
        }
        break;
    }
    }
}

void RemainerApp::hideAllEffects()
{
    // Hide light effect with smooth fade-out
    if (m_lightEffect) {
        LightEffectWidget *light = m_lightEffect;
        m_lightEffect = nullptr;
        connect(light, &LightEffectWidget::animationFinished, 
                light, &QWidget::deleteLater, Qt::QueuedConnection);
        light->hideAnimated();
    }

    // Hide spinning circles with smooth fade-out
    if (m_spinningCircles) {
        SpinningCirclesWidget *spinning = m_spinningCircles;
        m_spinningCircles = nullptr; // Clear pointer immediately to prevent re-entry
        connect(spinning, &SpinningCirclesWidget::animationFinished, 
                spinning, &QWidget::deleteLater, Qt::QueuedConnection);
        spinning->hideAnimated();
    }

    // DISABLED: Method 3 - Blast effect cleanup
    // // Stop blast timer
    // m_blastTimer->stop();
    // 
    // // Blast widgets already have fade-out animation built-in
    // // They will delete themselves when animation completes (in onAnimationFinished)
    // // Just remove from list to prevent access, but let them finish animating
    // QList<BlastWidget*> blastsToRemove = m_blastWidgets;
    // m_blastWidgets.clear();
    // // Widgets will clean themselves up when their animations finish

    // DISABLED: Method 3 - Opacity overlay cleanup
    // // Hide opacity overlays with smooth fade-out
    // QList<OpacityOverlayWidget*> overlaysToRemove = m_opacityOverlays;
    // m_opacityOverlays.clear();
    // for (OpacityOverlayWidget *overlay : overlaysToRemove) {
    //     if (overlay) {
    //         overlay->hideAnimated();
    //         // Widget will delete itself after animation completes
    //     }
    // }

    // Hide colored widgets with smooth fade-out
    QList<ColoredWidget*> coloredToRemove = m_coloredWidgets;
    m_coloredWidgets.clear();
    for (ColoredWidget *colored : coloredToRemove) {
        if (colored) {
            colored->hideAnimated();
            // Widget will delete itself after animation completes
        }
    }
}

void RemainerApp::createTrayMenu()
{
    m_trayMenu = new QMenu();

    m_activeAction = m_trayMenu->addAction("Active");
    m_activeAction->setCheckable(true);
    m_activeAction->setChecked(true);
    connect(m_activeAction, &QAction::triggered, this, &RemainerApp::onActiveAction);

    m_trayMenu->addSeparator();

    m_stopOneHourAction = m_trayMenu->addAction("Stop for 1 Hour");
    connect(m_stopOneHourAction, &QAction::triggered, this, &RemainerApp::onStopOneHourAction);

    m_stopAction = m_trayMenu->addAction("Stop Completely");
    connect(m_stopAction, &QAction::triggered, this, &RemainerApp::onStopAction);

    m_trayMenu->addSeparator();

    m_startupAction = m_trayMenu->addAction("Set as Startup Program");
    m_startupAction->setCheckable(true);
    connect(m_startupAction, &QAction::triggered, this, &RemainerApp::onStartupAction);
    updateStartupMenuAction();

    m_trayMenu->addSeparator();
    m_trayMenu->addAction("Quit", qApp, &QApplication::quit);
}

void RemainerApp::updateTrayIcon()
{
    if (!m_trayIcon)
        return;

    // Try to load icon from file first
    QString iconPath = QCoreApplication::applicationDirPath() + "/icon.ico";
    QIcon trayIcon(iconPath);
    
    // If not found in app dir, try current directory (for development)
    if (trayIcon.isNull() || trayIcon.availableSizes().isEmpty()) {
        trayIcon = QIcon("icon.ico");
    }
    
    // If icon file found, use it with status overlay
    if (!trayIcon.isNull() && !trayIcon.availableSizes().isEmpty()) {
        // Use the icon file as base
        QPixmap basePixmap = trayIcon.pixmap(32, 32);
        if (!basePixmap.isNull()) {
            // Add status indicator overlay
            QPainter painter(&basePixmap);
            painter.setRenderHint(QPainter::Antialiasing);
            
            // Draw small status dot in corner
            QColor statusColor;
            if (m_isStopped) {
                statusColor = QColor(200, 50, 50);
            } else if (!m_isActive) {
                statusColor = QColor(200, 200, 50);
            } else {
                statusColor = QColor(50, 200, 50);
            }
            
            painter.setBrush(statusColor);
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(20, 20, 10, 10); // Small dot in bottom-right corner
            
            m_trayIcon->setIcon(QIcon(basePixmap));
            return;
        }
    }
    
    // Fallback: Create a simple generated icon
    QPixmap pixmap(32, 32);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    if (m_isStopped) {
        painter.setBrush(QColor(200, 50, 50));
    } else if (!m_isActive) {
        painter.setBrush(QColor(200, 200, 50));
    } else {
        painter.setBrush(QColor(50, 200, 50));
    }
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(4, 4, 24, 24);

    m_trayIcon->setIcon(QIcon(pixmap));
}

void RemainerApp::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::DoubleClick) {
        // Toggle active state
        m_isActive = !m_isActive;
        m_activeAction->setChecked(m_isActive);
        updateTrayIcon();
        if (!m_isActive) {
            hideAllEffects();
        }
    }
}

void RemainerApp::onStopAction()
{
    m_isStopped = true;
    m_isActive = false;
    m_stopUntilTime = QDateTime();
    m_activeAction->setChecked(false);
    hideAllEffects();
    updateTrayIcon();
}

void RemainerApp::onStopOneHourAction()
{
    m_isStopped = true;
    m_isActive = false;
    m_stopUntilTime = QDateTime::currentDateTime().addSecs(3600);
    m_activeAction->setChecked(false);
    hideAllEffects();
    updateTrayIcon();
}

void RemainerApp::onActiveAction()
{
    m_isStopped = false;
    m_isActive = true;
    m_stopUntilTime = QDateTime();
    m_activeAction->setChecked(true);
    m_elapsedTimer.restart();
    updateTrayIcon();
}

bool RemainerApp::isStartupEnabled() const
{
#ifdef Q_OS_WIN
    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    QString appName = "Remainer";
    return settings.contains(appName);
#else
    // For non-Windows, check startup folder or other platform-specific method
    return false;
#endif
}

void RemainerApp::setStartupEnabled(bool enabled)
{
#ifdef Q_OS_WIN
    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    QString appName = "Remainer";
    
    if (enabled) {
        // Get the executable path
        QString appPath = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
        settings.setValue(appName, appPath);
    } else {
        settings.remove(appName);
    }
    settings.sync();
    
    updateStartupMenuAction();
#else
    Q_UNUSED(enabled);
    // Platform-specific implementation for other OS
#endif
}

void RemainerApp::updateStartupMenuAction()
{
    if (m_startupAction) {
        bool enabled = isStartupEnabled();
        m_startupAction->setChecked(enabled);
        m_startupAction->setText(enabled ? "Remove from Startup" : "Set as Startup Program");
    }
}

void RemainerApp::onStartupAction()
{
    bool currentlyEnabled = isStartupEnabled();
    setStartupEnabled(!currentlyEnabled);
}
