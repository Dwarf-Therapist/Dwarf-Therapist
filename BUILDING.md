Building Dwarf Therapist
========================

Building Dwarf Therapist requires a C++14 compiler, the minimum version for common compilers are:

 - GCC 5
 - Microsoft Visual C++ 2015
 - Clang 3.4
 - Apple LLVM 6.1.0 (Xcode 6.3)

For building the manual, see the README in the [Manual repository](https://github.com/Dwarf-Therapist/Manual).

Windows
-------

### Visual Studio ###

1.) Download Microsoft Visual Studio (2022, community edition is free - VS2019 can be used as well).

https://visualstudio.microsoft.com/downloads/#

Make sure "Development Tools for .NET" and "MSBuild" are included in the installtion options.

In the installer tab "Individual components", select these:

    - C++ Cmake Tools for Windows
    - MSVC v141 - VS2017 C++ x64/x86 build Tools

2.) Download Qt Creator & Libraries
Take version 5.9 or minor version above, for instance at time of writing it's 5.15.2.

You can either :
* use the [official installer](https://doc.qt.io/qt-6/get-and-install-qt.html).
* or use direct download links from the archive at https://download.qt.io/new_archive/qt, for instance [Qt 5.14.2 here](https://download.qt.io/new_archive/qt/5.14/5.14.2/qt-opensource-windows-x86-5.14.2.exe)
note that in any case you'll need to create a (free) account to use the installer.

Start the installer and select this option:

    - MSVC 2017 64-bit
    
Note the path where you are installing this, because this will be used later. The default path is "C:\Qt\Qt5.15.2". I did install it to "E:\Qt\Qt5.15.2". So if you see this path prefix below you should change this according to your install path.
    
3.) Download git for windows

https://gitforwindows.org
    
Install and keep the default options unless you know what to do.
    
Open a git console and navigate to a directory where you would like to download the sources of DT. (e.g. "E:\dev" ). In the git console you need to use unix path style. The path "E:\dev" will be "/e/dev" in the git console window.

4.) Check out the sources for Dwarf Therapist

Type the cmd below in the git console:
    
    git clone https://github.com/Dwarf-Therapist/Dwarf-Therapist.git
    
This will create a new directory named "Dwarf-Therapist" in the current directory. In our example the path of the sources will now be "E:\dev\Dwarf-Therapist". This path will be used for the commands in the next sections. Adapt the path accordingly, if you are using a different path.
    
5.) Compile

Open "x64 Native Tools Command Prompt for VS 2022" from start menu.
    
Navigate to the DT source directory and type the commands below:
    
    mkdir build
    cd build
    cmake -DCMAKE_PREFIX_PATH=E:/Qt/Qt5.15.2/5.15.2/msvc2017_64 -G "Visual Studio 15 2017 Win64" ..\
    MSBuild.exe DwarfTherapist.sln /p:Configuration=Release 

Troubleshooting:

Cmake is being called with a CMAKE_PREFIX_PATH using forward slashes "/" because this is the internal path notation for cmake. Cmake can deal with backward slashes, but only when it is running in command mode. When running in server mode it ONLY works with forward slashes. If you are using an IDE like "Visual Studio Code" cmake will be set to run in server mode and backward slashes will cause errors like "invalid escape \Q".


6.) Create a directory to collect all files for running Dwarf Therapist

After the compilation has been completed successfully, you should find a DwarfTherapist.exe in the "Release" folder which has been created in the build folder.

Now type these commands, assuming you are still in the build folder.

    mkdir ..\run
    copy Release\DwarfTherapist.exe ..\run
    E:\Qt\Qt5.15.2\5.15.2\msvc2017_64\bin\windeployqt.exe ..\run
    mkdir ..\run\data
    xcopy /s ..\share\* ..\run\data

The folder "run" should now contain all files needed to run DwarfTherapist.

Linux
-----

### Installing dependencies

Debian, Ubuntu, ...:

    sudo apt-get install cmake g++ qtbase5-dev qtdeclarative5-dev

Fedora:

    sudo dnf install cmake gcc-c++ qt5-qtbase-devel qt5-qtdeclarative-devel

### Building and installing

Download a source tarball or clone the repository using git:

    git clone https://github.com/Dwarf-Therapist/Dwarf-Therapist.git
    cd Dwarf-Therapist

Create a building directory and switch to it:

    mkdir build
    cd build

Run cmake:

    cmake .. <options>

Options are optional. `-DCMAKE_INSTALL_PREFIX=<path>` can be used to choose
where to install Dwarf Therapist.  For example:
`-DCMAKE_INSTALL_PREFIX=/usr/local` (default value) for a system-wide install
or `-DCMAKE_INSTALL_PREFIX=~/.local` to install for the current user only.

When cmake is configured, run `make` to build and then `make install` or
`sudo make install` to install Dwarf Therapist at the chosen location.

### Ptrace permissions

Some Linux distributions restrict the ability to trace other processes. Check your ptrace permission settings, run `sysctl kernel.yama.ptrace_scope`. If the value is 0, Dwarf Therapist should be able to connect to Dwarf Fortress. If the value is higher, read [ptrace_scope help](dist/ptrace_scope/README.md).

macOS
-----

### Installing dependencies

    brew install cmake
    brew install qt

### Building

Create a building directory and switch to it:

    mkdir build
    cd build

Run cmake passing Qt location:

    cmake .. -DCMAKE_PREFIX_PATH=/usr/local/opt/qt

Build using make:

    make

### Installing

    TODO

Cygwin / Mingw
--------------

There is a short guide for building a version for Cygwin and 
cross compiling using mingw in BUILDING.experimental.

CMake options
-------------

Add `-D<option name>=<value>` to cmake command line to use those.

### `BUILD_PORTABLE` (default: `OFF`)

If `ON`, build an executable that use `--portable` mode by default. In portable
mode, Dwarf Therapist only uses and stores files next to the application
executable. Configuration is stored in the application directory on Windows,
"../Resources" on macOS, "../etc" on Linux. Data directory is "data" on
Windows, "../Resources" on macOS, "../share" on Linux.

### `BUILD_DEVMODE` (default: `OFF`)

If `ON` build an executable that use `--devmode` with the current source
directory by default. Configuration is stored in the application directory and
user data (written by DT) is stored in "share". This option also disable
installation.
