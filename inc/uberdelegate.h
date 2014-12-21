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
#ifndef UBER_DELEGATE_H
#define UBER_DELEGATE_H

#include <QStyledItemDelegate>

class DwarfModel;
class DwarfModelProxy;

class UberDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    UberDelegate(QObject *parent = 0);
    void paint(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &proxy_idx) const;

    typedef enum {
        SDM_GROWING_CENTRAL_BOX = 0,
        SDM_GLYPH_LINES,
        SDM_GROWING_FILL,
        SDM_NUMERIC,
        SDM_TOTAL_METHODS
    } SKILL_DRAWING_METHOD;

    static QString name_for_method(const SKILL_DRAWING_METHOD &method) {
        switch (method) {
            case SDM_GROWING_CENTRAL_BOX: return "Growing Central Box";
            case SDM_GLYPH_LINES: return "Line Glyphs";
            case SDM_GROWING_FILL: return "Growing Fill";
            case SDM_NUMERIC: return "Text";
            default: return "UNKNOWN SDM";
        }
    }

    virtual QSize sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &idx) const;

    void set_model(DwarfModel *model) {m_model = model;}
    void set_proxy(DwarfModelProxy *proxy) {m_proxy = proxy;}

    QColor color_active_labor;
    QColor color_active_group;
    QColor color_inactive_group;
    QColor color_partial_group;
    QColor color_guides;
    QColor color_border;
    QColor color_dirty_border;
    QColor color_mood;
    QColor color_had_mood;
    QColor color_skill;

    int cell_size;
    int cell_padding;
    bool auto_contrast;
    bool show_aggregates;

private:
    DwarfModel *m_model;
    DwarfModelProxy *m_proxy;
    QPolygonF m_star_shape;
    QPolygonF m_diamond_shape;
    SKILL_DRAWING_METHOD m_skill_drawing_method;
    bool draw_happiness_icons;
    bool color_mood_cells;
    bool color_health_cells;
    bool color_attribute_syns;
    bool color_pref_matches;
    bool gradient_cell_bg;
    QFont m_fnt;

    void paint_cell(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &proxy_idx, const bool drawing_aggregate) const;

    void paint_grid(const QRect &adjusted, bool dirty, QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &proxy_idx, bool draw_border = true) const;

    void paint_guide_borders(const QStyleOptionViewItem &opt, QPainter *p, const bool drawing_aggregate) const;

    //! return the bg color that was painted
    //! drawing labor bg cells (shaded for active possibly)
    QColor paint_bg_active(const QRect &adjusted, bool active, QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &proxy_idx, const int &state, const QColor &active_col_override = Qt::black) const;
    //! drawing any other cell that cannot be active (non-labor)
    QColor paint_bg(const QRect &adjusted, QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &proxy_idx, const bool use_gradient = true, const QColor &col_override = Qt::black) const;

    void paint_values(const QRect &adjusted, float rating, QString text_rating, QColor bg, QPainter *p,
                    const QStyleOptionViewItem &opt, const QModelIndex &proxy_idx, float median = 50.0f,
                    float min_limit=5.0f, float max_limit=95.0f, float min_ignore=40.0f, float max_ignore=60.0f, bool bold_text = false) const;

    void paint_mood_cell(const QRect &adjusted, QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &proxy_idx, int skill_id, bool dirty) const;
    void paint_wear_cell(const QRect &adjusted, QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &proxy_idx, const int wear_level) const;
    void paint_labor_aggregate(const QRect &adjusted, QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &proxy_idx) const;
    void paint_icon(const QRect &adjusted, QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &proxy_idx) const;

    void paint_border(const QRect &adjusted, QPainter *p, const QColor &color) const;

    QColor get_pen_color(const QColor bg) const;

    private slots:
        void read_settings();
};

#endif
