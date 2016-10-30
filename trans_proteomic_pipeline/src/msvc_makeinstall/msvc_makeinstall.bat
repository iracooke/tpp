copy %1..\README %1..\README.txt
"c:/Program Files/NSIS/makensis.exe" -DPRODUCT_BUILD_DIR=%1%2 %1../installer_win32/TPP_installer.nsi
del %1..\README.txt
