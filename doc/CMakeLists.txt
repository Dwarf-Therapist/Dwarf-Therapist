cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)
project(DwarfTherapistManual)

find_package(LATEX COMPONENTS PDFLATEX)
if(LATEX_FOUND AND LATEX_PDFLATEX_FOUND)
    include(UseLATEX)
    add_latex_document("Dwarf Therapist.tex"
        IMAGE_DIRS images
        FORCE_PDF)

    if(UNIX AND NOT APPLE)
        # Installation is currently Linux-only
        install(FILES "Dwarf Therapist.pdf"
                DESTINATION share/doc/dwarftherapist)
    endif()
endif()
