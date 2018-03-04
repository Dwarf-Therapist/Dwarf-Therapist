#!/bin/bash

ninja install

# Add PDF manual
install -m 0644 -D -t "$APPDIR/usr/share/doc/dwarftherapist/" "Dwarf Therapist.pdf"

# Download and extract linuxdeployqt (because of missing FUSE)
wget "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"
chmod +x linuxdeployqt*.AppImage
./linuxdeployqt*.AppImage --appimage-extract
mv squashfs-root linuxdeployqt

linuxdeployqt/AppRun "$APPDIR/usr/share/applications/dwarftherapist.desktop" -bundle-non-qt-libs -qmake=$QT_PREFIX/bin/qmake

# Download patched AppRun for optional loading of libstdc++
wget "https://github.com/darealshinji/AppImageKit-checkrt/releases/download/continuous/AppRun-patched-x86_64"
chmod +x "AppRun-patched-x86_64"
rm "$APPDIR/AppRun"
cp "AppRun-patched-x86_64" "$APPDIR/AppRun"
mkdir -p "$APPDIR/usr/optional/"{libstdc++,libgcc_s}
wget "https://github.com/darealshinji/AppImageKit-checkrt/releases/download/continuous/exec-x86_64.so"
cp "exec-x86_64.so" "$APPDIR/usr/optional/exec.so"
cp "/usr/lib/x86_64-linux-gnu/libstdc++.so.6" "$APPDIR/usr/optional/libstdc++/"
cp "/lib/x86_64-linux-gnu/libgcc_s.so.1" "$APPDIR/usr/optional/libgcc_s/"

# Download and extract appimagetool
wget "https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage"
chmod +x "appimagetool-x86_64.AppImage"
./appimagetool-x86_64.AppImage --appimage-extract
mv squashfs-root appimagetool

appimagetool/AppRun "$APPDIR"

# Rename DT AppImage using current tag
filename=Dwarf_Therapist-x86_64.AppImage
mv -v "$filename" "${filename/Dwarf_Therapist/DwarfTherapist-${TRAVIS_TAG}-${TRAVIS_OS_NAME}}"

