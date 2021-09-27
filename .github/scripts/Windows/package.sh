#!/bin/bash
set -ev

ARCH=${QT_ARCH%%_*}

# Compile Qt style plugins
case $ARCH in
win32)
    VSARCH=x86
    ;;
win64)
    VSARCH=amd64
    ;;
esac
for plugin in fusiondark DarkStyle; do
    git clone https://github.com/cvuchener/$plugin
    cd $plugin
    cmd //C $GITHUB_WORKSPACE/.github/scripts/Windows/build_plugin.bat $VSARCH
    cd -
done

dest=DwarfTherapist-${GITHUB_REF##*/}-${ARCH}
cmake --install "$BINARY_DIR" --prefix "$dest"

windeployqt "$dest/DwarfTherapist.exe"

mkdir "$dest/doc"
cp "Dwarf Therapist.pdf" "$dest/doc/"

7z a "$dest.zip" "$dest/"
