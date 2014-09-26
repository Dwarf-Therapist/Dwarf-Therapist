#ifndef ICONCHOOSER_H
#define ICONCHOOSER_H

#include <QDialog>
#include <QStandardItemModel>
#include <QListView>

class IconChooser : public QDialog
{
    Q_OBJECT

public:
    explicit IconChooser(QWidget *parent = 0);
    ~IconChooser();

    int selected_id;
public Q_SLOTS:
    void showImage(int num);
    void finished();

    void imageClicked(QModelIndex);

private:
    QStringList m_imageNamesList;
    QListView* m_imageListView;
    QStandardItemModel* m_standardModel;
};

#endif // ICONCHOOSER_H
