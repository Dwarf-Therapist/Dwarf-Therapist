#include <QGridLayout>
#include "iconchooser.h"

const int imageSize = 16;

QImage scale(const QString &imageFileName)
{
    QImage image(imageFileName);
    return image.scaled(QSize(imageSize, imageSize), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

IconChooser::IconChooser(QWidget *parent)
    : QDialog(parent)
    , selected_id(-1)
{

    QGridLayout* m_gridLayout = new QGridLayout(this);
    m_imageListView = new QListView(this);
    m_gridLayout->addWidget(m_imageListView);

    m_imageListView->setViewMode( QListView::IconMode );
    m_imageListView->setUniformItemSizes(true);
    m_imageListView->setSelectionRectVisible(true);
    m_imageListView->setMovement(QListView::Static);
    m_imageListView->setSelectionMode(QListView::SingleSelection);
    m_imageListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_imageListView->setToolTip(tr("Double click to select an icon."));

    m_imageListView->setResizeMode(QListView::Adjust);

    m_standardModel = new QStandardItemModel(this);
    m_imageListView->setModel(m_standardModel);


    for(int i = 1; i < 107; i++){
        showImage(i);
    }


    connect(m_imageListView, SIGNAL(doubleClicked(QModelIndex)), SLOT(imageClicked(QModelIndex)));
}

IconChooser::~IconChooser()
{
}

void IconChooser::showImage(int num)
{
    QStandardItem* imageitem = new QStandardItem();
    QString icn_path = ":/profession/prof_" + QString::number(num) + ".png";
    imageitem->setIcon(QIcon(icn_path));
    imageitem->setData(num,Qt::UserRole);
    m_standardModel->appendRow(imageitem);
}

void IconChooser::finished()
{
}

void IconChooser::imageClicked(QModelIndex index)
{
    if(index.row() < m_standardModel->rowCount()){
        QStandardItem *i = m_standardModel->itemFromIndex(index);
        selected_id = i->data(Qt::UserRole).toInt();
    }else{
        selected_id = -1;
    }
    close();
}
