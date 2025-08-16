#pragma once

#include <qdbuspendingreply.h>
#include <qjsonvalue.h>

#include <QDBusInterface>
#include <QDBusPendingReply>
#include <QDBusReply>
#include <QJsonObject>
#include <QMutex>

// JSON Bridge class that wraps the WindowManager
class WindowManagerJsonBridge : public QObject {
    Q_OBJECT

 public:
    explicit WindowManagerJsonBridge(QObject* parent = nullptr);

 public slots:
    void processJsonInput(const QString& json);

    // Convert incoming signals to JSON and output to stdout
    void windowDesktopsChanged(const QString& window_uuid,
                               const QStringList& desktop_ids);
    void windowActivitiesChanged(const QString& window_uuid,
                                 const QStringList& activities_uuids);
    void activityChanged(const QString& activity_uuid);
    void desktopChanged(const QString& desktop_id);

 private slots:
    void outputJson(const QJsonObject& json);

 private:
    QDBusConnection m_dbusConnection;
    QDBusInterface* m_dbusInterface;
    void setupStdinReader();
    template <typename ReturnType, typename... Args>
    void callMethod(int id, const QString& method, const Args&... args);
    void sendSignal(const QString& signal, const QJsonArray& params);

    void connectToSignals();

    QMutex m_stdout_mutex;
};
