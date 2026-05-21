#include "eventmonitor.h"
#include <QApplication>
#include <QScreen>
#include <QMutex>
#include <QMutexLocker>
#include <QCoreApplication>

#ifdef Q_OS_WIN
EventMonitor *EventMonitor::s_instance = nullptr;
#endif

EventMonitor::EventMonitor(QObject *parent)
    : QObject(parent)
    , m_monitoring(false)
#ifdef Q_OS_WIN
    , m_mouseHook(nullptr)
    , m_keyboardHook(nullptr)
#endif
{
    m_activityTimer = new QTimer(this);
    m_activityTimer->setSingleShot(true);
    m_activityTimer->setInterval(100); // Check every 100ms
    connect(m_activityTimer, &QTimer::timeout, this, &EventMonitor::onActivity);

#ifdef Q_OS_WIN
    s_instance = this;
    m_activityTimer->start();
#endif
}

EventMonitor::~EventMonitor()
{
    stopMonitoring();
#ifdef Q_OS_WIN
    s_instance = nullptr;
#endif
}

bool EventMonitor::startMonitoring()
{
    if (m_monitoring)
        return true;

#ifdef Q_OS_WIN
    m_mouseHook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, GetModuleHandle(nullptr), 0);
    m_keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandle(nullptr), 0);

    if (m_mouseHook && m_keyboardHook) {
        m_monitoring = true;
        return true;
    } else {
        if (m_mouseHook) {
            UnhookWindowsHookEx(m_mouseHook);
            m_mouseHook = nullptr;
        }
        if (m_keyboardHook) {
            UnhookWindowsHookEx(m_keyboardHook);
            m_keyboardHook = nullptr;
        }
        return false;
    }
#else
    // For non-Windows platforms, use Qt's event filter
    qApp->installEventFilter(this);
    m_monitoring = true;
    return true;
#endif
}

void EventMonitor::stopMonitoring()
{
    if (!m_monitoring)
        return;

#ifdef Q_OS_WIN
    if (m_mouseHook) {
        UnhookWindowsHookEx(m_mouseHook);
        m_mouseHook = nullptr;
    }
    if (m_keyboardHook) {
        UnhookWindowsHookEx(m_keyboardHook);
        m_keyboardHook = nullptr;
    }
#else
    qApp->removeEventFilter(this);
#endif

    m_monitoring = false;
}

void EventMonitor::onActivity()
{
    emit activityDetected();
    
#ifdef Q_OS_WIN
    // Check for mouse position update
    QMutexLocker locker(&m_mutex);
    if (m_pendingMousePos != m_lastMousePos) {
        emit mouseMoved(m_pendingMousePos);
        m_lastMousePos = m_pendingMousePos;
    }
#endif
}

#ifdef Q_OS_WIN
LRESULT CALLBACK EventMonitor::LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode >= 0 && s_instance) {
        MSLLHOOKSTRUCT *pMouseStruct = (MSLLHOOKSTRUCT *)lParam;
        QPoint pos(pMouseStruct->pt.x, pMouseStruct->pt.y);
        
        if (wParam == WM_MOUSEMOVE) {
            QMutexLocker locker(&s_instance->m_mutex);
            s_instance->m_pendingMousePos = pos;
        }
        
        // Trigger activity check
        s_instance->m_activityTimer->stop();
        s_instance->m_activityTimer->start();
    }
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

LRESULT CALLBACK EventMonitor::LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode >= 0 && s_instance) {
        // Trigger activity check
        s_instance->m_activityTimer->stop();
        s_instance->m_activityTimer->start();
    }
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}
#endif
