#/bin/bash
# Dependencies (automatic install included):
#	*buntu 14.04:
#		qt5-qmake qtdeclarative5-dev build-essential checkinstall

# running from scripts subdirectory
[ -d src ] || cd ..

function exec_qmake(){
	qmake -qt=5 || return 1
}

function exec_checkinstall(){
	sudo checkinstall \
		--pkgname="dwarf-therapist" \
		--pkgversion="$1" \
		--pkgrelease=1 \
		--nodoc \
		--pkgsource="https://github.com/splintermind/Dwarf-Therapist" \
		--maintainer="`whoami`"\
	|| return 1
}

if ! exec_qmake; then
	sudo apt-get install qt5-qmake qtdeclarative5-dev --no-install-recommends
	exec_qmake || exit 1
fi

if ! make; then
	sudo apt-get install build-essential
	make || exit 1
fi

# checking if it's a git repo or an unpacked release
[ -d .git ] && \
	version="`date '+%Y-%m-%d'`-`git log --pretty=format:'%h' -n 1`" \
|| \
	version="`head -n1 CHANGELOG.txt | grep Version | grep -o "[0-9]*\.[0-9]*"`"

[ -f description-pak ] || \
	echo "Advanced GUI to manage dwarfs in the Dwarf Fortress game." > description-pak

if ! exec_checkinstall $version; then
	sudo apt-get install checkinstall
	exec_checkinstall $version
fi