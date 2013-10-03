#ifndef ICONCHOOSER_H
#define ICONCHOOSER_H

#include <QtWidgets>

class IconChooser : public QDialog
{
    Q_OBJECT

public:
    explicit IconChooser(QWidget *parent = 0);//QStringList imageNamesList, QWidget *parent = 0);
    ~IconChooser();

    int selected_id;
public Q_SLOTS:
    void showImage(int num);
    void finished();

    void imageClicked(QModelIndex);

private:
    QStringList m_imageNamesList;
    QFutureWatcher<QImage>*     m_imageScaler;
    QListView*                  m_imageListView;
    QStandardItemModel*         m_standardModel;
};

#endif // ICONCHOOSER_H
