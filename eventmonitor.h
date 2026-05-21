#ifndef EVENTMONITOR_H
#define EVENTMONITOR_H

#include <QObject>
#include <QTimer>
#include <QPoint>
#include <QMutex>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

class EventMonitor : public QObject
{
    Q_OBJECT

public:
    explicit EventMonitor(QObject *parent = nullptr);
    ~EventMonitor();

    bool startMonitoring();
    void stopMonitoring();
    bool isMonitoring() const { return m_monitoring; }

signals:
    void activityDetected();
    void mouseMoved(const QPoint &position);

private slots:
    void onActivity();

private:
#ifdef Q_OS_WIN
    static LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
    static EventMonitor *s_instance;
    HHOOK m_mouseHook;
    HHOOK m_keyboardHook;
    QPoint m_pendingMousePos;
    QMutex m_mutex;
#endif

    bool m_monitoring;
    QTimer *m_activityTimer;
    QPoint m_lastMousePos;
};

#endif // EVENTMONITOR_H
