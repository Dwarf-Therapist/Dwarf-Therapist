#! /bin/bash

# exit whenever a command called in this script fails
set -e

appdir=""

show_usage() {
    echo "Usage: bash $0 --appdir <AppDir>"
}

while [ "$1" != "" ]; do
    case "$1" in
        --plugin-api-version)
            echo "0"
            exit 0
            ;;
        --plugin-type)
            echo "input"
            exit 0
            ;;
        --appdir)
            appdir="$2"
            shift
            shift
            ;;
        *)
            echo "Invalid argument: $1"
            echo
            show_usage
            exit 2
    esac
done

if [[ "$appdir" == "" ]]; then
    show_usage
    exit 2
fi

hook_file="$appdir/apprun-hooks/xdg-data-dirs.sh"
echo "Writing hook file: $hook_file"
mkdir -p "$appdir"/apprun-hooks
cat > "$hook_file" <<\EOF
if [ -z "$APPDIR" ]; then
    echo "\$APPDIR is not set"
else
    APPDIR_DATA="$APPDIR/usr/share"
    case :"$XDG_DATA_DIRS": in
        *:"$APPDIR_DATA":*)
            ;;
        *) 
            export XDG_DATA_DIRS="$APPDIR_DATA:${XDG_DATA_DIRS-/usr/local/share:/usr/share}"
            ;;
    esac
fi
EOF
