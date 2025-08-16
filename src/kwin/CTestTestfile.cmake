# CMake generated Testfile for 
# Source directory: /home/netro/firefox-extension-plasma-desktop-fix/src/kwin
# Build directory: /home/netro/firefox-extension-plasma-desktop-fix/src/kwin
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test([=[appstreamtest]=] "/usr/bin/cmake" "-DAPPSTREAMCLI=/usr/bin/appstreamcli" "-DINSTALL_FILES=/home/netro/firefox-extension-plasma-desktop-fix/install_manifest.txt" "-P" "/usr/share/ECM/kde-modules/appstreamtest.cmake")
set_tests_properties([=[appstreamtest]=] PROPERTIES  _BACKTRACE_TRIPLES "/usr/share/ECM/kde-modules/KDECMakeSettings.cmake;168;add_test;/usr/share/ECM/kde-modules/KDECMakeSettings.cmake;187;appstreamtest;/usr/share/ECM/kde-modules/KDECMakeSettings.cmake;0;;/home/netro/firefox-extension-plasma-desktop-fix/src/kwin/CMakeLists.txt;9;include;/home/netro/firefox-extension-plasma-desktop-fix/src/kwin/CMakeLists.txt;0;")
