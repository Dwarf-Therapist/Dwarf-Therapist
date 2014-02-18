[This is a heavily modified version of the original Dwarf Therapist](http://code.google.com/p/dwarftherapist/).

Many new features (attributes, roles, optimization plans, health views, inventory, etc.) have been added, and many bugs have been resolved.

If you'd like to help support this project [![Donate](http://dl.dropbox.com/u/185441/happy-thoughts.png)](https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=GM5Z6DYJEVW56&lc=CA&item_name=Donation&currency_code=CAD&bn=PP%2dDonationsBF%3abtn_donate_SM%2egif%3aNonHosted)

### [Change Log](https://github.com/splintermind/Dwarf-Therapist/wiki/Change-Log "Full Change Log")

### Windows
* [Download Latest Version for Windows DF 34.11](http://dffd.wimbli.com/file.php?id=7184 "DFFD")

### OSX
* [Download Latest Version for OSX DF 34.11](http://dffd.wimbli.com/file.php?id=8418 "DFFD")

### Linux
There currently isn't a packaged version for linux. Here are the instructions to build from source:

#### Required Packages
* qtbase5-dev, qtbase5-dev-tools, qtscript5-dev
* qt5-qmake
* libqt5script5, libqt5scripttools5
* libqxt-core0, libqxt-gui0

#### Building
````
git clone http://github.com/splintermind/Dwarf-Therapist dwarftherapist
cd dwarftherapist
qmake
sudo make install
````

**If the build is unsuccessful, be sure to remove any *.o or *.moc files in the bin/release folder before rebuilding.**

Additionally, you'll most likely need to run Dwarf Therapist with sudo. This is due to ptrace protection that exists on some flavours of linux.

Alternatively you can create a script that will run Dwarf Therapist as a child of Dwarf Fortress:
''''
#!/bin/bash
set -e
cd df_linux 
./df &
cd ../dwarftherapist 
exec ./bin/release/DwarfTherapist
''''