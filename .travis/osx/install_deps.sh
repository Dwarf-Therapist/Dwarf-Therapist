#!/bin/bash
set -ev

pip3 install aqtinstall
aqt install -O "$HOME/Qt" "$QT_VERSION" mac desktop
