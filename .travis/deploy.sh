#!/bin/bash
set -ev

if [ "$TRAVIS_OS_NAME" == "osx" ]; then
    brew install jq
fi

#manual_url=$(wget https://api.github.com/repos/Dwarf-Therapist/Manual/releases/latest -O - | jq -r .assets[0].browser_download_url)
manual_url="https://github.com/Dwarf-Therapist/Manual/releases/download/v23.2beta/Dwarf.Therapist.Manual.zip"
wget "$manual_url" -O manual.zip && unzip manual.zip
