set(KF6_MIN_VERSION "6.0.0")

find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
set(TARGET_NAME "io.github.berrylium0078.workspace_daemon")

include(FeatureSummary)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

find_package(Qt6 CONFIG REQUIRED COMPONENTS
    Core
    Widgets
    DBus
)
find_package(KWin REQUIRED COMPONENTS
    kwin
)

kcoreaddons_add_plugin(${TARGET_NAME} INSTALL_NAMESPACE "kwin/plugins")

target_sources(${TARGET_NAME} PRIVATE
    plugin.cpp
    workspaceDaemon.h
    workspaceDaemon.cpp
)
target_include_directories(${TARGET_NAME} PRIVATE
    /usr/include/PlasmaActivities/
)

target_link_libraries(${TARGET_NAME} PRIVATE
    KWin::kwin
    Qt6::DBus
    PlasmaActivities
)
