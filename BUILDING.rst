========================
BUILDING DWARF THERAPIST
========================

Windows
=======
Download the Qt Creator IDE and open the dwarftherapist.pro file.

Linux
=====

Dependencies
------------

Debian-based
************

::

    sudo apt-get install qt4-qmake libqt4-dev # Qt 4
    sudo apt-get install qt5-qmake qtbase5-dev qtbase5-dev-tools qtdeclarative5-dev # Qt 5

Gentoo-based
************

::

    sudo emerge -av qtcore qtdeclarative # Qt 4 (Currently only Qt 4 is supported)
    
* As Gentoo only supports Qt 4, qmake doesn't have a -qt paramater. Otherwise, follow the instructions as below

Building
--------

::

    qmake -qt=4 # Qt 4 on Debian-based
    qmake -qt=5 # Qt 5 on Debian-based
    make -j$(nproc) # Run as many jobs as processing units

For instructions on where to find qmake and how to invoke it on other distros, see your distribution's documentation.

This will take some time.  Get a cup of coffee.  Once your build is complete, run::

    sudo make install

If you want, you can now remove the folder you cloned from github.

Troubleshooting
---------------

If your build fails for whatever reason, the easiest way to start over again is to go to wherever you cloned the project and run::

    rm -rf Dwarf-Therapist

and you can restart with cloning the project.

If you are having problems with building, ask on the Splintermind thread in the Bay12Forums: http://www.bay12forums.com/smf/index.php?topic=122968.

Uninstallation
--------------

Automatic
*********

If you still have the folder you cloned from github, run::

    sudo make uninstall

in that directory.

Manual
******

Using sudo rm -rf, remove::

* /usr/bin/{dwarftherapist,DwarfTherapist}
* /usr/share/dwarftherapist/
* /usr/share/applications/dwarftherapist.desktop
* /usr/share/doc/dwarftherapist/
* /usr/share/pixmaps/dwarftherapist.*
