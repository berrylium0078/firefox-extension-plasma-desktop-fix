#include "bridge.h"

#include <qdbuspendingcall.h>
#include <qdbuspendingreply.h>
#include <qnamespace.h>

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusReply>
#include <QDataStream>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSocketNotifier>
#include <QThread>
#include <concepts>
#include <cstdio>

#include "dbus-common.h"

WindowManagerJsonBridge::WindowManagerJsonBridge(QObject* parent)
    : QObject(parent),
      m_dbusConnection(
          QDBusConnection::connectToPeer(DBUS_ADDRESS, "peerConn")) {
    m_dbusInterface = new QDBusInterface("", OBJECT_PATH, INTERFACE_NAME,
                                         m_dbusConnection, this);

    if (!m_dbusInterface->isValid()) {
        qCritical() << "Failed to connect to D-Bus service:"
                    << m_dbusInterface->lastError().message();
        return;
    }
    connectToSignals();
    setupStdinReader();
}

void WindowManagerJsonBridge::connectToSignals() {
    // Connect to all signals from the remote WindowManager
    m_dbusConnection.connect("", OBJECT_PATH, INTERFACE_NAME,
                             "windowDesktopsChanged", this,
                             SLOT(windowDesktopsChanged(QString, QStringList)));

    m_dbusConnection.connect(
        "", OBJECT_PATH, INTERFACE_NAME, "windowActivitiesChanged", this,
        SLOT(windowActivitiesChanged(QString, QStringList)));

    m_dbusConnection.connect("", OBJECT_PATH, INTERFACE_NAME, "activityChanged",
                             this, SLOT(activityChanged(QString)));

    m_dbusConnection.connect("", OBJECT_PATH, INTERFACE_NAME, "desktopChanged",
                             this, SLOT(desktopChanged(QString)));
}

void WindowManagerJsonBridge::windowDesktopsChanged(
    const QString& window_uuid, const QStringList& desktop_ids) {
    sendSignal("windowDesktopsChanged",
               {window_uuid, QJsonArray::fromStringList(desktop_ids)});
}

void WindowManagerJsonBridge::windowActivitiesChanged(
    const QString& window_uuid, const QStringList& activities_uuids) {
    sendSignal("windowActivitiesChanged",
               {window_uuid, QJsonArray::fromStringList(activities_uuids)});
}

void WindowManagerJsonBridge::activityChanged(const QString& activity_uuid) {
    sendSignal("activityChanged", {activity_uuid});
}

void WindowManagerJsonBridge::desktopChanged(const QString& desktop_id) {
    sendSignal("desktopChanged", {desktop_id});
}

template <std::convertible_to<QJsonValue> T>
class DefaultConverter {
    DefaultConverter() {}
    QJsonValue operator()(const T& t) { return t; }
};

void WindowManagerJsonBridge::processJsonInput(const QString& jsonLine) {
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonLine.toUtf8(), &parseError);
    assert(parseError.error == QJsonParseError::NoError);
    QJsonObject json = doc.object();
    assert(json.contains("method") && json.contains("params") &&
           json.contains("id"));
    int id = json["id"].toInt();
    QString method = json["method"].toString();
    QJsonArray params = json["params"].toArray();

    // Call the appropriate D-Bus method
    if (method == "claimWindow") {
        assert(params.size() == 1 && params[0].isString());
        callMethod<QString>(id, "claimWindow", params[0].toString());
    } else if (method == "getCurrentDesktop") {
        assert(params.size() == 0);
        callMethod<QString>(id, "getCurrentDesktop");
    } else if (method == "getCurrentActivity") {
        assert(params.size() == 0);
        callMethod<QString>(id, "getCurrentActivity");
    } else if (method == "switchToActivityDesktop") {
        assert(params.size() == 2 && params[0].isString() &&
               params[1].isString());
        emit outputJson({{"debug", "before switchToActivityDesktop"}});
        callMethod<void>(id, "switchToActivityDesktop", params[0].toString(),
                         params[1].toString());
        emit outputJson({{"debug", "after switchToActivityDesktop"}});
    } else if (method == "setWindowActivities") {
        assert(params.size() == 2 && params[0].isString() &&
               params[1].isArray());

        QString window_uuid = params[0].toString();
        QStringList activities_uuids;
        for (const QJsonValue& value : params[1].toArray()) {
            assert(value.isString());
            activities_uuids.append(value.toString());
        }
        callMethod<void>(id, "setWindowActivities", window_uuid,
                         activities_uuids);

    } else if (method == "setWindowDesktops") {
        assert(params.size() == 2 && params[0].isString() &&
               params[1].isArray());

        QString window_uuid = params[0].toString();
        QStringList desktop_ids;
        for (const QJsonValue& value : params[1].toArray()) {
            assert(value.isString());
            desktop_ids.append(value.toString());
        }

        callMethod<void>(id, "setWindowDesktops", window_uuid, desktop_ids);

    } else if (method == "getWindowActivities") {
        assert(params.size() == 1 && params[0].isString());
        QString window_uuid = params[0].toString();
        callMethod<QStringList>(id, "getWindowActivities", window_uuid);
    } else if (method == "getWindowDesktops") {
        assert(params.size() == 1 && params[0].isString());
        QString window_uuid = params[0].toString();
        callMethod<QStringList>(id, "getWindowDesktops", window_uuid);
    } else {
        emit outputJson(
            {{"error", QString("Unknown method: %1").arg(method)}, {"id", id}});
    }
}
void WindowManagerJsonBridge::setupStdinReader() {
    QThread* thread = QThread::create([&]() -> void {
        quint32 rawLength;
        while (true) {
            if (fread(&rawLength, 4, 1, stdin) != 1 || rawLength <= 0) break;
            QByteArray messageData(rawLength, Qt::Uninitialized);
            if (fread(messageData.data(), 1, rawLength, stdin) != rawLength)
                break;
            emit this->processJsonInput(QString::fromUtf8(messageData));
        }
    });
    thread->start();
}

void WindowManagerJsonBridge::outputJson(const QJsonObject& json) {
    QMutexLocker locker(&m_stdout_mutex);
    QJsonDocument doc(json);
    QByteArray message = doc.toJson(QJsonDocument::Compact);
    int length = message.size();
    fwrite(&length, 4, 1, stdout);
    fwrite(message.data(), 1, length, stdout);
    fflush(stdout);
}

template <typename ReturnType, typename... Args>
void WindowManagerJsonBridge::callMethod(int id, const QString& method,
                                         const Args&... args) {
    // TODO use async or callback
    QDBusPendingReply<ReturnType> reply(m_dbusInterface->call(method, args...));

    if (reply.isValid()) {
        // TODO
        QJsonValue result;
        if constexpr (std::same_as<ReturnType, void>) {
            result = QJsonValue::Null;
        } else if constexpr (std::same_as<ReturnType, QStringList>) {
            QJsonArray arr;
            for (auto& s : reply.value()) arr.append(s);
            result = arr;
        } else {
            result = reply.value();
        }
        emit outputJson({{"id", id}, {"result", result}});
        if (method == "switchToActivityDesktop")
            emit outputJson({{"debug", "switchToActivityDesktop!"}});
    } else {
        emit outputJson({{"id", id}, {"error", reply.error().message()}});
    }
}

void WindowManagerJsonBridge::sendSignal(const QString& signal,
                                         const QJsonArray& params) {
    emit outputJson({{"signal", signal}, {"params", params}});
}

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    WindowManagerJsonBridge bridge(&app);
    return app.exec();
}
