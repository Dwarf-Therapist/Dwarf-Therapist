Ptrace permissions on Linux
===========================

Dwarf Therapist needs to be able to trace Dwarf Fortress to access its memory, but [Yama LSM](https://www.kernel.org/doc/Documentation/security/Yama.txt) may prevent that depending on your distribution settings.

You can check your `ptrace_scope` settings by running `sysctl kernel.yama.ptrace_scope` or `cat /proc/sys/kernel/yama/ptrace_scope`. If your `ptrace_scope` value is 0, no further action is needed. If `ptrace_scope` is 1 or 2, you will need to use one of the methods below.


Set ptracer in preloaded library
--------------------------------

This solution works with `ptrace_scope` set to 1, and it does not require root privileges. It works by creating a library that allow any process to trace it and preloading that library when running Dwarf Fortress.

The [patch_df_ptracer](patch_df_ptracer) script will compile the library, install it and patch DF and DFHack launcher scripts to preload it. It requires *gcc* to be installed, and *optionally zenity* for choosing DF directory from a GUI dialog.

Run `./patch_df_ptracer /path/to/df` (replace `/path/to/df` with the path to Dwarf Fortress installation directory) or simply `./patch_df_ptracer` to use GUI selection (requires zenity).


Adding `CAP_SYS_PTRACE` capability
----------------------------------

This solution works with `ptrace_scope` up to 2, it requires root privileges. *libcap* must be installed (`libcap2-bin` package on Debian/Ubuntu).

It can be done manually by running `setcap cap_sys_ptrace=eip /path/to/dwarftherapist` as root (replace `/path/to/dwarftherapist` with the path to Dwarf Therapist executable). The capability must be added again, if the executable is modified.

Or use the [ptrace_cap_wrapper](ptrace_cap_wrapper) script to run Dwarf Therapist. It will check the permissions and try to add the `CAP_SYS_PTRACE` capability if required.  You need to pass it the command line for Dwarf Therapist, e.g. `./ptrace_wrapper dwarftherapist`. The first parameter must be the Dwarf Therapist executable itself, it won't work with any wrapper script (including the AppImage bundle).


Changing `ptrace_scope` value
-----------------------------

`ptrace_scope` value may be changed temporarily by running `sysctl kernel.yama.ptrace_scope=0` or `tee /proc/sys/kernel/yama/ptrace_scope <<< 0` as root. It will only last until the next boot.

It can be changed permanently by editing `/etc/sysctl.conf` or one of the file in `/etc/sysctl.d/` (check if any one of them already changes the value of `kernel.yama.ptrace_scope`), read `sysctl.conf(5)` man page for more details.


Starting DF as child of DT
--------------------------

This solution works with `ptrace_scope` set to 1, and it does not require root privileges. But it requires that DF and DT are started together. If any of them is closed, both need to be restarted again.

Dwarf Fortress need to be executed as a child of Dwarf Therapist. This can be done by using the following script (edit the paths according to your installation setup).

```bash
#!/bin/bash
set -e
{
    # Setup DF environment here
    cd /path/to/df
    exec ./df
} &

# Setup DT environment here
exec dwarftherapist
```

