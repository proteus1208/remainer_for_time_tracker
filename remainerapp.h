#ifndef REMAINERAPP_H
#define REMAINERAPP_H

#include <QObject>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QTimer>
#include <QElapsedTimer>
#include <QPoint>
#include <QList>
#include <QScreen>
#include <QDateTime>
#include <QSettings>

class EventMonitor;
class LightEffectWidget;
class SpinningCirclesWidget;
class BlastWidget;
class OpacityOverlayWidget;
class ColoredWidget;

class RemainerApp : public QObject
{
    Q_OBJECT

public:
    explicit RemainerApp(QObject *parent = nullptr);
    ~RemainerApp();

    void initialize();

private slots:
    void onActivityDetected();
    void onMouseMoved(const QPoint &position);
    void onInactivityTimer();
    void onBlastTimer();
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void onStopAction();
    void onStopOneHourAction();
    void onActiveAction();
    void onStartupAction();
    void onSettingsAction();
    void onPauseInVmToggled(bool checked);
    void onScreensChanged();

private:
    enum InactivityState {
        Normal,
        TenSeconds,
        TwentySeconds,
        OneMinute,
        FiveMinutes
    };

    void updateInactivityState();
    void showEffectForState(InactivityState state);
    void hideAllEffects();
    void showSpinningCircles();
    void hideSpinningCircles();
    void createTrayMenu();
    void updateTrayIcon();
    bool isStartupEnabled() const;
    void setStartupEnabled(bool enabled);
    void updateStartupMenuAction();
    void loadSettings();
    void saveSettings();
    bool openSettingsDialog();
    void reevaluateInactivity();
    InactivityState stateForElapsed(qint64 elapsedMs) const;
    bool isVirtualMachineForeground() const;
    QSettings appSettings() const;

    EventMonitor *m_eventMonitor;
    QSystemTrayIcon *m_trayIcon;
    QMenu *m_trayMenu;
    QAction *m_stopAction;
    QAction *m_stopOneHourAction;
    QAction *m_activeAction;
    QAction *m_startupAction;
    QAction *m_pauseInVmAction;
    QAction *m_settingsAction;

    QTimer *m_inactivityTimer;
    QElapsedTimer m_elapsedTimer;
    QTimer *m_blastTimer;

    InactivityState m_currentState;
    bool m_isActive;
    bool m_isStopped;
    QDateTime m_stopUntilTime;
    QPoint m_lastMousePosition;

    bool m_shapesEnabled;
    int m_shapesIntervalSec;
    bool m_fullscreenEnabled;
    int m_fullscreenIntervalSec;
    bool m_pauseInVm;
    bool m_settingsConfigured;

    // Effect widgets
    LightEffectWidget *m_lightEffect;
    QList<SpinningCirclesWidget*> m_spinningCircles;
    QList<BlastWidget*> m_blastWidgets;
    QList<OpacityOverlayWidget*> m_opacityOverlays;
    QList<ColoredWidget*> m_coloredWidgets;
};

#endif // REMAINERAPP_H
