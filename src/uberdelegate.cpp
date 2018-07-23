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
#include "uberdelegate.h"
#include "dwarfmodel.h"
#include "dwarfmodelproxy.h"
#include "gamedatareader.h"
#include "dwarf.h"
#include "defines.h"
#include "columntypes.h"
#include "utils.h"
#include "dwarftherapist.h"
#include "skill.h"
#include "labor.h"
#include "defaultfonts.h"
#include "item.h"

#include "viewcolumn.h"
#include "gridview.h"

#include <QPainter>
#include <QSettings>

const float UberDelegate::MIN_DRAW_SIZE = 0.05625f;
const float UberDelegate::MAX_CELL_FILL = 0.76f;

UberDelegate::UberDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
    , m_model(0)
    , m_proxy(0)
{
    read_settings();
    connect(DT, SIGNAL(settings_changed()), this, SLOT(read_settings()));

    // Build a star shape for drawing later (http://doc.trolltech.com/4.5/itemviews-stardelegate-starrating-cpp.html)
    double pi = 3.14; // nobody cares, these are tiny
    for (int i = 0; i < 5; ++i) {
        // star points are 5 per circle which are 2pi/5 radians apart
        // we want to cross center by drawing points that are 4pi/5 radians apart
        // in order. The winding-fill will fill it in nicely as we draw each point
        // skip a point on each iteration by moving 4*pi/5 around the circle
        // which is basically .8 * pi
        m_star_shape << QPointF(
                            0.4 * cos(i * pi * 0.8), // x
                            0.4 * sin(i * pi * 0.8)  // y
                            );
    }

    m_diamond_shape << QPointF(0.5, 0.1) //top
                    << QPointF(0.75, 0.5) // right
                    << QPointF(0.5, 0.9) //bottom
                    << QPointF(0.25, 0.5); // left
}

void UberDelegate::read_settings() {
    QSettings *s = DT->user_settings();
    s->beginGroup("options");
    auto_contrast = s->value("auto_contrast", true).toBool();
    show_aggregates = s->value("show_aggregates", true).toBool();
    s->beginGroup("colors");
    color_skill = s->value("skill").value<QColor>();
    color_dirty_border = s->value("dirty_border").value<QColor>();
    color_had_mood = s->value("had_mood_border").value<QColor>();
    color_mood = s->value("highest_mood_border").value<QColor>();
    color_active_labor = s->value("active_labor").value<QColor>();
    color_active_group = s->value("active_group").value<QColor>();
    color_inactive_group= s->value("inactive_group").value<QColor>();
    color_partial_group = s->value("partial_group").value<QColor>();
    color_guides = s->value("guides").value<QColor>();
    color_border = s->value("border").value<QColor>();
    s->endGroup(); //colors
    s->beginGroup("grid");
    cell_size = s->value("cell_size", DEFAULT_CELL_SIZE).toInt();
    cell_padding = s->value("cell_padding", 0).toInt();
    cell_size += (cell_padding*2)+2; //increase the cell size by padding
    m_skill_drawing_method = static_cast<SKILL_DRAWING_METHOD>(s->value("skill_drawing_method", SDM_NUMERIC).toInt());
    draw_happiness_icons = s->value("happiness_icons",false).toBool();
    color_mood_cells = s->value("color_mood_cells",false).toBool();
    color_health_cells = s->value("color_health_cells",true).toBool();
    color_attribute_syns = s->value("color_attribute_syns",true).toBool();
    color_pref_matches = s->value("color_pref_matches",false).toBool();
    m_fnt = s->value("font", QFont(DefaultFonts::getRowFontName(), DefaultFonts::getRowFontSize())).value<QFont>();
    gradient_cell_bg = s->value("shade_cells",true).toBool();
    s->endGroup(); //grid
    s->endGroup(); //options
}

void UberDelegate::paint(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &proxy_idx) const {
    if (!proxy_idx.isValid()) {
        return;
    }

    bool cell_is_agg = proxy_idx.data(DwarfModel::DR_IS_AGGREGATE).toBool();
    bool drawing_aggregate = false;
    if(m_model && m_model->current_grouping() != DwarfModel::GB_NOTHING && show_aggregates && cell_is_agg)
        drawing_aggregate = true;

    //name column's cells are drawn differently
    if (proxy_idx.column() == 0) {
        QStyledItemDelegate::paint(p, opt, proxy_idx);
    }else{

        //all other cells
        paint_cell(p, opt, proxy_idx,drawing_aggregate);

        //vertical guide lines when columns are selected
        if (m_model && proxy_idx.column() == m_model->selected_col()) {
            p->save();
            p->setPen(QPen(color_guides));
            p->drawLine(opt.rect.topLeft(), opt.rect.bottomLeft());
            p->drawLine(opt.rect.topRight(), opt.rect.bottomRight());
            p->restore();
        }
    }
    paint_guide_borders(opt,p);
}

void UberDelegate::paint_cell(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &idx, const bool drawing_aggregate) const {
    QModelIndex model_idx = idx;
    if (m_proxy)
        model_idx = m_proxy->mapToSource(idx);

    COLUMN_TYPE type = static_cast<COLUMN_TYPE>(model_idx.data(DwarfModel::DR_COL_TYPE).toInt());
    QRect adjusted = opt.rect.adjusted(cell_padding, cell_padding, -cell_padding, -cell_padding);

    float rating = model_idx.data(DwarfModel::DR_RATING).toFloat();
    QString text_rating = model_idx.data(DwarfModel::DR_DISPLAY_RATING).toString();
    float limit = 100.0;

    Dwarf *d = 0;
    if(m_model){
        d = m_model->get_dwarf_by_id(idx.data(DwarfModel::DR_ID).toInt());
    }
//    if(!d && !drawing_aggregate){
//        return QStyledItemDelegate::paint(p, opt, idx);
//    }

    int state = idx.data(DwarfModel::DR_STATE).toInt();
    QColor state_color = QColor(Qt::transparent);
    if(m_model){
        ViewColumn *vc = m_model->current_grid_view()->get_column(idx.column());
        if(vc){
            state_color = vc->get_state_color(state);
        }
    }

    switch (type) {
    case CT_SKILL:
    {
        QColor bg = paint_bg(adjusted, p, opt, idx);
        limit = 15.0;
        if(rating >= 0){
            paint_values(adjusted, rating, text_rating, bg, p, opt, idx, 0, 0, limit, 0, 0);
        }
        if(d){
            paint_mood_cell(adjusted,p,opt,idx,model_idx.data(DwarfModel::DR_OTHER_ID).toInt(),false,d);
        }
    }
        break;
    case CT_LABOR:
    {
        if (!drawing_aggregate && d) {
            int labor_id = idx.data(DwarfModel::DR_LABOR_ID).toInt();
            QColor bg = paint_bg_active(adjusted,d->labor_enabled(labor_id),p,opt,idx,state,state_color);
            limit = 15.0;
            if(rating >= 0){
                paint_values(adjusted, rating, text_rating, bg, p, opt, idx, 0, 0, limit, 0, 0);
            }
            paint_mood_cell(adjusted,p,opt,idx,GameDataReader::ptr()->get_labor(labor_id)->skill_id, d->is_labor_state_dirty(labor_id),d);
        }else {
            paint_labor_aggregate(adjusted, p, opt, idx);
        }
    }
        break;
    case CT_HAPPINESS:
    {
        paint_bg(adjusted, p, opt, idx, true, state_color);
        if(draw_happiness_icons || (d && d->in_stressed_mood())){
            paint_icon(adjusted,p,opt,idx);
        }else{
            paint_grid(adjusted, false, p, opt, idx);
        }
    }
        break;
    case CT_EQUIPMENT:
    {
        Item::ITEM_STATE i_status = static_cast<Item::ITEM_STATE>(idx.data(DwarfModel::DR_SPECIAL_FLAG).toInt());
        paint_bg(adjusted, p, opt, idx, true, state_color);
        paint_wear_cell(adjusted,p,opt,idx,i_status);
    }
        break;
    case CT_ITEMTYPE:
    {
        QColor bg = paint_bg(adjusted, p, opt, idx, true, model_idx.data(Qt::BackgroundColorRole).value<QColor>());
        //if we're drawing numbers, we only want to draw counts for squads
        //this is a special case because we're drawing different information if it's text mode
        if(m_skill_drawing_method == SDM_NUMERIC && rating == 100)
            rating = -1;

        if(rating != 100){
            //only show red squares for missing items (0-50 rating)
            paint_values(adjusted, rating/2.0f, text_rating, bg, p, opt, idx, 50.0f,5.0f,95.0f,0,0,false);
        }
        Item::ITEM_STATE i_status = static_cast<Item::ITEM_STATE>(idx.data(DwarfModel::DR_SPECIAL_FLAG).toInt());
        paint_wear_cell(adjusted,p,opt,idx,i_status);
    }
        break;
    case CT_ROLE: case CT_SUPER_LABOR: case CT_CUSTOM_PROFESSION:
    {
        bool is_dirty = false;
        bool is_active = false;
        bool cp_border = false;

        if(type == CT_CUSTOM_PROFESSION){
            QString custom_prof_name = idx.data(DwarfModel::DR_CUSTOM_PROF).toString();
            if(!custom_prof_name.isEmpty()){
                if(d && d->profession() == custom_prof_name){
                    cp_border = true;
                }
                is_dirty = d->is_custom_profession_dirty(custom_prof_name);
            }
        }

        int dirty_alpha = 255;
        int active_alpha = 255;
        int min_alpha = 75;

        if(d && d->can_set_labors()){
            if(idx.data(DwarfModel::DR_LABORS).canConvert<QVariantList>()){
                QVariantList labors = idx.data(DwarfModel::DR_LABORS).toList();
                int active_count = 0;
                int dirty_count = 0;
                foreach(QVariant id, labors){
                    if(d->labor_enabled(id.toInt())){
                        is_active = true;
                        active_count++;
                    }
                    if(d->is_labor_state_dirty(id.toInt())){
                        is_dirty = true;
                        dirty_count++;
                    }
                }
                float perc = 0.0;
                if(is_active){
                    perc = (float)active_count / labors.count();
                    if(labors.count() > 5){
                        if(perc <= 0.33)
                            active_alpha = 63;
                        else if(perc <= 0.66)
                            active_alpha = 127;
                        else if(perc <= 0.90)
                            active_alpha = 190;
                    }else{
                        active_alpha *= perc;
                    }
                }
                if(is_dirty){
                    if(dirty_count > 0){
                        dirty_alpha = (255 * ((float)dirty_count / labors.count()));
                        if(dirty_alpha < min_alpha)
                            dirty_alpha = min_alpha;
                    }
                }
            }
        }

        QColor bg;
        QColor bg_color = state_color; //color_active_labor;
        if(is_active){
            bg_color.setAlpha(active_alpha);
        }
        bg = paint_bg_active(adjusted, is_active, p, opt, idx, state, bg_color);

        if(type == CT_ROLE){
            paint_values(adjusted, rating, text_rating, bg, p, opt, idx, 50.0f, 5.0f, 95.0f, 42.5f, 57.5f);
        }else if(rating >= 0){
            limit = 15.0f;
            paint_values(adjusted, rating, text_rating, bg, p, opt, idx, 0, 0, limit, 0, 0);
        }

        if(is_dirty){ //dirty border always has priority
            QColor color_dirty_adjusted = color_dirty_border;
            color_dirty_adjusted.setAlpha(dirty_alpha);
            paint_border(adjusted,p,color_dirty_adjusted);
            paint_grid(adjusted,false,p,opt,idx,false);
        }else if(cp_border){ //border for matching custom prof
            paint_border(adjusted,p,color_active_labor);
            paint_grid(adjusted, false, p, opt, idx,false);
        }else{ //normal border, or role pref border
            int pref_alpha = idx.data(DwarfModel::DR_SPECIAL_FLAG).toInt();
            if(color_pref_matches && type == CT_ROLE && pref_alpha > 0){
                if(pref_alpha < min_alpha)
                    pref_alpha = min_alpha;
                else if(pref_alpha > 255)
                    pref_alpha = 255;
                QColor color_prefs = Role::color_has_prefs();
                color_prefs.setAlpha(pref_alpha);
                paint_border(adjusted,p,color_prefs);
                paint_grid(adjusted, false, p, opt, idx, false);
            }else{
                paint_grid(adjusted, false, p, opt, idx);
            }
        }

    }
        break;
    case CT_IDLE:
    {
        paint_bg(adjusted, p, opt, idx, true, model_idx.data(Qt::BackgroundColorRole).value<QColor>());
        paint_icon(adjusted,p,opt,idx);
    }
        break;
    case CT_PROFESSION:
    {
        paint_bg(adjusted, p, opt, idx, true, model_idx.data(Qt::BackgroundColorRole).value<QColor>());
        paint_icon(adjusted,p,opt,idx);
    }
        break;
    case CT_HIGHEST_MOOD:
    {
        paint_bg(adjusted, p, opt, idx, true, model_idx.data(Qt::BackgroundColorRole).value<QColor>());
        paint_icon(adjusted,p,opt,idx);

        bool had_mood = idx.data(DwarfModel::DR_SPECIAL_FLAG).toBool();
        if(had_mood){
            p->save();
            QRect moodr = adjusted;
            moodr.adjust(2,2,-2,-2);
            p->setPen(QPen(Qt::darkRed,2));
            p->drawLine(moodr.bottomLeft(), moodr.topRight());
            p->restore();
        }
    }
        break;
    case CT_TRAIT: case CT_BELIEF:
    {
        QColor bg = paint_bg(adjusted, p, opt, idx);
        if(type==CT_TRAIT && rating == -1)
            rating = 50; //don't draw if the unit doesn't have the trait at all
        paint_values(adjusted, rating, text_rating, bg, p, opt, idx, 50, 10, 90);
        int alpha = idx.data(DwarfModel::DR_SPECIAL_FLAG).toInt();
        if(alpha > 0){
            paint_border(adjusted,p,QColor(168, 10, 44, alpha));
            paint_grid(adjusted, false, p, opt, idx, false);
        }else{
            paint_grid(adjusted, false, p, opt, idx);
        }
    }
        break;
    case CT_ATTRIBUTE:
    {
        QColor bg = paint_bg(adjusted, p, opt, idx);
        paint_values(adjusted, rating, text_rating, bg, p, opt, idx, 50.0f, 2.0f, 98.0f,30.0f,70.0f);

        if(color_attribute_syns && idx.data(DwarfModel::DR_SPECIAL_FLAG).toInt() > 0){
            paint_border(adjusted,p,Attribute::color_affected_by_syns());
            paint_grid(adjusted, false, p, opt, idx, false);
        }else{
            paint_grid(adjusted, false, p, opt, idx);
        }
    }
        break;
    case CT_WEAPON:
    {
        QColor bg = paint_bg(adjusted, p, opt, idx);
        paint_values(adjusted, rating, text_rating, bg, p, opt, idx, 50.0f, 1, 99, 49, 51, true);
        paint_grid(adjusted, false, p, opt, idx);

    }
        break;
    case CT_FLAGS:
    {
        if(d){
            int bit_pos = idx.data(DwarfModel::DR_OTHER_ID).toInt();
            paint_bg_active(adjusted, d->get_flag_value(bit_pos), p, opt, idx, state, state_color);
            paint_grid(adjusted,d->is_flag_dirty(bit_pos),p,opt,idx);
        }else{
            QStyledItemDelegate::paint(p, opt, idx);
        }
    }
        break;
    case CT_TRAINED:
    {
        QColor bg = paint_bg(adjusted, p, opt, idx, false, model_idx.data(Qt::BackgroundColorRole).value<QColor>());
        //arbitrary ignore range is used just to hide tame animals
        paint_values(adjusted, rating, text_rating, bg, p, opt, idx, 50.0f, 1.0f, 95.0f, 49.9f, 50.1f, true);
        paint_grid(adjusted, false, p, opt, idx);
    }
        break;
    case CT_KILLS:
    {
        QColor bg = paint_bg(adjusted, p, opt, idx, false, model_idx.data(Qt::BackgroundColorRole).value<QColor>());
        paint_values(adjusted, rating, text_rating, bg, p, opt, idx, 50.0f, 1.0f, 95.0f, 49.9f, 50.1f, true);
        paint_grid(adjusted, false, p, opt, idx);
    }
        break;
    case CT_PREFERENCE:
    {
        QColor bg = paint_bg(adjusted, p, opt, idx, false, model_idx.data(Qt::BackgroundColorRole).value<QColor>());
        if (rating > 0)
            paint_values(adjusted, rating, text_rating, bg, p, opt, idx, 0.0f, 0.0f, 100.0f, 0.0f, 0.0f, true);
        paint_grid(adjusted, false, p, opt, idx);
    }
        break;
    case CT_HEALTH:
    {
        QColor bg = paint_bg(adjusted, p, opt, idx, false, model_idx.data(Qt::BackgroundColorRole).value<QColor>());

        //draw the symbol text in bold
        p->save();
        if (rating != 0) {
            if(color_health_cells){
                p->setPen(model_idx.data(Qt::TextColorRole).value<QColor>());
            }else{
                if(auto_contrast){
                    p->setPen(complement(bg));
                }else{
                    p->setPen(Qt::black);
                }
            }

            QFont tmp = m_fnt;
            tmp.setBold(true);
            p->setFont(tmp);
            p->drawText(opt.rect, Qt::AlignCenter, text_rating);
        }
        p->restore();

        paint_grid(adjusted, false, p, opt, idx);
    }
        break;
    case CT_NEED:
    {
        paint_bg(adjusted, p, opt, idx, false, state_color);
        limit = 15.0f;
        if (rating > 0)
            paint_values(adjusted, rating, text_rating, state_color, p, opt, idx, 0, 0, limit, 0, 0);
        paint_grid(adjusted, false, p, opt, idx);
    }
        break;
    case CT_SPACER:
    case CT_DEFAULT:
    default:
    {
        if(adjusted.width() > 0){
            paint_bg(adjusted, p, opt, idx, false, QColor(Qt::transparent));
        }
        break;
    }
    }
}

void UberDelegate::paint_icon(const QRect &adjusted, QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &proxy_idx) const
{
    QModelIndex idx = proxy_idx;
    if (m_proxy)
        idx = m_proxy->mapToSource(proxy_idx);

    p->save();
    QIcon icon = idx.data(Qt::DecorationRole).value<QIcon>();
    QRect adj_with_borders(adjusted.topLeft(),adjusted.bottomRight());
    adj_with_borders.adjust(0,0,-2,-2); //adjust for the grid lines so the image is inside
    QPixmap pixmap = icon.pixmap(adj_with_borders.size());
    QSize iconSize = icon.actualSize(pixmap.size());
    //line width is one, so center the image inside that
    QPointF topLeft(adjusted.x() + ((adjusted.width()-iconSize.width())/(float)2)
                    ,adjusted.y() + ((adjusted.height()-iconSize.height())/(float)2));
    p->drawPixmap(topLeft,pixmap);
    p->restore();

    paint_grid(adjusted, false, p, opt, idx);
}

QColor UberDelegate::paint_bg_active(const QRect &adjusted, bool active, QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &proxy_idx, const int &state, const QColor &active_col_override) const{
    QModelIndex idx = proxy_idx;
    if (m_proxy)
        idx = m_proxy->mapToSource(proxy_idx);

    QColor bg = idx.data(DwarfModel::DR_DEFAULT_BG_COLOR).value<QColor>();
    p->save();
    p->fillRect(opt.rect, bg);


    if(active || state == ViewColumn::STATE_DISABLED || state == ViewColumn::STATE_ACTIVE){ //always draw disabled or active
        if(active_col_override != QColor(Qt::black))
            bg = active_col_override;
        else
            bg = color_active_labor;
    }

    if(gradient_cell_bg){
        QLinearGradient grad(adjusted.topLeft(),adjusted.bottomRight());
        grad.setColorAt(0,bg);
        bg.setAlpha(70);
        grad.setColorAt(1,bg);
        p->fillRect(adjusted, grad);
    }else{
        p->fillRect(adjusted, QBrush(bg));
    }
    p->restore();

    return bg;
}

QColor UberDelegate::paint_bg(const QRect &adjusted, QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &proxy_idx, const bool use_gradient, const QColor &col_override) const{
    QModelIndex idx = proxy_idx;
    if (m_proxy)
        idx = m_proxy->mapToSource(proxy_idx);

    QColor bg = idx.data(DwarfModel::DR_DEFAULT_BG_COLOR).value<QColor>();
    p->save();
    p->fillRect(opt.rect, bg);

    if (col_override != QColor(Qt::black))
        bg = col_override;

    if(use_gradient && gradient_cell_bg){
        QLinearGradient grad(adjusted.topLeft(),adjusted.bottomRight());
        grad.setColorAt(0,bg);
        bg.setAlpha(75);
        grad.setColorAt(1,bg);
        p->fillRect(adjusted, grad);
    }else{
        p->fillRect(adjusted, QBrush(bg));
    }
    p->restore();

    return bg;
}

QColor UberDelegate::get_pen_color(const QColor bg) const{
    QColor c = Qt::black;
    if (auto_contrast){
        c = complement(bg);
        if(c.toHsv().value() > 50){
            return QColor(Qt::gray);
        }else{
            return QColor(Qt::black);
        }
    }
    return c;
}

void UberDelegate::paint_values(const QRect &adjusted, float rating, QString text_rating, QColor bg, QPainter *p, const QStyleOptionViewItem &opt,
                                const QModelIndex &idx, float median, float min_limit, float max_limit, float min_ignore, float max_ignore, bool bold_text) const{

    QColor color_fill = color_skill;

    QPen pn;
    pn.setColor(get_pen_color(bg));
    pn.setWidth(0);

    if (auto_contrast)
        color_fill = complement(bg);

    QModelIndex model_idx = idx;
    if (m_proxy)
        model_idx = m_proxy->mapToSource(idx);

    //some columns will ignore a mid range of values and draw nothing
    //however this is NEVER done when using the numeric drawing method
    if((min_ignore != 0 || max_ignore != 0) && m_skill_drawing_method != SDM_NUMERIC){
        if (rating >= min_ignore && rating <= max_ignore){
            return; //paint nothing for mid ranges
        }
    }

    //if we have a median, draw values below it in red
    //if the auto-contrast is drawing in white, it means we've got a very dark background
    //so also adjust our negative drawing color to a bright orange
    if (rating < median){
        QColor neg = QColor("#DB241A");
        if(auto_contrast &&  color_fill.toHsv().value() == 255){
            neg.setGreen(neg.green() + 76);
            neg.setBlue(0);
            color_fill = QColor::fromHsv(neg.hue(), neg.saturation(), 200);
        }else{
            color_fill = neg;
        }
        pn.setColor(Qt::gray);
    }

    //check the median passed in and covert to normal: 0-50-100 if necessary
    float adj_rating = rating;
    if(median > 0){
        if(median != 50.0f){
            if(rating < median){
                adj_rating = adj_rating / median * 50.0f;
            }else{
                adj_rating = ((adj_rating - median) / (100.0f-median) * 50.0f) + 50.0f;
            }
        }
        //        //also invert the value if it was below the median, and rescale our drawing values from 0-50 and 50-100
        //        if(adj_rating > 50.0f){
        //            adj_rating = (adj_rating - 50.0f) * 2.0f;
        //        }else{
        //            adj_rating = 100 - (adj_rating * 2.0f);
        //        }
    }

    p->save();
    switch(m_skill_drawing_method) {
    default:
    case SDM_GROWING_CENTRAL_BOX:
        if (adj_rating >= max_limit || adj_rating <= min_limit) {
            // draw diamond
            p->setRenderHint(QPainter::Antialiasing);
            p->setPen(pn);
            p->setBrush(QBrush(color_fill));
            p->translate(opt.rect.x() + 2, opt.rect.y() + 2);
            p->scale(opt.rect.width() - 4, opt.rect.height() - 4);
            p->drawPolygon(m_diamond_shape);
        } else if (adj_rating > -1) {
            //0.05625 (MIN_DRAW_SIZE) is the smallest dot we can draw here, so scale to ensure the smallest exp value (1/500 or .002) can always be drawn
            //relative to our maximum limit for this range. this could still be improved to take into account the cell size, as having even
            //smaller cells than the default (16) may not draw very low dabbling skill xp levels

            double size = MAX_CELL_FILL;
            if((median > 0 && adj_rating > 50.0f) || median == 0){
                if(max_ignore > max_limit){
                    size = ((adj_rating-median) * (MAX_CELL_FILL - MIN_DRAW_SIZE)) / (max_limit - median) + MIN_DRAW_SIZE;
                }else{
                    size = ((adj_rating-max_ignore) * (MAX_CELL_FILL - MIN_DRAW_SIZE)) / (max_limit - max_ignore) + MIN_DRAW_SIZE;
                }
            }else{
                if(min_ignore < min_limit){
                    size = ((adj_rating-min_limit) * (MAX_CELL_FILL - MIN_DRAW_SIZE)) / (median - min_limit);
                }else{
                    size = ((adj_rating-min_limit) * (MAX_CELL_FILL - MIN_DRAW_SIZE)) / (min_ignore - min_limit);
                }
                size = MAX_CELL_FILL - size + MIN_DRAW_SIZE;
            }

            //double size = (((adj_rating-0) * (perc_of_cell - 0.05625)) / (100.0f-0)) + 0.05625;
            //size = roundf(size * 100) / 100; //this is to aid in the problem of an odd number of pixel in an even size cell, or vice versa
            double inset = (1.0f - size) / 2.0f;
            p->translate(adjusted.x(),adjusted.y());
            p->scale(adjusted.width(),adjusted.height());
            p->fillRect(QRectF(inset, inset, size, size), QBrush(color_fill));
        }
        break;
    case SDM_GROWING_FILL:
        if (rating >= max_limit) {
            // draw diamond
            p->setRenderHint(QPainter::Antialiasing);
            p->setPen(pn);
            p->setBrush(QBrush(color_fill));
            p->translate(opt.rect.x() + 2, opt.rect.y() + 2);
            p->scale(opt.rect.width() - 4, opt.rect.height() - 4);
            p->drawPolygon(m_diamond_shape);
        } else if (rating > -1 && rating < max_limit) {
            float size = 0.8f * (rating / max_limit) + 0.1f;
            p->translate(adjusted.x(), adjusted.y());
            p->scale(adjusted.width(), adjusted.height());
            p->fillRect(QRectF(0, 0, size, 1), QBrush(color_fill));
        }
        break;
    case SDM_GLYPH_LINES:
    {
        p->setBrush(QBrush(color_fill));
        //match pen to brush for glyphs
        pn.setColor(color_fill);
        p->setPen(pn);
        p->translate(adjusted.x(), adjusted.y());
        p->scale(adjusted.width(), adjusted.height());
        QVector<QLineF> lines;

        if(rating >= max_limit){
            p->resetTransform();
            p->setPen(pn);
            p->translate(adjusted.x() + adjusted.width()/2.0,
                         adjusted.y() + adjusted.height()/2.0);
            p->scale(adjusted.width(), adjusted.height());
            p->rotate(-18);
            p->setRenderHint(QPainter::Antialiasing);
            p->drawPolygon(m_star_shape, Qt::WindingFill);
        }else{
            //scale rating down to 0-14 + 1 to ensure dabbling is drawn
            adj_rating = floor((((rating-0)/(max_limit-0))*0.15)*100);

            switch ((int)adj_rating) {//((int)rating) {
            case 0: //dabbling
                lines << QLineF(QPointF(0.499, 0.5), QPointF(0.501, 0.5));
                break;
            case 15:
            {
                p->resetTransform();
                p->translate(adjusted.x() + adjusted.width()/2.0,
                             adjusted.y() + adjusted.height()/2.0);
                p->scale(adjusted.width(), adjusted.height());
                p->rotate(-18);
                p->setRenderHint(QPainter::Antialiasing);
                p->drawPolygon(m_star_shape, Qt::WindingFill);
            }
                break;
            case 14:
            {
                QPolygonF poly;
                poly << QPointF(0.5, 0.1)
                     << QPointF(0.5, 0.5)
                     << QPointF(0.9, 0.5);
                p->drawPolygon(poly);
            } // fallthrough
            case 13:
            {
                QPolygonF poly;
                poly << QPointF(0.1, 0.5)
                     << QPointF(0.5, 0.5)
                     << QPointF(0.5, 0.9);
                p->drawPolygon(poly);
            } // fallthrough
            case 12:
            {
                QPolygonF poly;
                poly << QPointF(0.9, 0.5)
                     << QPointF(0.5, 0.5)
                     << QPointF(0.5, 0.9);
                p->drawPolygon(poly);
            } // fallthrough
            case 11:
            {
                QPolygonF poly;
                poly << QPointF(0.1, 0.5)
                     << QPointF(0.5, 0.5)
                     << QPointF(0.5, 0.1);
                p->drawPolygon(poly);
            } // fallthrough
            case 10: // accomplished
                lines << QLineF(QPointF(0.5, 0.1), QPointF(0.9, 0.5));
                // fallthrough
            case 9: //professional
                lines << QLineF(QPointF(0.1, 0.5), QPointF(0.5, 0.1));
                // fallthrough
            case 8: //expert
                lines << QLineF(QPointF(0.5, 0.9), QPointF(0.1, 0.5));
                // fallthrough
            case 7: //adept
                lines << QLineF(QPointF(0.5, 0.9), QPointF(0.9, 0.5));
                // fallthrough
            case 6: //talented
                lines << QLineF(QPointF(0.5, 0.5), QPointF(0.5, 0.9));
                // fallthrough
            case 5: //proficient
                lines << QLineF(QPointF(0.5, 0.5), QPointF(0.9, 0.5));
                // fallthrough
            case 4: //skilled
                lines << QLineF(QPointF(0.5, 0.1), QPointF(0.5, 0.5));
                // fallthrough
            case 3: //competent
                lines << QLineF(QPointF(0.1, 0.5), QPointF(0.5, 0.5));
                // fallthrough
            case 2: //untitled
                lines << QLineF(QPointF(0.7, 0.3), QPointF(0.3, 0.7));
                // fallthrough
            case 1: //novice
                lines << QLineF(QPointF(0.3, 0.3), QPointF(0.7, 0.7));
                break;
            }
            p->drawLines(lines);
        }
    }
        break;
    case SDM_NUMERIC:
        if (rating > -1) { // don't draw 0s everywhere
            p->setPen(color_fill);
            //for some reason df's masterwork glyph's quality is reduced when using bold
            if(bold_text && !text_rating.contains(QChar(0x263C))){
                QFont tmp = m_fnt;
                tmp.setBold(true);
                p->setFont(tmp);
            }
            p->drawText(opt.rect, Qt::AlignCenter, text_rating);
        }
        break;
    }
    p->restore();
}

void UberDelegate::paint_wear_cell(const QRect &adjusted, QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &proxy_idx, const Item::ITEM_STATE i_status) const {
    if(!m_proxy){
        paint_grid(adjusted, false, p, opt, proxy_idx);
        return;
    }
    if(i_status > 0){
        paint_border(adjusted,p,Item::get_color(i_status));
        paint_grid(adjusted, false, p, opt, proxy_idx, false); //draw dirty border and guides
    }else{
        paint_grid(adjusted, false, p, opt, proxy_idx);
    }
}

void UberDelegate::paint_mood_cell(const QRect &adjusted, QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &proxy_idx, int skill_id, bool dirty, Dwarf *d) const {
    if(!m_proxy){ //skill legend won't have a proxy
        paint_grid(adjusted, false, p, opt, proxy_idx);
        return;
    }

    if(color_mood_cells && !dirty){ //dirty is always drawn over mood
        QList<Skill> skills = d->get_moodable_skills().values();
        if(d->get_moodable_skills().contains(skill_id)){
            QColor mood = color_mood;
            if(d->had_mood())
                mood = color_had_mood;
            paint_border(adjusted,p,mood);
            paint_grid(adjusted, dirty, p, opt, proxy_idx, false); //draw dirty border and guides
        }else{
            paint_grid(adjusted, dirty, p, opt, proxy_idx);
        }

    }else{
        paint_grid(adjusted, dirty, p, opt, proxy_idx);
    }
}

void UberDelegate::paint_border(const QRect &adjusted, QPainter *p, const QColor &color) const{
    QRect thick_border = adjusted;
    thick_border.adjust(1,1,0,0);

    p->setPen(QPen(color,2));

    QPoint topRight = QPoint(thick_border.topRight().x(),thick_border.topRight().y()+2);
    QPoint bottomRight = QPoint(thick_border.bottomRight().x(),thick_border.bottomRight().y()-2);

    QPoint topLeft = QPoint(thick_border.topLeft().x(),thick_border.topLeft().y()+2);
    QPoint bottomLeft = QPoint(thick_border.bottomLeft().x(),thick_border.bottomLeft().y()-2);

    p->drawLine(topRight,bottomRight);
    p->drawLine(topLeft,bottomLeft);
    p->drawLine(thick_border.topRight(),thick_border.topLeft());
    p->drawLine(thick_border.bottomLeft(),thick_border.bottomRight());
}


void UberDelegate::paint_labor_aggregate(const QRect &adjusted, QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &proxy_idx) const {
    if (!proxy_idx.isValid()) {
        return;
    }
    QModelIndex first_col = m_proxy->index(proxy_idx.row(), 0, proxy_idx.parent());
    if (!first_col.isValid()) {
        return;
    }

    int labor_id = proxy_idx.data(DwarfModel::DR_LABOR_ID).toInt();

    int dirty_count = 0;
    int enabled_count = 0;
    for (int i = 0; i < m_proxy->rowCount(first_col); ++i) {
        int dwarf_id = m_proxy->data(m_proxy->index(i, 0, first_col), DwarfModel::DR_ID).toInt();
        Dwarf *d = m_model->get_dwarf_by_id(dwarf_id);
        if (!d)
            continue;
        if (d->labor_enabled(labor_id))
            enabled_count++;
        if (d->is_labor_state_dirty(labor_id))
            dirty_count++;
    }

    QStyledItemDelegate::paint(p, opt, proxy_idx); // slap on the main bg
    //paint_bg(adjusted,p,opt,proxy_idx,false,QColor(Qt::magenta));
    p->save();
    if (m_proxy->rowCount(first_col) > 0 && enabled_count == m_proxy->rowCount(first_col)) {
        p->fillRect(adjusted, QBrush(color_active_group));
    } else if (enabled_count > 0) {
        p->fillRect(adjusted, QBrush(color_partial_group));
        if(auto_contrast){
            p->setPen(complement(color_partial_group));
        }else{
            p->setPen(Qt::black);
        }
        QFont tmp = m_fnt;
        //tmp.setBold(true);
        p->setFont(tmp);
        p->drawText(opt.rect, Qt::AlignCenter, QString::number(enabled_count));

    } else {
        p->fillRect(adjusted, QBrush(color_inactive_group));
    }
    p->restore();
    paint_grid(adjusted, dirty_count > 0, p, opt, proxy_idx);

}

void UberDelegate::paint_grid(const QRect &adjusted, bool dirty, QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &, bool draw_border) const {
    Q_UNUSED(opt);
    p->save();
    p->setBrush(Qt::NoBrush);

    QRect rec_border = adjusted;
    //dirty labor
    if(dirty){
        p->setPen(QPen(color_dirty_border,2));
        rec_border.adjust(1,1,0,0); //offset for thicker border
    }else{
        //normal white border
        p->setPen(color_border);
    }

    //draw border (dirty or otherwise), may have already been drawn for some cells (moodable skill/labor)
    if(draw_border || dirty){
        p->drawLine(rec_border.topRight(),rec_border.bottomRight());
        p->drawLine(rec_border.topLeft(),rec_border.bottomLeft());
        p->drawLine(rec_border.topRight(),rec_border.topLeft());
        p->drawLine(rec_border.bottomLeft(),rec_border.bottomRight());
    }
    p->restore();
}

void UberDelegate::paint_guide_borders(const QStyleOptionViewItem &opt, QPainter *p) const {
    //draw either the guide along the top/bottom or the aggregate border
    if(opt.state.testFlag(QStyle::State_Selected) || opt.state.testFlag(QStyle::State_MouseOver)) {
        p->save();
        p->setPen(color_guides);
        p->drawLine(opt.rect.topLeft(), opt.rect.topRight());
        p->drawLine(opt.rect.bottomLeft(), opt.rect.bottomRight());
        p->restore();
    }
}

QSize UberDelegate::sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &idx) const {
    if (idx.column() == 0)
        return QStyledItemDelegate::sizeHint(opt, idx);
    return QSize(cell_size, cell_size);
}
