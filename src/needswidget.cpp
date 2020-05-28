/*
Dwarf Therapist
Copyright (c) 2018 Clément Vuchener

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

#include "needswidget.h"
#include "ui_needswidget.h"

#include <QPainter>

#include "dfinstance.h"
#include "dwarf.h"
#include "gamedatareader.h"
#include "histfigure.h"
#include "standardpaths.h"
#include "unitneed.h"

enum NeedsWidgetRole
{
    ListRole = Qt::UserRole+1,
    SortRole,
    NeedRole,
    DeityRole,
    DegreeRole,
};

NeedsWidget::NeedsWidget(QWidget *parent)
    : QWidget(parent)
    , ui(std::make_unique<Ui::NeedsWidget>())
{
    ui->setupUi(this);

    // Focus setup
    m_focus_model.setHorizontalHeaderLabels({tr("Focus"), tr("Count")});

    QList<QColor> focus_colors;
    for (int i = 0; i < Dwarf::FOCUS_DEGREE_COUNT; ++i)
        focus_colors.append(Dwarf::get_focus_color(i, false, true));
    m_focus_delegate = std::make_unique<NeedsDelegate>(focus_colors);

    ui->focus_view->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->focus_view->setModel(&m_focus_model);
    ui->focus_view->setItemDelegate(m_focus_delegate.get());

    connect(ui->focus_view->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
            this, SLOT(focus_selection_changed()));

    // Needs setup
    m_needs_model.setHorizontalHeaderLabels({tr("Need"), tr("Count")});

    QList<QColor> needs_colors;
    for (int i = 0; i < UnitNeed::DEGREE_COUNT; ++i)
        needs_colors.append(UnitNeed::degree_color(i, false, true));
    m_needs_delegate = std::make_unique<NeedsDelegate>(needs_colors);

    ui->need_view->view()->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->need_view->set_filter_mode(SortFilterProxyModel::TopLevelMode);
    ui->need_view->set_model(&m_needs_model);
    ui->need_view->view()->setItemDelegate(m_needs_delegate.get());
    ui->need_view->filter_proxy().setSortRole(SortRole);

    auto settings = StandardPaths::settings();
    settings->beginGroup("needs_widget");
    ui->focus_view->header()->restoreState(settings->value("focus_header").toByteArray());
    ui->need_view->view()->header()->restoreState(settings->value("need_header").toByteArray());
    settings->endGroup();

    connect(ui->need_view, SIGNAL(item_selection_changed(const QItemSelection &, const QItemSelection &)),
            this, SLOT(need_selection_changed()));
    connect(ui->match_all_checkbox, SIGNAL(stateChanged(int)),
            this, SLOT(need_selection_changed()));

    if (DT) {
        connect(DT, SIGNAL(units_refreshed()), this, SLOT(refresh()));
    }
}

NeedsWidget::~NeedsWidget()
{
}

void NeedsWidget::save_state(QSettings &settings) const
{
    settings.beginGroup("needs_widget");
    settings.setValue("focus_header", ui->focus_view->header()->saveState());
    settings.setValue("need_header", ui->need_view->view()->header()->saveState());
    settings.endGroup();
}

void NeedsWidget::clear()
{
    m_focus_model.removeRows(0, m_focus_model.rowCount());
    m_needs_model.removeRows(0, m_needs_model.rowCount());
}

void NeedsWidget::refresh()
{
    clear();
    if (!DT)
        return;
    auto df = DT->get_DFInstance();
    if (!df)
        return;

    auto gdr = GameDataReader::ptr();
    const auto &data = df->get_needs_data();
    // All items except top level "overall focus" use these flags.
    auto flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;

    // Overall focus model
    auto overall_focus_name = new QStandardItem;
    overall_focus_name->setText(tr("Overall focus"));
    overall_focus_name->setToolTip(tr("Overall focus"));
    overall_focus_name->setFlags(Qt::ItemIsEnabled);

    QStringList focus_counts;
    QVariantList focus_values;
    int total_dwarves = 0;
    for (int i = 0; i < Dwarf::FOCUS_DEGREE_COUNT; ++i) {
        auto size = data.overall_focus.dwarves[i].size();
        auto color = Dwarf::get_focus_color(i);

        auto focus_degree_name = new QStandardItem;
        focus_degree_name->setText(Dwarf::get_focus_adjective(i));
        focus_degree_name->setToolTip(Dwarf::get_focus_adjective(i));
        focus_degree_name->setData(color, Qt::TextColorRole);
        focus_degree_name->setData(i, DegreeRole);
        focus_degree_name->setFlags(flags);

        auto focus_degree_count = new QStandardItem;
        focus_degree_count->setText(QString::number(size));
        focus_degree_count->setData(color, Qt::TextColorRole);
        focus_degree_count->setFlags(flags);

        overall_focus_name->appendRow({focus_degree_name, focus_degree_count});

        total_dwarves += size;
        focus_counts.append(QString("<font color=\"%1\">%2</font>")
                .arg(Dwarf::get_focus_color(i).name())
                .arg(size));
        focus_values.append((int)size);
    }

    auto overall_focus_count = new QStandardItem;
    overall_focus_count->setText(QString::number(total_dwarves));
    overall_focus_count->setData(focus_values, ListRole);
    overall_focus_count->setToolTip(focus_counts.join("/"));
    overall_focus_count->setFlags(Qt::ItemIsEnabled);

    m_focus_model.appendRow({overall_focus_name, overall_focus_count});

    // Need model
    for (const auto &p: data.needs) {
        int need_id = std::get<0>(p.first);
        int deity_id = std::get<1>(p.first);

        auto need_name = new QStandardItem;
        QString name = gdr->get_need_name(need_id);
        QString deity_name;
        if (deity_id != -1) {
            deity_name = HistFigure::get_name(df, deity_id, true);
            name.append(tr(" to %1").arg(deity_name));
        }
        need_name->setText(name);
        need_name->setToolTip(name);
        need_name->setData(name, SortRole);
        need_name->setData(need_id, NeedRole);
        need_name->setData(deity_id, DeityRole);
        need_name->setFlags(flags);

        int total_count = 0;
        QStringList degree_counts;
        QVariantList degree_values;
        for (int i = 0; i < UnitNeed::DEGREE_COUNT; ++i) {
            const auto &dwarves = p.second.dwarves[i];
            QColor color = UnitNeed::degree_color(i);

            QStringList dwarve_names;
            for (auto d: dwarves)
                dwarve_names.append(d->nice_name());

            auto desc = tr("<h4><font color=\"%1\">%2 dwarves are %3 after %4:</font></h4> <p>%5</p>", "", dwarves.size())
                    .arg(color.name())
                    .arg(dwarves.size())
                    .arg(UnitNeed::degree_adjective(i))

                    .arg(gdr->get_need_desc(need_id, i <= UnitNeed::NOT_DISTRACTED, deity_name))
                    .arg(dwarve_names.join(", "));

            auto need_degree_name = new QStandardItem;
            need_degree_name->setText(UnitNeed::degree_adjective(i));
            need_degree_name->setToolTip(desc);
            need_degree_name->setData(i, SortRole);
            need_degree_name->setData(color, Qt::TextColorRole);
            need_degree_name->setData(need_id, NeedRole);
            need_degree_name->setData(deity_id, DeityRole);
            need_degree_name->setData(i, DegreeRole);
            need_degree_name->setFlags(flags);

            auto need_degree_count = new QStandardItem;
            need_degree_count->setText(QString::number(dwarves.size()));
            need_degree_count->setToolTip(desc);
            need_degree_count->setData((int)dwarves.size(), SortRole);
            need_degree_count->setData(color, Qt::TextColorRole);
            need_degree_count->setFlags(flags);

            need_name->appendRow({need_degree_name, need_degree_count});

            total_count += dwarves.size();
            degree_counts.append(QString("<font color=\"%1\">%2</font>")
                    .arg(UnitNeed::degree_color(i, true).name())
                    .arg(dwarves.size()));
            degree_values.append((int)dwarves.size());
        }

        auto need_count = new QStandardItem;
        need_count->setText(QString::number(total_count));
        need_count->setData(total_count, SortRole);
        need_count->setData(degree_values, ListRole);
        need_count->setToolTip(degree_counts.join("/"));
        need_count->setFlags(flags);

        m_needs_model.appendRow({need_name, need_count});
    }
}

void NeedsWidget::focus_selection_changed()
{
    QVariantList list;
    for (const auto &index: ui->focus_view->selectionModel()->selection().indexes()) {
        auto degree = index.data(DegreeRole);
        if (degree.isValid())
            list.append(degree);
    }
    emit focus_selected(list);
}

void NeedsWidget::need_selection_changed()
{
    QVariantList list;
    for (const auto &index: ui->need_view->get_selection().indexes()) {
        QVariantList item_list;
        auto need = index.data(NeedRole);
        if (!need.isValid())
            continue;
        auto deity = index.data(DeityRole);
        auto degree = index.data(DegreeRole);
        item_list.append(need);
        item_list.append(deity);
        if (degree.isValid())
            item_list.append(degree);
        list.append(QVariant(item_list));
    }
    emit need_selected(list, ui->match_all_checkbox->checkState() == Qt::Checked);
}

NeedsDelegate::NeedsDelegate(const QList<QColor> &colors, QObject *parent)
    : QStyledItemDelegate(parent)
    , m_colors(colors)
{
}

NeedsDelegate::~NeedsDelegate()
{
}

void NeedsDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter, option, index);

    auto data = index.data(ListRole);
    if (!data.isValid())
        return;

    painter->save();

    auto rect = option.rect.marginsRemoved(QMargins(2, 2, 2, 2));
    painter->setPen(Qt::NoPen);
    painter->setBrush(option.palette.color(QPalette::Base));
    painter->drawRect(rect);

    auto values = data.toList();
    int bar_count = m_colors.count();
    int total = 0;
    for (int i = 0; i < bar_count; ++i)
        total += values.value(i, 0).toInt();
    if (total != 0) {
        int left = 0;
        int sum = 0;
        for (int i = 0; i < bar_count; ++i) {
            int count = values.value(i, 0).toInt();
            if (count == 0)
                continue;
            sum += count;
            int right = (rect.width() * sum + total/2) / total;
            if (right-left > 0) {
                auto color = m_colors[i];
                color.setAlphaF(0.5);
                painter->setBrush(color);
                painter->drawRect(rect.left() + left, rect.top(),
                                  right - left, rect.height());
            }
            left = right;
        }
    }

    painter->setPen(option.palette.color(QPalette::Text));
    painter->drawText(option.rect, Qt::AlignCenter, QString("%1").arg(total));

    painter->restore();
}

