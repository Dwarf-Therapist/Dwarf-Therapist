#!/bin/bash
set -ev

export APPDIR=DwarfTherapist.AppDir
cmake --install "$BINARY_DIR" --prefix "$APPDIR/usr"

# Add PDF manual
install -m 0644 -D -t "$APPDIR/usr/share/doc/dwarftherapist/" "Dwarf Therapist.pdf"

# Compile Qt style plugins
for plugin in fusiondark DarkStyle; do
    git clone https://github.com/cvuchener/$plugin
    cd $plugin
    qmake
    make
    sudo make install
    cd -
done

# Install dependencies for deployment
sudo apt update
sudo apt install -y \
    libfontconfig1 \
    libxcb-icccm4 \
    libxcb-image0 \
    libxcb-keysyms1 \
    libxcb-render-util0 \
    libxcb-shape0 \
    libxcb-xinerama0 \
    libxcb-xkb1 \
    libxkbcommon-x11-0 \
    libdbus-1-3 \
    libegl1

# Download and extract linuxdeployqt
wget "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"
chmod +x linuxdeployqt*.AppImage

./linuxdeployqt*.AppImage "$APPDIR/usr/share/applications/dwarftherapist.desktop" -extra-plugins=styles/libfusiondark.so,styles/libDarkStyle.so -bundle-non-qt-libs

rm "$APPDIR/AppRun"
wget "https://github.com/AppImage/AppImageKit/releases/download/continuous/AppRun-x86_64" -O "$APPDIR/AppRun"
chmod +x "$APPDIR/AppRun"

# Download and extract appimagetool
wget "https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage"
chmod +x "appimagetool-x86_64.AppImage"
./appimagetool-x86_64.AppImage "$APPDIR"

# Rename DT AppImage using current tag
filename=Dwarf_Therapist-x86_64.AppImage
mv -v "$filename" "${filename/Dwarf_Therapist/DwarfTherapist-${GITHUB_REF##*/}-linux}"

