#!/bin/bash

manual_url=$(wget https://api.github.com/repos/Dwarf-Therapist/Manual/releases/latest -O - | jq -r .assets[0].browser_download_url)
wget "$manual_url" -O manual.zip && unzip manual.zip
