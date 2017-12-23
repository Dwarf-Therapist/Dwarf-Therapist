#!/bin/bash

ninja install

# Add PDF manual
install -m 0644 -D -t "$DT_PREFIX/share/doc/dwarftherapist/" "Dwarf Therapist.pdf"

# Download linuxdeployqt
wget "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"
linuxdeployqt="./linuxdeployqt-continuous-x86_64.AppImage"
chmod +x "$linuxdeployqt"

# does not work because of missing FUSE
#"$linuxdeployqt" "$PREFIX/usr/share/applications/dwarftherapist.desktop" -appimage

# extract AppImage content instead
"$linuxdeployqt" --appimage-extract
EXTRACTED_PREFIX=$PWD/squashfs-root/usr
export PATH=$EXTRACTED_PREFIX/bin:$PATH
export LD_LIBRARY_PATH=$EXTRACTED_PREFIX/lib:$LD_LIBRARY_PATH

linuxdeployqt "$DT_PREFIX/share/applications/dwarftherapist.desktop" -appimage -qmake=$QT_PREFIX/bin/qmake

# Rename DT AppImage using current tag
filename=Dwarf_Therapist-x86_64.AppImage
mv -v "$filename" "${filename/Dwarf_Therapist/DwarfTherapist-${TRAVIS_TAG}-${TRAVIS_OS_NAME}}"

