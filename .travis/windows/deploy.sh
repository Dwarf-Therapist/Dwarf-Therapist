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
    cmd //C $TRAVIS_BUILD_DIR/.travis/windows/build_plugin.bat $VSARCH $QT_PREFIX
    cd -
done

dest=DwarfTherapist-${TRAVIS_TAG}-${ARCH}
cmake --install "$BINARY_DIR" --prefix "$dest"

"$QT_PREFIX/bin/windeployqt.exe" "$dest/DwarfTherapist.exe"

mkdir "$dest/doc"
cp "Dwarf Therapist.pdf" "$dest/doc/"

if [ "$QT_ARCH" = win32_msvc2015 ]; then
    wget "https://indy.fulgan.com/SSL/openssl-1.0.2u-i386-win32.zip" -O openssl.zip
    7z e openssl.zip libeay32.dll ssleay32.dll -o"$dest"
fi

7z a "$dest.zip" "$dest/"
