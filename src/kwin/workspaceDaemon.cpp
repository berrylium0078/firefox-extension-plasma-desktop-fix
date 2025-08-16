#include "kwin/workspaceDaemon.h"

#include <kwin/activities.h>
#include <kwin/virtualdesktops.h>
#include <kwin/window.h>
#include <kwin/workspace.h>
#include <qobject.h>
#include <qtmetamacros.h>
#include <quuid.h>

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusServer>
#include <QUuid>
#include <ranges>

#include "dbus-common.h"

using namespace std::ranges::views;

QStringList WindowManager::getWindowDesktops(const QString& window_uuid) {
    KWin::Window* window =
        KWin::workspace()->findWindow(QUuid::fromString(window_uuid));
    if (!window) return {};

    if (window->isOnAllDesktops()) return {};

    const auto& desktop_ids =
        window->desktops() | transform([](const KWin::VirtualDesktop* desktop) {
            return desktop->id();
        });
    return {desktop_ids.begin(), desktop_ids.end()};
}
QStringList WindowManager::getWindowActivities(const QString& window_uuid) {
    KWin::Window* window =
        KWin::workspace()->findWindow(QUuid::fromString(window_uuid));
    if (!window) return {};

    if (window->isOnAllActivities()) return {};
    return window->activities();
}

QString WindowManager::claimWindow(const QString& captionPrefix) {
    const KWin::Window* window =
        KWin::workspace()->findWindow([&](const KWin::Window* window) -> bool {
            return window->captionNormal().startsWith(captionPrefix);
        });
    if (!window) return QUuid().toString(QUuid::WithoutBraces);

    connect(window, &KWin::Window::activitiesChanged, this, [this, window]() {
        Q_EMIT this->windowActivitiesChanged(
            window->internalId().toString(QUuid::WithoutBraces),
            window->activities());
    });
    connect(window, &KWin::Window::desktopsChanged, this, [this, window]() {
        auto desktop_ids =
            window->desktops() |
            std::views::transform(
                [](KWin::VirtualDesktop* desktop) { return desktop->id(); });
        Q_EMIT this->windowDesktopsChanged(
            window->internalId().toString(QUuid::WithoutBraces),
            {desktop_ids.begin(), desktop_ids.end()});
    });
    return window->internalId().toString(QUuid::WithoutBraces);
}
QString WindowManager::getCurrentDesktop() {
    return KWin::VirtualDesktopManager::self()->currentDesktop()->id();
}
QString WindowManager::getCurrentActivity() {
    return KWin::workspace()->activities()->current();
}
void WindowManager::switchToActivityDesktop(const QString& activity_uuid,
                                            const QString& desktop_id) {
    KWin::VirtualDesktop* target =
        KWin::VirtualDesktopManager::self()->desktopForId(desktop_id);
    if (target != nullptr)
        KWin::VirtualDesktopManager::self()->setCurrent(target);
    KWin::workspace()->activities()->setCurrent(activity_uuid, target);
}
void WindowManager::setWindowActivities(const QString& window_uuid,
                                        const QStringList& activities_uuids) {
    KWin::Window* window =
        KWin::workspace()->findWindow(QUuid::fromString(window_uuid));

    if (window) {
        if (activities_uuids.isEmpty()) {
            window->setOnAllActivities(true);
        } else {
            window->setOnActivities(activities_uuids);
        }
    }
}
void WindowManager::setWindowDesktops(const QString& window_uuid,
                                      const QStringList& desktop_ids) {
    KWin::Window* window =
        KWin::workspace()->findWindow(QUuid::fromString(window_uuid));

    if (window) {
        if (desktop_ids.isEmpty()) {
            window->setOnAllDesktops(true);
        } else {
            // TODO: optimize this
            auto targetDesktops =
                desktop_ids | transform([](auto& id) {
                    return KWin::VirtualDesktopManager::self()->desktopForId(
                        id);
                }) |
                filter([](auto* ptr) { return ptr; });

            window->setDesktops({targetDesktops.begin(), targetDesktops.end()});
        }
    }
}

WindowManager::WindowManager(QObject* parent) : QDBusAbstractAdaptor(parent) {
    setAutoRelaySignals(true);

    connect(KWin::workspace()->activities(), &KWin::Activities::currentChanged,
            this, [this](const QString& new_activity_uuid) {
                Q_EMIT this->activityChanged(new_activity_uuid);
            });
    connect(KWin::VirtualDesktopManager::self(),
            &KWin::VirtualDesktopManager::currentChanged, this,
            [this](KWin::VirtualDesktop*, KWin::VirtualDesktop* newDesktop) {
                Q_EMIT this->desktopChanged(newDesktop->id());
            });
}

ServiceRegister::ServiceRegister() {
    auto server = new QDBusServer(QString::fromStdString(DBUS_ADDRESS), this);
    server->setAnonymousAuthenticationAllowed(true);
    connect(server, &QDBusServer::newConnection, [&](QDBusConnection conn) {
        auto* obj = new QObject(this);
        auto* win = new WindowManager(obj);
        if (conn.registerObject(QString::fromStdString(OBJECT_PATH), obj)) {
            qDebug() << "Registered object" << OBJECT_PATH;
        } else {
            qDebug() << "Failed to register object:" << OBJECT_PATH
                     << conn.lastError().message();
        }
    });
}
ServiceRegister::~ServiceRegister() {}
