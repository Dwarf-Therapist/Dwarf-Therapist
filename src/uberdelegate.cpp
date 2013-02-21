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
#include <QtGui>
#include "uberdelegate.h"
#include "dwarfmodel.h"
#include "dwarfmodelproxy.h"
#include "gamedatareader.h"
#include "dwarf.h"
#include "defines.h"
#include "gridview.h"
#include "columntypes.h"
#include "utils.h"
#include "dwarftherapist.h"
#include "militarypreference.h"
#include "skill.h"
#include "labor.h"
#include "customprofession.h"

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
    s->endGroup();
    s->endGroup();
    cell_size = s->value("options/grid/cell_size", DEFAULT_CELL_SIZE).toInt();    
    cell_padding = s->value("options/grid/cell_padding", 0).toInt();
    cell_size += (cell_padding*2)+2; //increase the cell size by padding
    auto_contrast = s->value("options/auto_contrast", true).toBool();
    draw_aggregates = s->value("options/show_aggregates", true).toBool();
    m_skill_drawing_method = static_cast<SKILL_DRAWING_METHOD>(s->value("options/grid/skill_drawing_method", SDM_GROWING_CENTRAL_BOX).toInt());
    draw_happiness_icons = s->value("options/grid/happiness_icons",true).toBool();
    color_mood_cells = s->value("options/grid/color_mood_cells",false).toBool();
    m_fnt = s->value("options/grid/font", QFont("Segoe UI", 8)).value<QFont>();
}

void UberDelegate::paint(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &proxy_idx) const {
    if (!proxy_idx.isValid()) {
        return;
    }
    if (proxy_idx.column() == 0) {        
        QStyledItemDelegate::paint(p, opt, proxy_idx);
        return;
    }

    paint_cell(p, opt, proxy_idx);

    if (m_model && proxy_idx.column() == m_model->selected_col()) {
        p->save();
        p->setPen(QPen(color_guides));
        p->drawLine(opt.rect.topLeft(), opt.rect.bottomLeft());
        p->drawLine(opt.rect.topRight(), opt.rect.bottomRight());
        p->restore();
    }
}

void UberDelegate::paint_cell(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &idx) const {
    QModelIndex model_idx = idx;
    if (m_proxy)
        model_idx = m_proxy->mapToSource(idx);

    COLUMN_TYPE type = static_cast<COLUMN_TYPE>(model_idx.data(DwarfModel::DR_COL_TYPE).toInt());        
    QRect adjusted = opt.rect.adjusted(cell_padding, cell_padding, -cell_padding, -cell_padding);

    float rating = model_idx.data(DwarfModel::DR_RATING).toFloat();
    QString text_rating = model_idx.data(DwarfModel::DR_DISPLAY_RATING).toString();
    float limit = 100.0;

    switch (type) {
    case CT_SKILL:
    {
        QColor bg = paint_bg(adjusted, false, p, opt, idx);
        limit = 15.0;
        if(rating >= 0)
            paint_values(adjusted, rating, text_rating, bg, p, opt, idx, 0, 0, limit, 0, 0);

        paint_mood_cell(adjusted,p,opt,idx,model_idx.data(DwarfModel::DR_SKILL_ID).toInt(),false);
    }
        break;
    case CT_LABOR:
    {
        bool agg = model_idx.data(DwarfModel::DR_IS_AGGREGATE).toBool();
        if (m_model->current_grouping() == DwarfModel::GB_NOTHING || !agg) {                        
            Dwarf *d = m_model->get_dwarf_by_id(idx.data(DwarfModel::DR_ID).toInt());
            if (!d) {
                return QStyledItemDelegate::paint(p, opt, idx);
            }
            int labor_id = idx.data(DwarfModel::DR_LABOR_ID).toInt();
            bool enabled = d->labor_enabled(labor_id);
            bool dirty = d->is_labor_state_dirty(labor_id);

            QColor bg = paint_bg(adjusted, enabled, p, opt, idx);
            limit = 15.0;
            if(rating >= 0)
                paint_values(adjusted, rating, text_rating, bg, p, opt, idx, 0, 0, limit, 0, 0);

            paint_mood_cell(adjusted,p,opt,idx,GameDataReader::ptr()->get_labor(labor_id)->skill_id, dirty);

        } else {
            if (draw_aggregates)
                paint_aggregate(adjusted, p, opt, idx);
        }
    }
        break;
    case CT_HAPPINESS:
    {
        paint_bg(adjusted, false, p, opt, idx, true, model_idx.data(Qt::BackgroundColorRole).value<QColor>());        
        if(draw_happiness_icons){
            paint_icon(adjusted,p,opt,idx);
        }else{
            p->save();
            paint_grid(adjusted, false, p, opt, idx);
            p->restore();
        }
    }
        break;
    case CT_ROLE:
    {
        QColor bg = paint_bg(adjusted, false, p, opt, idx);        
        paint_values(adjusted, rating, text_rating, bg, p, opt, idx, 50.0f, 2.0f, 98.0f);
        paint_grid(adjusted, false, p, opt, idx);
    }
        break;
    case CT_IDLE:
    {        
        paint_bg(adjusted, false, p, opt, idx, true, model_idx.data(Qt::BackgroundColorRole).value<QColor>());
        paint_icon(adjusted,p,opt,idx);        
    }
        break;
    case CT_PROFESSION:
    {
        paint_icon(adjusted,p,opt,idx);
    }
        break;
    case CT_HIGHEST_MOOD:
    {
        paint_bg(adjusted, false, p, opt, idx, true, model_idx.data(Qt::BackgroundColorRole).value<QColor>());
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
    case CT_TRAIT:
    {
        QColor bg = paint_bg(adjusted, false, p, opt, idx);        
        paint_values(adjusted, rating, text_rating, bg, p, opt, idx, 50, 10, 90);
        paint_grid(adjusted, false, p, opt, idx);
    }
        break;
    case CT_ATTRIBUTE:
    {        
        QColor bg = paint_bg(adjusted, false, p, opt, idx);                        
        paint_values(adjusted, rating, text_rating, bg, p, opt, idx, 50.0f, 2.0f, 98.0f, 49.0f, 51.0f);
        paint_grid(adjusted, false, p, opt, idx);

    }
        break;
    case CT_WEAPON:
    {
        QColor bg = paint_bg(adjusted, false, p, opt, idx);        
        paint_values(adjusted, rating, text_rating, bg, p, opt, idx, 50.0f, 1, 99, 49, 51);
        paint_grid(adjusted, false, p, opt, idx);

    }
        break;
    case CT_MILITARY_PREFERENCE:
    {
        bool agg = model_idx.data(DwarfModel::DR_IS_AGGREGATE).toBool();
        if (m_model->current_grouping() == DwarfModel::GB_NOTHING || !agg) {
            paint_pref(adjusted, p, opt, idx);
        } else {
            if (draw_aggregates)
                paint_aggregate(adjusted, p, opt, idx);
        }
    }
        break;
    case CT_FLAGS:
    {
        paint_flags(adjusted, p, opt, idx);
        paint_grid(adjusted, false, p, opt, idx);
    }
        break;
    case CT_TRAINED:
    {
        QColor bg = paint_bg(adjusted, false, p, opt, idx);
        //arbitrary ignore range is used just to hide tame animals
        paint_values(adjusted, rating, text_rating, bg, p, opt, idx, 50.0f, 1.0f, 95.0f, 49.9f, 50.1f);
        paint_grid(adjusted, false, p, opt, idx);
    }
        break;
    case CT_DEFAULT:
    case CT_SPACER:
    default:
        paint_bg(adjusted, false, p, opt, idx);        
        if (opt.state & QStyle::State_Selected) {
            p->save();
            p->setPen(color_guides);
            p->drawLine(opt.rect.topLeft(), opt.rect.topRight());
            p->drawLine(opt.rect.bottomLeft(), opt.rect.bottomRight());
            p->restore();
        }
        break;
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

QColor UberDelegate::paint_bg(const QRect &adjusted, bool active, QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &proxy_idx, const bool use_gradient, const QColor &col_override) const{
    QModelIndex idx = proxy_idx;
    if (m_proxy)
        idx = m_proxy->mapToSource(proxy_idx);

    QColor bg = idx.data(DwarfModel::DR_DEFAULT_BG_COLOR).value<QColor>();
    p->save();
    p->fillRect(opt.rect, bg);

    if(!active){
        if (col_override != QColor(Qt::black))
            bg = col_override;
    }else
        bg = color_active_labor;

    if((use_gradient || active) && DT->user_settings()->value("options/grid/shade_cells",true).toBool()){
        QLinearGradient grad(adjusted.topLeft(),adjusted.bottomRight());
        bg.setAlpha(75);
        grad.setColorAt(1,bg);
        bg.setAlpha(255);
        grad.setColorAt(0,bg);
        p->fillRect(adjusted, grad);
    }else{        
        p->fillRect(adjusted, QBrush(bg));
    }
    p->restore();

    return bg;
}

void UberDelegate::paint_values(const QRect &adjusted, float rating, QString text_rating, QColor bg, QPainter *p, const QStyleOptionViewItem &opt,
                              const QModelIndex &idx, float median, float min_limit, float max_limit, float min_ignore, float max_ignore) const{

    QColor c = color_skill;
    if (auto_contrast)
        c = compliment(bg);

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
        if(auto_contrast &&  c.toHsv().value() == 255){
            neg.setGreen(neg.green() + 76);
            neg.setBlue(0);
            c = QColor::fromHsv(neg.hue(), neg.saturation(), 200);
        }else{
            c = neg;
        }
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
        //also invert the value if it was below the median, and rescale our drawing values from 0-50 and 50-100
        if(adj_rating > 50.0f){
            adj_rating = (adj_rating - 50.0f) * 2.0f;
        }else{
            adj_rating = 100 - (adj_rating * 2.0f);
        }
    }

    p->save();
    switch(m_skill_drawing_method) {
    default:
    case SDM_GROWING_CENTRAL_BOX:                
        if (rating != 0 && (rating >= max_limit || rating <= min_limit)) {
            // draw diamond
            p->setRenderHint(QPainter::Antialiasing);
            p->setPen(Qt::gray);
            p->setBrush(QBrush(c));
            p->translate(opt.rect.x() + 2, opt.rect.y() + 2);
            p->scale(opt.rect.width()-4, opt.rect.height()-4);
            p->drawPolygon(m_diamond_shape);
        } else if (rating > -1 && rating < max_limit) {            
            //0.05625 is the smallest dot we can draw here, so scale to ensure the smallest exp value (1/500 or .002) can always be drawn
            //relative to our maximum limit for this range. this could still be improved to take into account the cell size, as having even
            //smaller cells than the default (16) may not draw very low dabbling skill xp levels
            //float offset = (225 * max_limit - 8 * perc_of_cell) / (4000*perc_of_cell + 225);
            //double size = perc_of_cell * ((roundf(adj_rating)+offset) / (max_limit-offset))
            float perc_of_cell = 0.76f; //max amount of the cell to fill            
            //double size = (((adj_rating-min_limit) * (perc_of_cell - 0.05625)) / (max_limit - min_limit)) + 0.05625;            
            double size = (((adj_rating-min_limit) * (perc_of_cell - 0.05625)) / (max_limit - min_limit)) + 0.05625;
            size = roundf(size * 100) / 100; //this is to aid in the problem of an odd number of pixel in an even size cell, or vice versa
            double inset = (1.0f - size) / 2.0f;
            p->translate(adjusted.x()-inset,adjusted.y()-inset);
            p->scale(adjusted.width()-size,adjusted.height()-size);
            p->fillRect(QRectF(inset, inset, size, size), QBrush(c));
        }
        break;
    case SDM_GROWING_FILL:
        if (rating >= max_limit) {
            // draw diamond
            p->setRenderHint(QPainter::Antialiasing);
            p->setPen(Qt::gray);
            p->setBrush(QBrush(c));
            p->translate(opt.rect.x() + 2, opt.rect.y() + 2);
            p->scale(opt.rect.width() - 4, opt.rect.height() - 4);
            p->drawPolygon(m_diamond_shape);
        } else if (rating > -1 && rating < max_limit) {
            float size = 0.8f * (rating / max_limit) + 0.1f;
            p->translate(adjusted.x(), adjusted.y());
            p->scale(adjusted.width(), adjusted.height());
            p->fillRect(QRectF(0, 0, size, 1), QBrush(c));
        }
        break;
    case SDM_GLYPH_LINES:
    {
        p->setBrush(QBrush(c));
        p->setPen(c);
        p->translate(adjusted.x(), adjusted.y());
        p->scale(adjusted.width(), adjusted.height());
        QVector<QLineF> lines;

        if(rating >= max_limit){
            p->resetTransform();
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
            }
            case 13:
            {
                QPolygonF poly;
                poly << QPointF(0.1, 0.5)
                     << QPointF(0.5, 0.5)
                     << QPointF(0.5, 0.9);
                p->drawPolygon(poly);
            }
            case 12:
            {
                QPolygonF poly;
                poly << QPointF(0.9, 0.5)
                     << QPointF(0.5, 0.5)
                     << QPointF(0.5, 0.9);
                p->drawPolygon(poly);
            }
            case 11:
            {
                QPolygonF poly;
                poly << QPointF(0.1, 0.5)
                     << QPointF(0.5, 0.5)
                     << QPointF(0.5, 0.1);
                p->drawPolygon(poly);
            }
            case 10: // accomplished
                lines << QLineF(QPointF(0.5, 0.1), QPointF(0.9, 0.5));
            case 9: //professional
                lines << QLineF(QPointF(0.1, 0.5), QPointF(0.5, 0.1));
            case 8: //expert
                lines << QLineF(QPointF(0.5, 0.9), QPointF(0.1, 0.5));
            case 7: //adept
                lines << QLineF(QPointF(0.5, 0.9), QPointF(0.9, 0.5));
            case 6: //talented
                lines << QLineF(QPointF(0.5, 0.5), QPointF(0.5, 0.9));
            case 5: //proficient
                lines << QLineF(QPointF(0.5, 0.5), QPointF(0.9, 0.5));
            case 4: //skilled
                lines << QLineF(QPointF(0.5, 0.1), QPointF(0.5, 0.5));
            case 3: //competent
                lines << QLineF(QPointF(0.1, 0.5), QPointF(0.5, 0.5));
            case 2: //untitled
                lines << QLineF(QPointF(0.7, 0.3), QPointF(0.3, 0.7));
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
            p->setPen(c);
            p->drawText(opt.rect, Qt::AlignCenter, text_rating); //QString::number((int)rating));
        }
        break;
    }
    p->restore();
}

void UberDelegate::paint_pref(const QRect &adjusted, QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &proxy_idx) const {
    QModelIndex idx = m_proxy->mapToSource(proxy_idx);
    Dwarf *d = m_model->get_dwarf_by_id(idx.data(DwarfModel::DR_ID).toInt());
    if (!d) {
        return QStyledItemDelegate::paint(p, opt, idx);
    }

    int labor_id = idx.data(DwarfModel::DR_LABOR_ID).toInt();
    short val = d->pref_value(labor_id);
    QString symbol = GameDataReader::ptr()->get_military_preference(labor_id)->value_symbol(val);
    bool dirty = d->is_labor_state_dirty(labor_id);

    QColor bg = paint_bg(adjusted, false, p, opt, proxy_idx);
    p->save();
    if (auto_contrast)
        p->setPen(QPen(compliment(bg)));
    p->drawText(opt.rect, Qt::AlignCenter, symbol);
    p->restore();
    paint_grid(adjusted, dirty, p, opt, proxy_idx);
}

void UberDelegate::paint_flags(const QRect &adjusted, QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &proxy_idx) const {
    QModelIndex idx = m_proxy->mapToSource(proxy_idx);
    Dwarf *d = m_model->get_dwarf_by_id(idx.data(DwarfModel::DR_ID).toInt());
    if (!d) {
        return QStyledItemDelegate::paint(p, opt, idx);
    }

    int bit_pos = idx.data(DwarfModel::DR_LABOR_ID).toInt();
    bool val = d->get_flag_value(bit_pos);
    paint_bg(adjusted, val, p, opt, proxy_idx);
}


void UberDelegate::paint_mood_cell(const QRect &adjusted, QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &proxy_idx, int skill_id, bool dirty) const {
    if(!m_proxy){ //skill legend won't have a proxy
        paint_grid(adjusted, false, p, opt, proxy_idx);
        return;
    }
    QModelIndex idx = m_proxy->mapToSource(proxy_idx);
    Dwarf *d = m_model->get_dwarf_by_id(idx.data(DwarfModel::DR_ID).toInt());
    if (!d) {
        return QStyledItemDelegate::paint(p, opt, idx);
    }

    if(color_mood_cells && !dirty){ //dirty is always drawn over mood
        if(d->highest_moodable().capped_level() > -1 && skill_id == d->highest_moodable().id()){
            p->setPen(QPen(color_mood,2));

            if(d->had_mood())
                p->setPen(QPen(color_had_mood,2));

            QRect moodr = adjusted;
            moodr.adjust(1,1,0,0);

            p->drawLine(moodr.topRight(),moodr.bottomRight());
            p->drawLine(moodr.topLeft(),moodr.bottomLeft());
            p->drawLine(moodr.topRight(),moodr.topLeft());
            p->drawLine(moodr.bottomLeft(),moodr.bottomRight());

            paint_grid(adjusted, dirty, p, opt, proxy_idx, false); //draw dirty border and guides
        }else{
            paint_grid(adjusted, dirty, p, opt, proxy_idx);
        }

    }else{
        paint_grid(adjusted, dirty, p, opt, proxy_idx);
    }
}


void UberDelegate::paint_aggregate(const QRect &adjusted, QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &proxy_idx) const {
    if (!proxy_idx.isValid()) {
        return;
    }
    //QModelIndex model_idx = m_proxy->mapToSource(proxy_idx);
    QModelIndex first_col = m_proxy->index(proxy_idx.row(), 0, proxy_idx.parent());
    if (!first_col.isValid()) {
        return;
    }

    //QString group_name = proxy_idx.data(DwarfModel::DR_GROUP_NAME).toString();
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

    p->save();
    if (enabled_count == m_proxy->rowCount(first_col)) {
        p->fillRect(adjusted, QBrush(color_active_group));
    } else if (enabled_count > 0) {
        p->fillRect(adjusted, QBrush(color_partial_group));
    } else {
        p->fillRect(adjusted, QBrush(color_inactive_group));
    }
    p->restore();

    paint_grid(adjusted, dirty_count > 0, p, opt, proxy_idx);
}

void UberDelegate::paint_grid(const QRect &adjusted, bool dirty, QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &, bool draw_border) const {
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
    //draw guide along top/bottom
    if(opt.state.testFlag(QStyle::State_Selected)){
        if((cell_padding == 0 && !dirty) || cell_padding > 0){ //0 padding replaces the guide with the dirty border
            p->setPen(color_guides);
            p->drawLine(opt.rect.topLeft(), opt.rect.topRight());
            p->drawLine(opt.rect.bottomLeft(), opt.rect.bottomRight());
        }
    }
}

QSize UberDelegate::sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &idx) const {
    if (idx.column() == 0)
        return QStyledItemDelegate::sizeHint(opt, idx);
    return QSize(cell_size, cell_size);
}
