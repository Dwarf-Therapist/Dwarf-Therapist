#!/bin/bash
set -ev

export APPDIR=DwarfTherapist.AppDir
cmake --install "$BINARY_DIR" --prefix "$APPDIR/usr"

# Add PDF manual
install -m 0644 -D -t "$APPDIR/usr/share/doc/dwarftherapist/" "Dwarf Therapist.pdf"

# Download linuxdeploy + plugins
wget "https://github.com/linuxdeploy/linuxdeploy/releases/latest/download/linuxdeploy-x86_64.AppImage"
wget "https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/latest/download/linuxdeploy-plugin-qt-x86_64.AppImage"
wget "https://github.com/linuxdeploy/linuxdeploy-plugin-appimage/releases/latest/download/linuxdeploy-plugin-appimage-x86_64.AppImage"
cp "$SOURCE_DIR/.github/scripts/Linux/linuxdeploy-plugin-xdgdatadirs.sh" .
chmod +x linuxdeploy-*.AppImage
export LINUXDEPLOY_OUTPUT_APP_NAME=DwarfTherapist-${GITHUB_REF##*/}-linux
./linuxdeploy-x86_64.AppImage \
    --appdir="$APPDIR" \
    --desktop-file="$APPDIR/usr/share/applications/dwarftherapist.desktop" \
    --plugin=qt \
    --plugin=xdgdatadirs \
    --output=appimage
