#!/bin/sh
rm -rf ../dist/DwarfTherapist.dmg
#mkdir -p ../dist
hdiutil convert template.dmg -format UDSP -o ../dist/DwarfTherapist
hdiutil resize -size 80m ../dist/DwarfTherapist.sparseimage
hdiutil mount ../dist/DwarfTherapist.sparseimage

# Prepare contents
cp -R ../release/DwarfTherapist.app/Contents /Volumes/Dwarf\ Therapist/DwarfTherapist.app/Contents
cp -f ../CHANGELOG.txt /Volumes/Dwarf\ Therapist/
cp -f ../LICENSE.txt /Volumes/Dwarf\ Therapist/
cp -f ../README.txt /Volumes/Dwarf\ Therapist/
rm -f /Volumes/Dwarf\ Therapist/DwarfTherapist.app/Contents/MacOS/log/run.log
mkdir -p /Volumes/Dwarf\ Therapist/DwarfTherapist.app/Contents/Resources
cp -R ../share/memory_layouts /Volumes/Dwarf\ Therapist/DwarfTherapist.app/Contents/Resources

macdeployqt /Volumes/Dwarf\ Therapist/DwarfTherapist.app

hdiutil eject /volumes/Dwarf\ Therapist/
hdiutil convert ../dist/DwarfTherapist.sparseimage -format UDBZ -o ../dist/DwarfTherapist.dmg
rm -f ../dist/DwarfTherapist.sparseimage

