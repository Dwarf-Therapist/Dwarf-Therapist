===============
Dwarf Therapist
===============

.. image:: https://travis-ci.org/splintermind/Dwarf-Therapist.svg?branch=DF2016
   :target: https://travis-ci.org/splintermind/Dwarf-Therapist
.. image:: https://scan.coverity.com/projects/3038/badge.svg
   :target: https://scan.coverity.com/projects/3038

This is a heavily modified version of the original `Dwarf Therapist`_.

Many new features (attributes, roles, optimization plans, health views, inventory, etc.) have been added, and many bugs have been resolved.

If you'd like to help support this project:

.. image:: http://dl.dropbox.com/u/185441/happy-thoughts.png
   :alt: Buy me a Beer! ... and I'll have happy thoughts ...
   :target: https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=GM5Z6DYJEVW56&lc=CA&item_name=Donation&currency_code=CAD&bn=PP%2dDonationsBF%3abtn_donate_SM%2egif%3aNonHosted

The changelog is `available at GitHub`_.

Latest Release
========
The `latest release`_ is always published in the project's `releases`_ page. These releases normally contain packages for Windows and OSX.

Alternate Download (DFFD)
-------------------------
If the latest published releases do not contain a package, they may be found on DFFD.

`Windows 32-bit <http://dffd.bay12games.com/file.php?id=11420>`_

`OSX <http://dffd.bay12games.com/file.php?id=11579>`_

Linux
=====
There is currently no official package for Linux.
You can find the instructions to build from source `in BUILDING.rst`_.

Running the program
-------------------
On default installations of most Debian-based distributions, you'll most likely need to run Dwarf Therapist with sudo.
This is due to the `Yama ptrace LSM`_ enabled by default on such distros.
You can explicitly give Dwarf Therapist permission to ptrace on Debian-based distributions with::

    sudo apt-get install libcap2-bin && sudo setcap cap_sys_ptrace=eip ./bin/release/DwarfTherapist

Or, you can create a script that will run Dwarf Therapist as a child of Dwarf Fortress::

    #!/bin/bash
    set -e
    cd df_linux
    ./df &
    cd ../dwarftherapist
    exec ./bin/release/DwarfTherapist

Alternatively, you can use the ``dist/dwarftherapist`` wrapper to automatically grant DT permissions to attach to a running DF process.

Support
=======
Primary support is available at the `Dwarf Therapist thread`_ at the Bay 12 Forums; if you are sure you have found a bug, file an issue at the `GitHub issue tracker`_.
Provide as much information as possible to help reproduce the issue.
Most runtime bugs will require you to send a save, preferably uploaded at `DFFD`_.
You may also be requested to provide a log, which is saved in ``log/log.txt`` on Windows and OSX and output to stderr on Linux systems (use ``2> log.txt`` to collect).

You can also join us in `#dwarftherapist on Freenode`_.

.. _Dwarf Therapist: http://code.google.com/p/dwarftherapist/
.. _available at GitHub: https://github.com/splintermind/Dwarf-Therapist/wiki/Change-Log
.. _in BUILDING.rst: https://github.com/splintermind/Dwarf-Therapist/blob/DF2014/BUILDING.rst
.. _Yama ptrace LSM: https://www.kernel.org/doc/Documentation/security/Yama.txt
.. _Dwarf Therapist thread: http://www.bay12forums.com/smf/index.php?topic=122968
.. _GitHub issue tracker: https://github.com/splintermind/Dwarf-Therapist/issues
.. _DFFD: http://dffd.wimbli.com/category.php?id=20
.. _#dwarftherapist on Freenode: http://webchat.freenode.net/?channels=%23dwarftherapist
.. _releases: https://github.com/splintermind/Dwarf-Therapist/releases
.. _latest release: https://github.com/splintermind/Dwarf-Therapist/releases/latest
