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

