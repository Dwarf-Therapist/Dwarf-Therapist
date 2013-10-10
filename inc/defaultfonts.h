#ifndef DEFAULT_FONTS_H
#define DEFAULT_FONTS_H

class DefaultFonts {
public:
    static const char* getHeaderFontName();
    static int getHeaderFontSize();
    static const char* getMainFontName();
    static int getMainFontSize();
    static const char* getRowFontName();
    static int getRowFontSize();
    static const char* getTooltipFontName();
    static int getTooltipFontSize();
private:
    DefaultFonts();
};
#endif//DEFAULT_FONTS_H
