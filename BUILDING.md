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

### Setting capabilities on Debian-based distribution

Debian-based distribution have [Yama LSM](https://www.kernel.org/doc/Documentation/security/Yama.txt)
enabled by default. For Dwarf Therapist to be able to access Dwarf Fortress
memory, you need to add the `CAP_SYS_PTRACE` capability to Dwarf Therapist
executable (replace `<install_prefix>` with the installation path):

    sudo apt-get install libcap2-bin
    sudo setcap cap_sys_ptrace=eip <install_prefix>/bin/DwarfTherapist

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

