/*
Dwarf Therapist
Copyright (c) 2020 Clement Vuchener

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

#ifndef MEMORY_LAYOUT_MANAGER_H
#define MEMORY_LAYOUT_MANAGER_H

#include <QAbstractItemModel>
#include <QUrl>
#include <QFileInfo>
#include <memory>
#include <QNetworkAccessManager>

class MemoryLayout;

class MemoryLayoutManager: public QAbstractItemModel
{
    Q_OBJECT
public:
    MemoryLayoutManager(QObject *parent = nullptr);
    ~MemoryLayoutManager() override;

    enum Column {
        NamePathColumn,
        ChecksumColumn,
        ColumnCount
    };

    QModelIndex index(int row, int column, const QModelIndex &parent = {}) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &index = {}) const override;
    int columnCount(const QModelIndex &index = {}) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    const QFileInfo &fileInfo(const QModelIndex &index) const;

    void updateMemoryLayout(const QString &name, const QString &git_sha, const QByteArray &data);

    QStringList get_supported_versions() const;
    const MemoryLayout *get_memory_layout(const QString &checksum) const;

public slots:
    void reload();

signals:
    void memoryLayoutUpdated(const MemoryLayout &layout);

private:
    struct file_info {
        QFileInfo fi;
        QString git_sha;
        bool writable;
    };
    struct version_info {
        QString checksum;
        QString name;
        std::vector<file_info> files;
        std::unique_ptr<MemoryLayout> memory_layout;
    };

    std::vector<version_info> m_layouts;
    QNetworkAccessManager m_network;
};

#endif
