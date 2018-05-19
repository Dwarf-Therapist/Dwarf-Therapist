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

    TODO

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

