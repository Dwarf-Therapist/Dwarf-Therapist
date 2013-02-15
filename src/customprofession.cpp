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
#include "customprofession.h"
#include "gamedatareader.h"
#include "ui_customprofession.h"
#include "dwarf.h"
#include "defines.h"
#include "labor.h"
#include "profession.h"
#include "dwarftherapist.h"
#include "iconchooser.h"
#include "utils.h"

/*!
Default ctor. Creates a blank skill template with no name
*/
CustomProfession::CustomProfession(QObject *parent)
    : QObject(parent)
    , ui(new Ui::CustomProfessionEditor)
    , m_dwarf(0)
    , m_dialog(0)
    , m_is_mask(false)
    , m_bg_custom_color(0x0)
    , m_font_color(Qt::black)
    , m_bg_color(Qt::transparent)
    , m_txt("")
    , m_id(-1)
    , m_fnt(0x0)
{
}

/*!
When passed in a pointer to a Dwarf, this new custom profession
will adopt whatever labors that Dwarf has enabled as its
own template.

This is used by the "Create custom profession from this dwarf..." action.

\param[in] d The Dwarf to use as a labor template
\param[in] parent The Qt owner of this object
*/
CustomProfession::CustomProfession(Dwarf *d, QObject *parent)
    : QObject(parent)
    , ui(new Ui::CustomProfessionEditor)
    , m_dwarf(d)
    , m_dialog(0)
    , m_is_mask(false)
    , m_bg_custom_color(0x0)
    , m_font_color(Qt::black)
    , m_bg_color(Qt::transparent)
    , m_txt("")
    , m_id(-1)
    , m_fnt(0x0)
{
    GameDataReader *gdr = GameDataReader::ptr();
    QList<Labor*> labors = gdr->get_ordered_labors();

    foreach(Labor *l, labors) {
        if (m_dwarf && m_dwarf->labor_enabled(l->labor_id))
            add_labor(l->labor_id);
    }    
}

/*!
Change the enabled status of a template labor. This doesn't
affect any dwarves using this custom profession, only the
template itself.

\param[in] labor_id The id of the labor to change
\param[in] active Should the labor be enabled or not
*/
void CustomProfession::set_labor(int labor_id, bool active) {
    if (m_active_labors.contains(labor_id) && !active)
        m_active_labors.remove(labor_id);
    if (active)
        m_active_labors.insert(labor_id, true);
}

/*!
Check if the template has a labor enabled

\param[in] labor_id The id of the labor to check
\returns true if this labor is enabled
*/
bool CustomProfession::is_active(int labor_id) {
    return m_active_labors.value(labor_id, false);
}

/*!
Get a vector of all enabled labors in this template by labor_id
*/
QVector<int> CustomProfession::get_enabled_labors() {
    QVector<int> labors;
    foreach(int labor, m_active_labors.uniqueKeys()) {
        if (m_active_labors.value(labor)) {
            labors << labor;
        }
    }
    return labors;
}

/*!
Pops up a dialog box asking for a name for this object as well
as a list of labors that can be enabled/disabled via checkboxes

\param[in] parent If set, the dialog will launch as a model under parent
\returns QDialog::exec() result (int)
*/
int CustomProfession::show_builder_dialog(QWidget *parent) {
    GameDataReader *gdr = GameDataReader::ptr();

    m_dialog = new QDialog(parent);    

    ui->setupUi(m_dialog);

    ui->name_edit->setText(m_name);
    connect(ui->name_edit, SIGNAL(textChanged(const QString &)), this, SLOT(set_name(QString)));
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(accept()));

    //add font color chooser
    m_font_custom_color = new CustomColor("",tr("The color of the text drawn over the icon."),
                                 "text_color", Qt::black, 0);
    m_font_custom_color->set_color(m_font_color);
    ui->hLayoutText->insertWidget(3,m_font_custom_color);
    connect(m_font_custom_color, SIGNAL(color_changed(QString,QColor)), this, SLOT(color_selected(QString,QColor)));

    ui->chk_mask->setChecked(m_is_mask);
    ui->chk_mask->setToolTip("This profession's labours will be applied in addition to any labors already enabled.");
    connect(ui->chk_mask,SIGNAL(clicked(bool)),this,SLOT(mask_changed(bool)));

    //add background color chooser
    m_bg_custom_color = new CustomColor("",tr("The background color of the icon."),
                                        "bg_color", Qt::transparent, 0);
    m_bg_custom_color->set_color(m_bg_color);
    connect(m_bg_custom_color, SIGNAL(color_changed(QString,QColor)), this, SLOT(color_selected(QString,QColor)));
    ui->hlayout_bg_color->insertWidget(1,m_bg_custom_color);

    //no id number means it's a custom profession which can have labors, otherwise it's simply an icon/text override
    if(m_id < 0){
        QList<Labor*> labors = gdr->get_ordered_labors();
        int num_active = 0;
        foreach(Labor *l, labors) {
            QListWidgetItem *item = new QListWidgetItem(l->name, ui->labor_list);
            item->setData(Qt::UserRole, l->labor_id);
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            if (is_active(l->labor_id)) {
                item->setCheckState(Qt::Checked);
                num_active++;
            } else {
                item->setCheckState(Qt::Unchecked);
            }
            ui->labor_list->addItem(item);
        }

        connect(ui->labor_list,
                SIGNAL(itemChanged(QListWidgetItem*)),
                this,
                SLOT(item_check_state_changed(QListWidgetItem*)));

        ui->lbl_skill_count->setNum(num_active);
    }else{
        //clear the mask stuff
        ui->chk_mask->setVisible(false);
        ui->lbl_selection_count->setVisible(false);
        ui->lbl_skill_count->setVisible(false);
        //hide the list
        ui->labor_list->adjustSize();
        ui->labor_list->setVisible(false);
        //always show the prof name and lock it
        ui->name_edit->setText(GameDataReader::ptr()->get_profession(m_id)->name(true));
        ui->name_edit->setEnabled(false);
        //disable resizing
        m_dialog->adjustSize();
        m_dialog->setSizeGripEnabled(false);
        m_dialog->layout()->setSizeConstraint(QLayout::SetFixedSize);
    }

    refresh_icon();
    connect(ui->btnIcon,SIGNAL(clicked()),this,SLOT(choose_icon()));

    ui->le_prefix->setText(m_txt);
    connect(ui->le_prefix, SIGNAL(textChanged(QString)), this, SLOT(prefix_changed(QString)));

    int code = m_dialog->exec();
    m_dialog->deleteLater();    
    return code;
}

/*!
Called when the show_builder_dialog widget's OK button is pressed, or the
dialog is otherwise accepted by the user

We intercept this call to verify the form is valid before saving it.
\sa is_valid()
*/
void CustomProfession::accept() {
    if (!is_valid()) {
        return;
    }
    m_dialog->accept();
}

/*!
Called after the show_builder_dialog widget is accepted, used to verify that
the CustomProfession has all needed information to save

\returns true if this instance is ok to save.
*/
bool CustomProfession::is_valid() {
    if (!m_dialog)
        return true;

    QString proposed_name = ui->name_edit->text();
    if (proposed_name.isEmpty()) {
        QMessageBox::warning(m_dialog, tr("Naming Error!"),
            tr("You must enter a name for this custom profession!"));
        return false;
    }
    /* Let's not do this...
    QHash<short, Profession*> profs = GameDataReader::ptr()->get_professions();
    foreach(Profession *p, profs) {
        if (proposed_name == p->name(true)) {
            QMessageBox::warning(m_dialog, tr("Naming Error!"),
                tr("The profession '%1' is a default game profession, please choose a different name.").arg(proposed_name));
            return false;
        }
    }
    */
    return true;
}

void CustomProfession::item_check_state_changed(QListWidgetItem *item) {
    if (item->checkState() == Qt::Checked) {
        add_labor(item->data(Qt::UserRole).toInt());
        ui->lbl_skill_count->setNum(ui->lbl_skill_count->text().toInt() + 1);
    } else {
        remove_labor(item->data(Qt::UserRole).toInt());
        ui->lbl_skill_count->setNum(ui->lbl_skill_count->text().toInt() - 1);
    }
}

void CustomProfession::prefix_changed(QString val){
    m_txt = val;
    refresh_icon();
}

void CustomProfession::mask_changed(bool value){
    m_is_mask = value;
}

void CustomProfession::delete_from_disk() {
    QSettings s(QSettings::IniFormat, QSettings::UserScope, COMPANY, PRODUCT, this);
    s.beginGroup("custom_professions");
    QString name = m_name;
    if(m_id > -1)
        m_name += "::" + QString::number(m_id);
    s.remove(name);
    s.endGroup();
}

void CustomProfession::choose_icon(){
    IconChooser *ic = new IconChooser();
    ic->exec();
    m_icon_id = ic->selected_id;
    m_path = ":/profession/img/profession icons/prof_" + QString::number(m_icon_id) + ".png";
    refresh_icon();
}

void CustomProfession::color_selected(QString key, QColor col){
    if(key == "bg_color"){
        m_bg_color = col;
    }else{
        m_font_color = col;
    }
    refresh_icon();
}

void CustomProfession::refresh_icon(){
    create_image();
    m_dialog->setWindowIcon(QIcon(m_pixmap));
    if(m_path == "" || m_icon_id == -1)
        ui->lbl_icon->setText(tr("No Icon."));
    else{
        ui->lbl_icon->setText(tr(""));
        ui->lbl_icon->setPixmap(QPixmap(m_path));
    }
}

void CustomProfession::create_image(){
    if(m_icon_id > -1 && m_txt == ""){
        m_pixmap = QPixmap(m_path); //default profession icon
    }else{
        QPainter p;
        QPixmap icn(m_path);
        m_pixmap = QPixmap(16,16);

        //fill the background
        m_pixmap.fill(m_bg_color);

        p.begin(&m_pixmap);
        //draw the icon if we have one
        if(m_icon_id > -1)
            p.drawPixmap(0,0,icn);
        //draw the text

        p.setPen(QPen(m_font_color));
        QRect rtxt = m_pixmap.rect();
        rtxt.adjust(0,1,0,0);
        p.setFont(*get_font());
        p.drawText(rtxt, Qt::AlignCenter, m_txt);
        p.end();
    }
}

QPixmap CustomProfession::get_pixmap(){
    if(!m_pixmap)
        create_image();
    return m_pixmap;
}

QString CustomProfession::get_embedded_pixmap(){
    if(!m_pixmap)
        create_image();
    return embedPixmap(m_pixmap);
}

QFont* CustomProfession::get_font(){
    if(!m_fnt){
        m_fnt = new QFont(DT->user_settings()->value("options/grid/font", QFont("Segoe UI", 8)).value<QFont>());
        m_fnt->setBold(true);
        m_fnt->setPointSize(8); //icons are only 16x16, this is about the largest font we can use and still get text inside
    }
    return m_fnt;
}

void CustomProfession::set_name(QString name){
    //an icon override will be stored as name::id
    QStringList names = name.split("::",QString::SkipEmptyParts);
    m_name = names.at(0);
}

QString CustomProfession::get_save_name(){
    if(m_id > -1)
        return m_name + "::" + QString::number(m_id);
    else
        return m_name;
}
