===============
Dwarf Therapist
===============

.. image:: https://travis-ci.org/Dwarf-Therapist/Dwarf-Therapist.svg?branch=master
   :target: https://travis-ci.org/Dwarf-Therapist/Dwarf-Therapist
.. image:: https://scan.coverity.com/projects/13797/badge.svg
   :target: https://scan.coverity.com/projects/dwarf-therapist-dwarf-therapist

This is the maintained version of `Splintermind's Dwarf Therapist`_ (itself a heavily modified version of the `original Dwarf Therapist`_).

The Dwarf Therapist guide source and releases have `their own repository <https://github.com/Dwarf-Therapist/Manual>`_.

Latest Release
========
The `latest release`_ is always published in the project's `releases`_ page. These releases normally contain packages for Windows, OSX, and Linux.

Alternate Download (DFFD)
-------------------------
If the latest published releases do not contain a package, they may be found on DFFD.

`Windows 32-bit <http://dffd.bay12games.com/file.php?id=13094>`_

`Windows 64-bit <http://dffd.bay12games.com/file.php?id=13095>`_

The manual for Dwarf Therapist can be downloaded separately.

`Stand-alone Manual <http://dffd.bay12games.com/file.php?id=7889>`_

Building
========
Dwarf-Therapist requires a C++ compiler (with C++14 support), cmake (3.1.0 or newer), and Qt5 (with Widgets and QML modules).

Detailed building instructions can be found in `BUILDING.md`_

Linux
=====
In addition to the AppImage provided on the `releases`_ page, packages for specific distributions can be found at:

- `COPR repository`_ for Fedora users.

Running the program
-------------------
On default installations of most Debian-based distributions, you'll most likely need to run Dwarf Therapist with sudo.
This is due to the `Yama ptrace LSM`_ enabled by default on such distros.
You can explicitly give Dwarf Therapist permission to ptrace on Debian-based distributions with::

    sudo apt-get install libcap2-bin && sudo setcap cap_sys_ptrace=eip ./bin/release/DwarfTherapist

Or, you can create a script that will run Dwarf Fortress as a child of Dwarf Therapist::

    #!/bin/bash
    set -e
    cd df_linux
    ./df &
    cd ../dwarftherapist
    exec ./bin/release/DwarfTherapist

Alternatively, you can use the ``dist/ptrace_cap_wrapper/dwarftherapist`` wrapper to automatically grant DT permissions to attach to a running DF process (edit the value of ``_DT_BINARY`` if Dwarf Therapist is not in the path).

Support
=======
Primary support is available at the `Dwarf Therapist thread`_ at the Bay 12 Forums; if you are sure you have found a bug, file an issue at the `GitHub issue tracker`_.
Provide as much information as possible to help reproduce the issue.
Most runtime bugs will require you to send a save, preferably uploaded at `DFFD`_.
You may also be requested to provide a log, which is saved in ``log/log.txt`` on Windows and OSX and output to stderr on Linux systems (use ``2> log.txt`` to collect).

You can also join us in `#dwarftherapist on Freenode`_.

.. _Splintermind's Dwarf Therapist: https://github.com/splintermind/Dwarf-Therapist/
.. _original Dwarf Therapist: http://code.google.com/p/dwarftherapist/
.. _BUILDING.md: https://github.com/Dwarf-Therapist/Dwarf-Therapist/blob/master/BUILDING.md
.. _Yama ptrace LSM: https://www.kernel.org/doc/Documentation/security/Yama.txt
.. _Dwarf Therapist thread: http://www.bay12forums.com/smf/index.php?topic=168411
.. _GitHub issue tracker: https://github.com/Dwarf-Therapist/Dwarf-Therapist/issues
.. _DFFD: http://dffd.wimbli.com/category.php?id=20
.. _#dwarftherapist on Freenode: http://webchat.freenode.net/?channels=%23dwarftherapist
.. _releases: https://github.com/Dwarf-Therapist/Dwarf-Therapist/releases
.. _latest release: https://github.com/Dwarf-Therapist/Dwarf-Therapist/releases/latest
.. _COPR repository: https://copr.fedorainfracloud.org/coprs/cvuchener/Dwarf-Therapist/
