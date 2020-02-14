#!/bin/bash
set -ev

brew update
brew outdated cmake || brew upgrade cmake
brew install ninja
pip3 install aqtinstall
aqt install -O "$HOME/Qt" "$QT_VERSION" mac desktop
