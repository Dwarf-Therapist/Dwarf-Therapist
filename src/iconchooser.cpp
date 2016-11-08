#include "gamedatareader.h"
#include "iconchooser.h"

#include <QDialogButtonBox>
#include <QFile>
#include <QLabel>
#include <QListView>
#include <QStandardItemModel>
#include <QVBoxLayout>

const int imageSize = 16;

QImage scale(const QString &imageFileName)
{
    QImage image(imageFileName);
    return image.scaled(QSize(imageSize, imageSize), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

IconChooser::IconChooser(QWidget *parent)
    : QDialog(parent)
    , m_selected_id(-1)
{
    setWindowIcon(QIcon(":img/hammer.png"));
    setWindowTitle(tr("Choose New Icon"));

    QVBoxLayout* vLayout = new QVBoxLayout(this);
    m_imageListView = new QListView(this);
    vLayout->addWidget(m_imageListView);

    m_imageListView->setViewMode( QListView::IconMode );
    m_imageListView->setUniformItemSizes(true);
    m_imageListView->setSelectionRectVisible(true);
    m_imageListView->setMovement(QListView::Static);
    m_imageListView->setSelectionMode(QListView::SingleSelection);
    m_imageListView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    m_lbl_info = new QLabel(tr("Choose a new icon and click OK"),this);
    vLayout->addWidget(m_lbl_info,0,Qt::AlignCenter);

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    vLayout->addWidget(buttons);

    connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

    m_imageListView->setResizeMode(QListView::Adjust);

    m_standardModel = new QStandardItemModel(this);
    m_imageListView->setModel(m_standardModel);

    for(int i = 1; i <= GameDataReader::ptr()->get_professions().count(); i++){
        showImage(i);
    }

    connect(m_imageListView, SIGNAL(clicked(QModelIndex)), SLOT(imageClicked(QModelIndex)));
}

IconChooser::~IconChooser()
{
}

void IconChooser::showImage(int num)
{
    QStandardItem* imageitem = new QStandardItem();
    QString icn_path = ":/profession/prof_" + QString::number(num) + ".png";
    if(QFile::exists(icn_path)){
        imageitem->setIcon(QIcon(icn_path));
        imageitem->setData(num,Qt::UserRole);
        imageitem->setData(icn_path,Qt::UserRole+1);
        if(icn_path.contains("/"))
            icn_path = icn_path.mid(icn_path.lastIndexOf("/")+1);
        imageitem->setData(icn_path,Qt::UserRole+2);
        imageitem->setToolTip(icn_path);
        m_standardModel->appendRow(imageitem);
    }
}

void IconChooser::imageClicked(QModelIndex index)
{
    if(index.row() < m_standardModel->rowCount()){
        QStandardItem *i = m_standardModel->itemFromIndex(index);
        m_selected_id = i->data(Qt::UserRole).toInt();
        m_lbl_info->setText(i->data(Qt::UserRole+2).toString());
    }else{
        m_selected_id = -1;
        m_lbl_info->setText(tr("No icon selected."));
    }
}

void IconChooser::accept(){
    return QDialog::accept();
}
void IconChooser::reject(){
    m_selected_id = -1;
    return QDialog::reject();
}
