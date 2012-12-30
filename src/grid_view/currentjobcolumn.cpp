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

#include "currentjobcolumn.h"
#include "columntypes.h"
#include "viewcolumnset.h"
#include "dwarfmodel.h"
#include "dwarf.h"
#include "dwarftherapist.h"
#include "defines.h"
#include "dwarfjob.h"
#include "gamedatareader.h"
#include "reaction.h"

CurrentJobColumn::CurrentJobColumn(const QString &title, ViewColumnSet *set,
                       QObject *parent)
    : ViewColumn(title, CT_IDLE, set, parent)
{    
}

CurrentJobColumn::CurrentJobColumn(const CurrentJobColumn &to_copy)
    : ViewColumn(to_copy)
{    
}

QStandardItem *CurrentJobColumn::build_cell(Dwarf *d) {
    QStandardItem *item = init_cell(d);
    short job_id = d->current_job_id();
    QString pixmap_name(":img/help.png");
    if (job_id < 0) {
        if(d->is_on_break()){
            pixmap_name = ":status/img/hourglass.png"; // break
        }else{
            pixmap_name = ":status/img/cross-small.png"; // idle
        }
    } else {
        DwarfJob *job = GameDataReader::ptr()->get_job(job_id);
        if (job) {

            DwarfJob::DWARF_JOB_TYPE job_type = job->type;
            if(!job->reactionClass.isEmpty() && !d->current_sub_job_id().isEmpty()) {
                Reaction* reaction = d->get_reaction();
                if(reaction!=0) {
                    job_type = DwarfJob::get_type(reaction->skill());
                }
            }

            TRACE << "Dwarf: " << d->nice_name() << " job -" << job_id << ": (" << job->description << "," << job_type << ")";

            switch (job_type) {
            case DwarfJob::DJT_IDLE:                
                pixmap_name = ":status/img/cross-small.png";
                break;
            case DwarfJob::DJT_DIG:
                pixmap_name = ":status/img/shovel.png";
                break;
            case DwarfJob::DJT_CUT:
                pixmap_name = ":status/img/tree--minus.png";
                break;
            case DwarfJob::DJT_SLEEP:{
                pixmap_name = ":status/img/moon.png";
                //item->setData(QColor(50,50,50), DwarfModel::DR_DEFAULT_BG_COLOR);
            }
                break;
            case DwarfJob::DJT_DRINK:
                pixmap_name = ":status/img/ale.png";
                break;
            case DwarfJob::DJT_FOOD:
                pixmap_name = ":status/img/cutlery.png";
                break;
            case DwarfJob::DJT_BUILD:
                pixmap_name = ":status/img/hammer--plain.png";
                break;
            case DwarfJob::DJT_HAUL:
                pixmap_name = ":status/img/cart-box.png";
                break;
            case DwarfJob::DJT_FIGHT:
                pixmap_name = ":status/img/crossed-swords.png";
                break;
            case DwarfJob::DJT_MOOD:
                pixmap_name = ":img/exclamation.png";
                break;
            case DwarfJob::DJT_FORGE:
                pixmap_name = ":status/img/status_forge.png";
                break;
            case DwarfJob::DJT_MEDICAL:
                pixmap_name = ":status/img/first_aid_kit.png";
                break;
            case DwarfJob::DJT_WAX_WORKING:
                pixmap_name = ":status/img/lump.png";
                break;
            case DwarfJob::DJT_POTTERY:
                pixmap_name = ":status/img/pot.png";
                break;
            case DwarfJob::DJT_PRESSING:
                pixmap_name = ":status/img/cup2.png";
                break;
            case DwarfJob::DJT_SPINNING:
                pixmap_name = ":status/img/spinning.png";
                break;
            case DwarfJob::DJT_BEE_KEEPING:
                pixmap_name = ":status/img/bee_i_guess.png";
                break;
            case DwarfJob::DJT_STAIRS:
                pixmap_name = ":status/img/stairs.png";
                break;
            case DwarfJob::DJT_FORTIFICATION:
                pixmap_name = ":status/img/wall-brick.png";
                break;
            case DwarfJob::DJT_ENGRAVE:
                pixmap_name = ":status/img/paint-brush.png";
                break;
            case DwarfJob::DJT_LEAF:
                pixmap_name = ":status/img/leaf.png";
                break;
            case DwarfJob::DJT_BUILD_REMOVE:
                pixmap_name = ":status/img/hammer--minus.png";
                break;
            case DwarfJob::DJT_BAG_ADD:
                pixmap_name = ":status/img/paper-bag--plus.png";
                break;
            case DwarfJob::DJT_MONEY:
                pixmap_name = ":status/img/money-coin.png";
                break;
            case DwarfJob::DJT_RETURN:
                pixmap_name = ":status/img/arrow-return.png";
                break;
            case DwarfJob::DJT_PARTY:
                pixmap_name = ":status/img/rubber-balloons.png";
                break;
            case DwarfJob::DJT_SOAP:
                pixmap_name = ":status/img/soap.png";
                break;
            case DwarfJob::DJT_SEEK:
                pixmap_name = ":status/img/eye--arrow.png";
                break;
            case DwarfJob::DJT_GEM_CUT:
                pixmap_name = ":status/img/diamond.png";
                break;
            case DwarfJob::DJT_GEM_ENCRUST:
                pixmap_name = ":status/img/ruby.png";
                break;
            case DwarfJob::DJT_SEEDS:
                pixmap_name = ":status/img/beans.png";
                break;
            case DwarfJob::DJT_LEAF_ARROW:
                pixmap_name = ":status/img/leaf--arrow.png";
                break;
            case DwarfJob::DJT_WATER_ARROW:
                pixmap_name = ":status/img/water--arrow.png";
                break;
            case DwarfJob::DJT_TOMBSTONE:
                pixmap_name = ":status/img/headstone-rip.png";
                break;
            case DwarfJob::DJT_ANIMAL:
                pixmap_name = ":status/img/animal.png";
                break;
            case DwarfJob::DJT_BOOK_OPEN:
                pixmap_name = ":status/img/book-open-list.png";
                break;
            case DwarfJob::DJT_HANDSHAKE:
                pixmap_name = ":status/img/hand-shake.png";
                break;
            case DwarfJob::DJT_CONSTRUCT:
                pixmap_name = ":status/img/hammer-screwdriver.png";
                break;
            case DwarfJob::DJT_ABACUS:
                pixmap_name = ":status/img/abacus.png";
                break;
            case DwarfJob::DJT_FURNACE:
                pixmap_name = ":status/img/fire.png";
                break;
            case DwarfJob::DJT_REPORT:
                pixmap_name = ":status/img/balloon-prohibition.png";
                break;
            case DwarfJob::DJT_JUSTICE:
                pixmap_name = ":status/img/balance.png";
                break;
            case DwarfJob::DJT_SHIELD:
                pixmap_name = ":status/img/shield.png";
                break;
            case DwarfJob::DJT_DEPOT:
                pixmap_name = ":status/img/wooden-box--arrow.png";
                break;
            case DwarfJob::DJT_BROOM:
                pixmap_name = ":status/img/broom.png";
                break;
            case DwarfJob::DJT_SWITCH:
                pixmap_name = ":status/img/switch.png";
                break;
            case DwarfJob::DJT_CHAIN:
                pixmap_name = ":status/img/chain.png";
                break;
            case DwarfJob::DJT_UNCHAIN:
                pixmap_name = ":status/img/chain-unchain.png";
                break;
            case DwarfJob::DJT_FILL_WATER:
                pixmap_name = ":status/img/water--plus.png";
                break;
            case DwarfJob::DJT_MARKET:
                pixmap_name = ":status/img/store-market-stall.png";
                break;
            case DwarfJob::DJT_KNIFE:
                pixmap_name = ":status/img/knife_bloody.png";
                break;
            case DwarfJob::DJT_BOW:
                pixmap_name = ":status/img/bow.png";
                break;
            case DwarfJob::DJT_CHEESE:
                pixmap_name = ":status/img/cheese.png";
                break;
            case DwarfJob::DJT_HELM:
                pixmap_name = ":status/img/helm.png";
                break;
            case DwarfJob::DJT_GLOVE:
                pixmap_name = ":status/img/glove.png";
                break;
            case DwarfJob::DJT_BOOT:
                pixmap_name = ":status/img/boot.png";
                break;
            case DwarfJob::DJT_ARMOR:
                pixmap_name = ":status/img/armor.png";
                break;
            case DwarfJob::DJT_FISH:
                pixmap_name = ":status/img/carp.png";
                break;
            case DwarfJob::DJT_RAW_FISH:
                pixmap_name = ":status/img/fish.png";
                break;
            case DwarfJob::DJT_MILK:
                pixmap_name = ":status/img/milk.png";
                break;
            case DwarfJob::DJT_REST:
                pixmap_name = ":status/img/bandaid--exclamation.png";
                break;
            case DwarfJob::DJT_COOKING:
                pixmap_name = ":status/img/meat.png";
                break;
            case DwarfJob::DJT_BUCKET_POUR:
                pixmap_name = ":status/img/paint-can--arrow.png";
                break;
            case DwarfJob::DJT_GIVE_LOVE:
                pixmap_name = ":status/img/heart--arrow.png";
                break;
            case DwarfJob::DJT_DYE:
                pixmap_name = ":status/img/color--plus.png";
                break;
            case DwarfJob::DJT_WEAPON:
                pixmap_name = ":status/img/weapon.png";
                break;
            case DwarfJob::DJT_SWITCH_CONNECT:
                pixmap_name = ":status/img/switch-network.png";
                break;
            case DwarfJob::DJT_ZONE_ADD:
                pixmap_name = ":status/img/zone--plus.png";
                break;
            case DwarfJob::DJT_CRAFTS:
                pixmap_name = ":status/img/crown.png";
                break;
            case DwarfJob::DJT_GEAR:
                pixmap_name = ":status/img/gear-small.png";
                break;
            case DwarfJob::DJT_TROUBLE:
                pixmap_name = ":status/img/hand-finger.png";
                break;
            case DwarfJob::DJT_STORAGE:
                pixmap_name = ":status/img/box--plus.png";
                break;
            case DwarfJob::DJT_BREW:
                pixmap_name = ":status/img/ale--plus.png";
                break;
            default:
            case DwarfJob::DJT_DEFAULT:
                pixmap_name = ":status/img/control_play_blue.png";
                break;
            }
        }
    }
    item->setData(QIcon(pixmap_name), Qt::DecorationRole);
    
    item->setData(CT_IDLE, DwarfModel::DR_COL_TYPE);
    item->setData(d->current_job_id(), DwarfModel::DR_SORT_VALUE);
    QColor bg = QColor(175,175,175);
    if(DT->user_settings()->value("options/grid/shade_cells",true)==false)
        bg = QColor(255,255,255);
    item->setData(bg,Qt::BackgroundColorRole);

    QString tooltip = QString("<h3>%1</h3>%2 (%3)<h4>%4</h4>")
            .arg(m_title)
            .arg(d->current_job())
            .arg(d->current_job_id())
            .arg(d->nice_name());
    item->setToolTip(tooltip);
    return item;
}

QStandardItem *CurrentJobColumn::build_aggregate(const QString &group_name,
                                           const QVector<Dwarf*> &dwarves) {
    Q_UNUSED(group_name);
    Q_UNUSED(dwarves);
    QStandardItem *item = new QStandardItem;
    item->setData(m_bg_color, DwarfModel::DR_DEFAULT_BG_COLOR);
    return item;
}
