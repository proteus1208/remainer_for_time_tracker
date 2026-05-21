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
    void createTrayMenu();
    void updateTrayIcon();
    bool isStartupEnabled() const;
    void setStartupEnabled(bool enabled);
    void updateStartupMenuAction();

    EventMonitor *m_eventMonitor;
    QSystemTrayIcon *m_trayIcon;
    QMenu *m_trayMenu;
    QAction *m_stopAction;
    QAction *m_stopOneHourAction;
    QAction *m_activeAction;
    QAction *m_startupAction;

    QTimer *m_inactivityTimer;
    QElapsedTimer m_elapsedTimer;
    QTimer *m_blastTimer;

    InactivityState m_currentState;
    bool m_isActive;
    bool m_isStopped;
    QDateTime m_stopUntilTime;
    QPoint m_lastMousePosition;

    // Effect widgets
    LightEffectWidget *m_lightEffect;
    SpinningCirclesWidget *m_spinningCircles;
    QList<BlastWidget*> m_blastWidgets;
    QList<OpacityOverlayWidget*> m_opacityOverlays;
    QList<ColoredWidget*> m_coloredWidgets;
};

#endif // REMAINERAPP_H
