#!/bin/bash
set -ev

unset MACOSX_DEPLOYMENT_TARGET # pip3 doesn't like this variable, it's only needed for cmake anyway
pip3 install aqtinstall
aqt install -O "$HOME/Qt" "$QT_VERSION" mac desktop
