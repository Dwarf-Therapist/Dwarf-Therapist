#!/bin/bash

set -e

source="${BASH_SOURCE%\\*}"
source="${source%/*}"
: ${source:=$1}

if [[ -z $source ]]; then
    printf "Could not locate DT source directory. Pass DT clone as first argument and try again.\n" >&2
    x=1
fi

if [[ -z $QTDIR ]]; then
    printf "QTDIR is empty or unset. Set QTDIR to your Qt toolchain directory containing \"bin\" and try again.\n" >&2
    x=1
fi

if [[ ! -x $QTDIR/bin/Qt5Core.dll ]]; then
    printf "Warning: QTDIR seems invalid, could not find Qt5Core.dll. Proceeding anyways.\n" >&2
fi

if [[ $x == 1 ]]; then exit 1; fi

depends="$source/depends.exe"
found=

printf "Checking for DT executables...\n"

for d in "" debug release; do
    dt="$d"/DwarfTherapist.exe
    if [[ -f $dt ]]; then
        found=1
        printf "Found %s.\n" "$dt"
        if [[ ! -x $depends ]]; then
            printf "Downloading Dependency Walker... "
            pushd "$source"
            if [[ $PROCESSOR_ARCHITECTURE == AMD64 ]] || [[ $PROCESSOR_ARCHITEW6432 == AMD64 ]]; then b="x64";
            elif [[ $PROCESSOR_ARCHITECTURE == IA64 ]] || [[ $PROCESSOR_ARCHITEW6432 == IA64 ]]; then b="ia64";
            else b="x86"; fi
            f=depends22_"$b".zip
            curl -L -o "$f" http://dependencywalker.com/"$f"
            unzip "$f" depends.exe
            rm "$f"
            popd
            printf "done.\n"
        fi

        printf "Tracing dependencies... "
        "$depends" -c -oc:"$d"/depends.csv "$dt" || [[ $? == 10 ]] || exit $?
        printf "done.\n"
        while read dep; do
            if [[ $dep == ,* ]]; then
                rdep=${dep#,\"}
                deps+="${rdep%%\"*} "
            fi
        done < "$d"/depends.csv
        rm "$d"/depends.csv

        printf "Copying dependencies... "
        robocopy -sl -np -njh -njs "$QTDIR\\bin" "$d"/ $deps
        printf "done.\n"
    fi
done

if [[ -z $found ]]; then
    printf "Could not find any DT executables; build DT then run this from either the root of the build or the location of DwarfTherapist.exe.\n" >&2
fi
