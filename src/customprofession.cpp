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
#include <QMessageBox>
#include <QPainter>
#include "customprofession.h"
#include "gamedatareader.h"
#include "ui_customprofession.h"
#include "dwarf.h"
#include "defines.h"
#include "profession.h"
#include "dwarftherapist.h"
#include "iconchooser.h"
#include "utils.h"
#include "defaultfonts.h"
#include "multilabor.h"
#include "superlabor.h"

/*!
Default ctor. Creates a blank skill template with no name
*/
CustomProfession::CustomProfession(QObject *parent)
    : MultiLabor(parent)
    , ui(new Ui::CustomProfessionEditor)
    , m_icon_id(-1)
    , m_is_mask(false)
    , m_font_custom_color(0x0)
    , m_bg_custom_color(0x0)
    , m_font_color(Qt::black)
    , m_bg_color(Qt::transparent)
    , m_txt("")
    , m_prof_id(-1)
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
    : MultiLabor(parent)
    , ui(new Ui::CustomProfessionEditor)
    , m_icon_id(-1)
    , m_is_mask(false)
    , m_font_custom_color(0x0)
    , m_bg_custom_color(0x0)
    , m_font_color(Qt::black)
    , m_bg_color(Qt::transparent)
    , m_txt("")
    , m_prof_id(-1)
    , m_fnt(0x0)
{
    m_dwarf = d;
    if(m_dwarf){
        m_name = d->profession();
        set_labors();
    }
}

//profession icon
CustomProfession::CustomProfession(int profession_id, QObject *parent)
    : MultiLabor(parent)
    , ui(new Ui::CustomProfessionEditor)
    , m_icon_id(-1)
    , m_is_mask(false)
    , m_font_custom_color(0x0)
    , m_bg_custom_color(0x0)
    , m_font_color(Qt::black)
    , m_bg_color(Qt::transparent)
    , m_txt("")
    , m_prof_id(profession_id)
    , m_fnt(0x0)
{
    set_name(gdr->get_profession(m_prof_id)->name());
}

//from the ini file. currently is in a different format and is read slightly different
CustomProfession::CustomProfession(QString name, QSettings &s, QObject *parent)
    :MultiLabor(parent)
    , ui(new Ui::CustomProfessionEditor)
    , m_font_custom_color(0x0)
    , m_bg_custom_color(0x0)
    , m_fnt(0x0)
{
    set_name(name);
    s.beginGroup(name);
    init(s);
    s.endGroup();
}

//import from file
CustomProfession::CustomProfession(QSettings &s, QObject *parent)
    :MultiLabor(parent)
    , ui(new Ui::CustomProfessionEditor)
    , m_font_custom_color(0x0)
    , m_bg_custom_color(0x0)
    , m_fnt(0x0)
{
    set_name(s.value("name", "UNKNOWN").toString());
    init(s);
}

CustomProfession::~CustomProfession() {
    delete ui;
}

void CustomProfession::init(QSettings &s){
    build_icon_path(s.value("icon_id",-1).toInt());
    m_font_color = s.value("text_color", QColor(Qt::black)).value<QColor>();
    m_bg_color = s.value("bg_color", QColor(Qt::transparent)).value<QColor>();
    m_txt = s.value("text", "").toString();
    m_role_name = s.value("role_name","").toString();
    m_is_mask = s.value("is_mask").toBool();
    m_prof_id = s.value("prof_id",-1).toInt();

    int labors = s.beginReadArray("labors");
    for(int j = 0; j < labors; ++j) {
        s.setArrayIndex(j);
        add_labor(s.childKeys()[0].toInt());
    }
    s.endArray();
    create_image();
}

/*!
Pops up a dialog box asking for a name for this object as well
as a list of labors that can be enabled/disabled via checkboxes

\param[in] parent If set, the dialog will launch as a model under parent
\returns QDialog::exec() result (int)
*/
int CustomProfession::show_builder_dialog(QWidget *parent) {
    m_dialog = new QDialog(parent);

    ui->setupUi(m_dialog);

    ui->name_edit->setText(m_name);
    connect(ui->name_edit, SIGNAL(textChanged(const QString &)), this, SLOT(set_name(QString)));
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(this,SIGNAL(selected_count_changed(int)),ui->lbl_skill_count,SLOT(setNum(int)));

    build_role_combo(ui->cb_roles);
    connect(ui->cb_roles,SIGNAL(currentIndexChanged(int)),this,SLOT(role_changed(int)),Qt::UniqueConnection);

    //add font color chooser
    m_font_custom_color = new CustomColor("",tr("The color of the text drawn over the icon."),"text_color", Qt::black, 0);
    m_font_custom_color->set_color(m_font_color);
    m_font_custom_color->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
    ui->hLayoutText->insertWidget(3,m_font_custom_color);
    connect(m_font_custom_color, SIGNAL(color_changed(QString,QColor)), this, SLOT(color_selected(QString,QColor)));

    //setup the mask
    ui->chk_mask->setChecked(m_is_mask);
    ui->chk_mask->setToolTip("This profession's labors will be applied in addition to any labors already enabled.");
    connect(ui->chk_mask,SIGNAL(clicked(bool)),this,SLOT(mask_changed(bool)));

    //add background color chooser
    m_bg_custom_color = new CustomColor("",tr("The background color of the icon."), "bg_color", Qt::transparent, 0);
    m_bg_custom_color->set_color(m_bg_color);
    connect(m_bg_custom_color, SIGNAL(color_changed(QString,QColor)), this, SLOT(color_selected(QString,QColor)));
    m_bg_custom_color->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
    ui->hlayout_bg_color->insertWidget(1,m_bg_custom_color);

    //no id number means it's a custom profession which can have labors, otherwise it's simply an icon/text override
    if(m_prof_id < 0){
        load_labors(ui->labor_list);
    }else{
        //clear the mask stuff
        ui->chk_mask->hide();
        ui->lbl_mask->hide();
        //hide the list
        ui->labor_list->adjustSize();
        ui->labor_list->hide();
        ui->lbl_selection_count->hide();
        ui->lbl_skill_count->hide();
        //hide the roles
        ui->lbl_roles->hide();
        ui->cb_roles->hide();
        //always show the prof name but lock it
        ui->name_edit->setText(GameDataReader::ptr()->get_profession(m_prof_id)->name(true));
        ui->name_edit->setEnabled(false);
        //disable resizing
        m_dialog->adjustSize();
        m_dialog->setSizeGripEnabled(false);
        m_dialog->layout()->setSizeConstraint(QLayout::SetFixedSize);
        //change the window name
        m_dialog->setWindowTitle(tr("Custom Icon"));
    }

    refresh_icon();
    connect(ui->btnIcon,SIGNAL(clicked()),this,SLOT(choose_icon()));

    ui->le_prefix->setText(m_txt);
    connect(ui->le_prefix, SIGNAL(textChanged(QString)), this, SLOT(prefix_changed(QString)));

    int code = m_dialog->exec();
    m_dialog->deleteLater();
    return code;
}

void CustomProfession::update_dwarf(){
    if(m_dwarf && m_prof_id < 0) //don't try to apply icons
        m_dwarf->apply_custom_profession(this);
}

/*!
Called after the show_builder_dialog widget is accepted, used to verify that
the CustomProfession has all needed information to save

\returns true if this instance is ok to save.
*/
bool CustomProfession::is_valid() {
    if (!m_dialog)
        return true;

    //dont worry about a naming conflict it's an icon
    if(m_prof_id < 0){
        QString proposed_name = ui->name_edit->text().trimmed();
        if (proposed_name.isEmpty()) {
            QMessageBox::warning(m_dialog, tr("Naming Error!"),
                                 tr("You must enter a name for this Custom Profession!"));
            return false;
        }
        foreach(CustomProfession *cp, DT->get_custom_professions()){
            if(cp != this && cp->get_name() == proposed_name){
                QMessageBox::warning(m_dialog, tr("Duplicate Name!"),
                                     tr("A Custom Profession with this name already exists!"));
                return false;
            }
        }
        foreach(SuperLabor *sl, DT->get_super_labors()){
            if(sl->get_name() == proposed_name){
                QMessageBox::warning(m_dialog, tr("Duplicate Name!"),
                                     tr("A Super Labor with this name already exists!"));
                return false;
            }
        }
    }else{
        //inform the user that the new icons won't be shown until the next read
        QMessageBox::information(m_dialog,tr("Read Required"),tr("The new profession icon has been saved and will be shown after the next read."));
    }
    return true;
}

void CustomProfession::role_changed(int index){
    m_role_name = ui->cb_roles->itemData(index,Qt::UserRole).toString();
}

void CustomProfession::prefix_changed(QString val){
    m_txt = val;
    refresh_icon();
}

void CustomProfession::mask_changed(bool value){
    m_is_mask = value;
}

void CustomProfession::build_icon_path(int id){
    m_icon_id = id;
    QString icn_path = QString(":/profession/prof_%1.png").arg(QString::number(m_icon_id));
    if(QFile::exists(icn_path)){
        m_icon_path = icn_path;
    }else{
        m_icon_path = "";
    }
}

void CustomProfession::choose_icon(){
    IconChooser *ic = new IconChooser();
    ic->exec();
    build_icon_path(ic->selected_id());
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
    if(has_icon())
        m_dialog->setWindowIcon(QIcon(m_pixmap));
    else
        m_dialog->setWindowIcon(QIcon(":img/hammer.png"));
    if(m_icon_path == "" || m_icon_id <= -1)
        ui->lbl_icon->setText(tr("None"));
    else{
        ui->lbl_icon->setText(tr(""));
        ui->lbl_icon->setPixmap(QPixmap(m_icon_path));
    }
}

bool CustomProfession::has_icon(){
    return (m_icon_id > -1 || m_txt != "" || !m_pixmap.isNull());
}

void CustomProfession::create_image(){
    if(m_icon_id > -1 && m_txt == ""){
        m_pixmap = QPixmap(m_icon_path); //default profession icon
    }else if(m_bg_color != QColor(Qt::transparent) || !m_txt.isEmpty() || !m_icon_path.isEmpty()){
        QPainter p;
        m_pixmap = QPixmap(16,16);

        //fill the background
        m_pixmap.fill(m_bg_color);

        p.begin(&m_pixmap);

        //draw the icon if we have one
        if(m_icon_id > -1 && !m_icon_path.isEmpty()){
            p.drawPixmap(0,0,QPixmap(m_icon_path));
        }

        //draw the text
        if(!m_txt.isEmpty()){
            p.setPen(QPen(m_font_color));
            QRect rtxt = m_pixmap.rect();
            rtxt.adjust(0,1,0,0);
            p.setFont(*get_font());
            p.drawText(rtxt, Qt::AlignCenter, m_txt);
        }
        p.end();
    }else{
        m_pixmap = QPixmap();
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
        m_fnt = new QFont(DT->user_settings()->value("options/grid/font", QFont(DefaultFonts::getRowFontName(), DefaultFonts::getRowFontSize())).value<QFont>());
        m_fnt->setBold(true);
        m_fnt->setPointSize(8); //icons are only 16x16, this is about the largest font we can use and still get text inside
    }
    return m_fnt;
}

void CustomProfession::set_name(QString name){
    if(name.contains("::")){
        //an icon override will be stored as name::id
        QStringList names = name.split("::",QString::SkipEmptyParts);
        m_name = names.at(0);
    }else{
        m_name = name;
    }
}

QString CustomProfession::get_save_name(){
    if(m_prof_id > -1)
        return m_name + "::" + QString::number(m_prof_id);
    else
        return m_name;
}


void CustomProfession::delete_from_disk() {
    QSettings s(QSettings::IniFormat, QSettings::UserScope, COMPANY, PRODUCT, this);
    s.beginGroup("custom_professions");
    s.remove(get_save_name());
    s.endGroup();
}

void CustomProfession::save(QSettings &s){
    s.beginGroup(get_save_name());
    s.setValue("icon_id",m_icon_id);
    s.setValue("text", m_txt);
    s.setValue("text_color", m_font_color);
    s.setValue("bg_color", m_bg_color);

    //save non-icon override custom profession stuff
    if(m_prof_id < 0){
        s.setValue("role_name", m_role_name);
        s.setValue("is_mask",m_is_mask);
        s.beginWriteArray("labors");
        int i = 0;
        foreach(int labor_id, get_enabled_labors()) {
            s.setArrayIndex(i++);
            s.setValue(QString::number(labor_id), true);
        }
        s.endArray();
    }else{
        s.setValue("prof_id", m_prof_id);
    }
    s.endGroup();
}

void CustomProfession::export_to_file(QSettings &s){
    //exports are done slightly differently, in an array format with the name as a value
    s.setValue("name", get_save_name());
    s.setValue("icon_id", m_icon_id);
    s.setValue("text", m_txt);
    s.setValue("text_color", m_font_color);
    s.setValue("bg_color", m_bg_color);
    if(m_prof_id < 0){
        s.setValue("is_mask",is_mask());
        s.setValue("role_name",m_role_name);
        s.beginWriteArray("labors");
        int idx = 0;
        foreach(int labor_id, get_enabled_labors()) {
            s.setArrayIndex(idx++);
            s.setValue(QString::number(labor_id), true);
        }
        s.endArray();
    }else{
        s.setValue("prof_id", m_prof_id);
    }
}
