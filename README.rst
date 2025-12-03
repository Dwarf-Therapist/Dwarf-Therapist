===============
Dwarf Therapist
===============

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
Dwarf-Therapist requires a C++ compiler (with C++14 support), cmake (3.1.0 or newer), and Qt5 (5.9 or newer, with Widgets and QML and Network modules).

Detailed building instructions can be found in `BUILDING.md`_.

Linux
=====
In addition to the AppImage provided on the `releases`_ page, packages for specific distributions can be found at:

- `COPR repository`_ for Fedora users.
- `AUR package`_ for Arch users.

Some Linux distributions restrict the ability to trace other processes. If Dwarf Therapist fails to connect to Dwarf Fortress, check `ptrace_scope help`_.

Memory layouts
==============
Dwarf Therapist uses per-version memory layout files to understand Dwarf Fortress internals. Official releases ship with layouts for the Dwarf Fortress versions they support, and most users should simply install the latest Dwarf Therapist release whenever Dwarf Fortress is updated.

Updating memory layouts for new Dwarf Fortress versions
-------------------------------------------------------
For advanced users who want to try Dwarf Therapist with a newer Dwarf Fortress version before an updated Dwarf Therapist release is available, a memory layout can be generated using DFHack.

Basic workflow (advanced users only):

1. Install DFHack that matches your Dwarf Fortress version and start the game with DFHack enabled.
2. In the DFHack console, run::

      devel/export-dt-ini

   This will create a file named ``therapist.ini`` in the Dwarf Fortress root directory.
3. In Dwarf Therapist, use **File → Open data directory** to open the data directory used by your installation on the current OS.
4. Move the generated ``therapist.ini`` file into the appropriate ``memory_layouts/<platform>`` subdirectory inside that data directory (for example, ``memory_layouts/windows`` or ``memory_layouts/linux``).
5. Restart Dwarf Therapist.

Caveats and limitations:

- The DFHack ``devel/export-dt-ini`` script and Dwarf Therapist are not always in sync. Changes to the script in newer DFHack versions can produce layouts that are incomplete or incompatible with the current Dwarf Therapist release.
- The generated layout may be incorrect and can cause wrong data to be displayed or other misbehaviour. This procedure is intended only for users who understand the risks and are comfortable diagnosing problems.
- If you encounter issues after adding a generated layout, remove the corresponding ``therapist.ini`` file from the ``memory_layouts`` directory and restart Dwarf Therapist.
- The preferred and supported way to get correct memory layouts is still to download and use the latest official Dwarf Therapist release for your Dwarf Fortress version.

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
.. _ptrace_scope help: https://github.com/Dwarf-Therapist/Dwarf-Therapist/blob/master/dist/ptrace_scope/README.md
.. _Yama ptrace LSM: https://www.kernel.org/doc/Documentation/security/Yama.txt
.. _Dwarf Therapist thread: http://www.bay12forums.com/smf/index.php?topic=168411
.. _GitHub issue tracker: https://github.com/Dwarf-Therapist/Dwarf-Therapist/issues
.. _DFFD: http://dffd.wimbli.com/category.php?id=20
.. _#dwarftherapist on Freenode: http://webchat.freenode.net/?channels=%23dwarftherapist
.. _releases: https://github.com/Dwarf-Therapist/Dwarf-Therapist/releases
.. _latest release: https://github.com/Dwarf-Therapist/Dwarf-Therapist/releases/latest
.. _COPR repository: https://copr.fedorainfracloud.org/coprs/cvuchener/Dwarf-Therapist/
.. _AUR package: https://aur.archlinux.org/packages/dwarftherapist/
