#!/bin/bash
set -ev

hdiutil convert osx/template.dmg -format UDSP -o DwarfTherapist
hdiutil resize -size 80m DwarfTherapist.sparseimage
hdiutil mount DwarfTherapist.sparseimage

# Prepare contents
cp -R "$BINARY_DIR/DwarfTherapist.app/Contents" "/Volumes/Dwarf Therapist/DwarfTherapist.app/Contents"
cp -f "$SOURCE_DIR/CHANGELOG.txt" "/Volumes/Dwarf Therapist/"
cp -f "$SOURCE_DIR/LICENSE.txt" "/Volumes/Dwarf Therapist/"
cp -f "$SOURCE_DIR/README.rst" "/Volumes/Dwarf Therapist/"
mkdir -p "/Volumes/Dwarf Therapist/DwarfTherapist.app/Contents/Resources"
cp -R "$SOURCE_DIR/share/memory_layouts" "/Volumes/Dwarf Therapist/DwarfTherapist.app/Contents/Resources"

macdeployqt "/Volumes/Dwarf Therapist/DwarfTherapist.app"

# Sometimes the disk is not ready after macdeployqt, wait and retry if it fails
retry=10
while ! hdiutil eject "/Volumes/Dwarf Therapist/" && [ "$retry" -gt 0 ]; do
    retry=$((retry-1))
    sleep 1
done
hdiutil convert DwarfTherapist.sparseimage -format UDBZ -o "DwarfTherapist-${GITHUB_REF##*/}-osx64-qt${QT_VERSION-latest}.dmg"

