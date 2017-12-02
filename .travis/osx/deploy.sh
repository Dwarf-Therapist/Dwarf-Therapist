#!/bin/bash

hdiutil convert osx/template.dmg -format UDSP -o DwarfTherapist
hdiutil resize -size 80m DwarfTherapist.sparseimage
hdiutil mount DwarfTherapist.sparseimage

# Prepare contents
cp -R DwarfTherapist.app/Contents /Volumes/Dwarf\ Therapist/DwarfTherapist.app/Contents
cp -f CHANGELOG.txt /Volumes/Dwarf\ Therapist/
cp -f LICENSE.txt /Volumes/Dwarf\ Therapist/
cp -f README.rst /Volumes/Dwarf\ Therapist/

$QT_PREFIX/bin/macdeployqt /Volumes/Dwarf\ Therapist/DwarfTherapist.app

hdiutil eject /volumes/Dwarf\ Therapist/
hdiutil convert DwarfTherapist.sparseimage -format UDBZ -o DwarfTherapist-${TRAVIS_TAG}-${TRAVIS_OS_NAME}64.dmg

