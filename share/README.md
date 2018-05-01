Data files for Dwarf Therapist
==============================

Gridviews
----------

Add a new file in gridviews in the `gridviews` directory. Files with a `.dtg`
extension are loaded in order by name.


Memory layouts
--------------

Add memory layouts in `memory_layouts/<os name>/`, where `<os name>` may be
`linux`, `osx`, or `windows` depending on your operating system. Memory layout
files may have any name as long as they have a `.ini` extension.


Game data
---------

The `game_data.ini` also contains titles and definitions for labors, jobs,
thoughts, attributes, default roles etc.

To override the default `game_data.ini`, a modified version should be placed
here with the desired changes. The original `game_data.ini` can be found in the
repository in the resources directory.

