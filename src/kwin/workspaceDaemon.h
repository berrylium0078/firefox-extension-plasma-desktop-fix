#pragma once

#include <kwin/plugin.h>
#include <qobject.h>

#include <KPluginFactory>
#include <QDBusAbstractAdaptor>
#include <QDBusConnection>
#include <QDBusError>
#include <QUuid>

#include "dbus-common.h"

class WindowManager : public QDBusAbstractAdaptor {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", INTERFACE_NAME)

 public:
    explicit WindowManager(QObject* obj);

 public Q_SLOTS:
    // Given a caption prefix to match (assuming uniqueness, e.g. QString),
    // Returns the internal QString of the window.
    QString claimWindow(const QString& captionPrefix);

 Q_SIGNALS:
    // only claimed windows will emit the two signals
    void windowDesktopsChanged(const QString& window_uuid,
                               const QStringList& desktop_ids);
    void windowActivitiesChanged(const QString& window_uuid,
                                 const QStringList& activities_uuids);

    void activityChanged(const QString& activity_uuid);
    void desktopChanged(const QString& desktop_id);

 public Q_SLOTS:
    QString getCurrentDesktop();
    QString getCurrentActivity();
    QStringList getWindowDesktops(const QString& window_uuid);
    QStringList getWindowActivities(const QString& window_uuid);
    void switchToActivityDesktop(const QString& activity_uuid,
                                 const QString& desktop_id);
    void setWindowActivities(const QString& window_uuid,
                             const QStringList& activities_uuids);
    void setWindowDesktops(const QString& window_uuid,
                           const QStringList& desktop_ids);
};

class ServiceRegister : public KWin::Plugin {
    Q_OBJECT
 public:
    ServiceRegister();
    ~ServiceRegister();
};
