#include "defaultfonts.h"
#include <QtGui>

#ifdef Q_OS_WIN
static const char* header_font_name = "Segoe UI";
static const int header_font_size = 9;
static const char* main_font_name = "Segoe UI";
static const int main_font_size = 9;
static const char* row_font_name = "Segoe UI";
static const int row_font_size = 8;
static const char* tooltip_font_name = "Segoe UI";
static const int tooltip_font_size = 9;
#else
#ifdef Q_OS_LINUX
static const char* header_font_name = "DejaVu Sans";
static const int header_font_size = 10;
static const char* main_font_name = "DejaVu Sans";
static const int main_font_size = 10;
static const char* row_font_name = "DejaVu Sans";
static const int row_font_size = 8;
static const char* tooltip_font_name = "DejaVu Sans";
static const int tooltip_font_size = 10;
#else
#ifdef Q_OS_MAC
static const char* header_font_name = "Lucida Grande";
static const int header_font_size = 11;
static const char* main_font_name = "Lucida Grande";
static const int main_font_size = 13;
static const char* row_font_name = "Lucida Grande";
static const int row_font_size = 10;
static const char* tooltip_font_name = "Lucida Grande";
static const int tooltip_font_size = 13;
#endif
#endif
#endif

const char* DefaultFonts::getHeaderFontName() {
    return header_font_name;
}

int DefaultFonts::getHeaderFontSize() {
    return header_font_size;
}

const char* DefaultFonts::getMainFontName() {
    return main_font_name;
}

int DefaultFonts::getMainFontSize() {
    return main_font_size;
}

const char* DefaultFonts::getRowFontName() {
    return row_font_name;
}

int DefaultFonts::getRowFontSize() {
    return row_font_size;
}

const char* DefaultFonts::getTooltipFontName() {
    return tooltip_font_name;
}

int DefaultFonts::getTooltipFontSize() {
    return tooltip_font_size;
}
