find_package(Qt6 CONFIG REQUIRED COMPONENTS
    Core
    DBus
)

set(EXECUTABLE_NAME "firefox-extension-plasma-desktop-fix")

add_executable(${EXECUTABLE_NAME}
    bridge.cpp
    bridge.h
)
target_link_libraries(${EXECUTABLE_NAME} PRIVATE
    Qt6::DBus
)

install(TARGETS ${EXECUTABLE_NAME} DESTINATION /usr/bin)
install(FILES io.github.berrylium0078.plasma_desktop_fix.json DESTINATION /usr/lib/mozilla/native-messaging-hosts)
