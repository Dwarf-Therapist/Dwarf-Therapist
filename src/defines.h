/*
Dwarf Therapist
Copyright (c) 2009 Trey Stout (chmod)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
#ifndef DEFINES_H
#define DEFINES_H

#define DEFAULT_CELL_SIZE 16
#define DEFAULT_SPACER_WIDTH 4

#define ERROR_NO_VALID_LAYOUTS 500

#define GLOBAL_SORT_COL_IDX 1

#define REPO_OWNER "Dwarf-Therapist"
#define REPO_NAME "Dwarf-Therapist"

#ifdef Q_OS_WIN
#   define LAYOUT_SUBDIR "windows"
#elif defined(Q_OS_LINUX)
#   define LAYOUT_SUBDIR "linux"
#elif defined(Q_OS_MAC)
#   define LAYOUT_SUBDIR "osx"
#endif

#endif
