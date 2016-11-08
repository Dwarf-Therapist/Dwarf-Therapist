#ifndef ICONCHOOSER_H
#define ICONCHOOSER_H

#include <QDialog>
#include <QStandardItemModel>
#include <QListView>
#include <QLabel>

class IconChooser : public QDialog
{
    Q_OBJECT

public:
    explicit IconChooser(QWidget *parent = 0);
    ~IconChooser();

    int selected_id() const {return m_selected_id;}

public Q_SLOTS:
    void showImage(int num);
    void imageClicked(QModelIndex);

    void accept();
    void reject();

private:
    QStringList m_imageNamesList;
    QListView *m_imageListView;
    QLabel *m_lbl_info;
    QStandardItemModel* m_standardModel;
    int m_selected_id;
};

#endif // ICONCHOOSER_H
