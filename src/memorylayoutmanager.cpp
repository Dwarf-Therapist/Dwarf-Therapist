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

#include "memorylayoutmanager.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QNetworkReply>
#include <QRegularExpression>

#include "defines.h"
#include "memorylayout.h"
#include "standardpaths.h"
#include "truncatingfilelogger.h"

MemoryLayoutManager::MemoryLayoutManager(QObject *parent)
    : QAbstractItemModel(parent)
{
    reload();
}

MemoryLayoutManager::~MemoryLayoutManager()
{
}

void MemoryLayoutManager::reload()
{
    beginResetModel();
    m_layouts.clear();
    auto writable_dir = StandardPaths::writable_data_location();
    for (auto search_path: StandardPaths::data_locations()) {
        bool writable = search_path == writable_dir;
        QDir dir(search_path + "/memory_layouts/" + LAYOUT_SUBDIR);
        dir.setNameFilters({"*.ini"});
        dir.setFilter(QDir::NoDotAndDotDot | QDir::Readable | QDir::Files);
        dir.setSorting(QDir::Name);
        for (auto info: dir.entryInfoList()) {
            LOGI << "opening layout" << info.absoluteFilePath();
            auto layout = std::make_unique<MemoryLayout>(info);
            if (layout->is_valid()) {
                auto checksum = layout->checksum().toLower();
                auto it = std::find_if(m_layouts.begin(), m_layouts.end(),
                        [&checksum](const auto &version) {
                            return version.checksum == checksum;
                        });
                if (it == m_layouts.end())
                    it = m_layouts.insert(it, version_info{checksum, layout->game_version(), {}, nullptr});
                it->files.push_back(file_info{info, layout->git_sha(), writable});
                LOGI << "adding valid layout" << layout->game_version() << "checksum:" << layout->checksum() << "SHA:" << layout->git_sha();
                it->memory_layout = std::move(layout);
            }
            else {
                LOGI << "ignoring invalid layout";
            }
        }
    }
    endResetModel();
}

QModelIndex MemoryLayoutManager::index(int row, int column, const QModelIndex &parent) const
{
    if (!parent.isValid())
        return createIndex(row, column, -1);
    else
        return createIndex(row, column, parent.row());
}

QModelIndex MemoryLayoutManager::parent(const QModelIndex &index) const
{
    int parent_row = index.internalId();
    if (parent_row < 0)
        return {};
    else
        return createIndex(parent_row, 0, -1);
}

int MemoryLayoutManager::rowCount(const QModelIndex &index) const
{
    if (!index.isValid())
        return m_layouts.size();
    else if (static_cast<int>(index.internalId()) == -1)
        return m_layouts[index.row()].files.size();
    else
        return 0;
}

int MemoryLayoutManager::columnCount(const QModelIndex &) const
{
    return ColumnCount;
}

QVariant MemoryLayoutManager::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return {};
    int parent_row = index.internalId();
    if (parent_row == -1) {
        const auto &version = m_layouts[index.row()];
        switch (index.column()) {
        case NamePathColumn:
            switch (role) {
            case Qt::DisplayRole:
                return version.name;
            default:
                return {};
            }
        case ChecksumColumn:
            switch (role) {
            case Qt::DisplayRole:
                return version.checksum;
            case Qt::ToolTipRole:
                return tr("DF checksum/timestamp");
            default:
                return {};
            }
        default:
            return {};
        }
    }
    else {
        const auto &file = m_layouts[parent_row].files[index.row()];
        switch (index.column()) {
        case NamePathColumn:
            switch (role) {
            case Qt::DisplayRole:
                return file.fi.filePath();
            default:
                return {};
            }
        case ChecksumColumn:
            switch (role) {
            case Qt::DisplayRole:
                return file.git_sha;
            case Qt::ToolTipRole:
                return tr("Git SHA");
            default:
                return {};
            }
        default:
            return {};
        }
    }
}

const QFileInfo &MemoryLayoutManager::fileInfo(const QModelIndex &index) const
{
    int parent_row = index.internalId();
    if (parent_row == -1)
        return m_layouts[index.row()].files[0].fi;
    else
        return m_layouts[parent_row].files[index.row()].fi;
}

void MemoryLayoutManager::updateMemoryLayout(const QString &name, const QString &git_sha, const QByteArray &data)
{
    auto str = QString::fromLocal8Bit(data);
    auto checksum = QRegularExpression("checksum\\s*=\\s*(0[xX][0-9a-fA-F]+)")
            .match(str)
            .captured(1);
    LOGD << "Received data for memory layout" << name << checksum << git_sha;
    if (checksum.isNull())
        return;
    auto it = std::find_if(m_layouts.begin(), m_layouts.end(),
            [&checksum](const auto &version) {
                return version.checksum == checksum;
            });
    if (it == m_layouts.end()) {
        auto name = QRegularExpression("version_name\\s*=\\s*(.*)\\s*$", QRegularExpression::MultilineOption)
                .match(str)
                .captured(1);
        LOGD << "New game version" << name;
        beginInsertRows({}, m_layouts.size(), m_layouts.size());
        it = m_layouts.insert(it, version_info{checksum, name, {}, nullptr});
        endInsertRows();
    }
    int row = std::distance(m_layouts.begin(), it);
    auto parent = index(row, 0);

    file_info *user = nullptr, *system = nullptr;
    if (!it->files.empty()) {
        if (it->files[0].writable) {
            user = &it->files[0];
            if (it->files.size() > 1)
                system = &it->files[1];
        }
        else
            system = &it->files[0];
    }
    if (system && system->git_sha == git_sha && user) {
        // First system layout is already up-to-date but hidden by a user file
        // Delete the user file
        LOGI << "Removing outdated user memory layout" << user->fi.absoluteFilePath();
        QFile file(user->fi.absoluteFilePath());
        if (file.remove()) {
            beginRemoveRows(parent, 0, 0);
            it->files.erase(it->files.begin());
            endRemoveRows();
        }
        else {
            LOGE << "Failed to remove" << user->fi.absoluteFilePath();
            return;
        }
    }
    else if (user && user->git_sha != git_sha) {
        // Already a user file, update it
        LOGI << "Updating user memory layout" << user->fi.absoluteFilePath();
        QFile file(user->fi.absoluteFilePath());
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
        auto mode = QIODevice::WriteOnly | QIODevice::ExistingOnly | QIODevice::Truncate;
#else
        auto mode = QIODevice::WriteOnly | QIODevice::Truncate;
#endif
        if (!file.open(mode) || file.write(data) == -1) {
            LOGE << "Failed to update" << user->fi.absoluteFilePath();
            return;
        }
        user->git_sha = git_sha;
        dataChanged(index(0, 0, parent), index(0, ColumnCount-1, parent));
    }
    else if (!user && (!system || system->git_sha != git_sha)) {
        // No user file, create it
        QFileInfo fi(StandardPaths::writable_data_location() + "/memory_layouts/" + LAYOUT_SUBDIR + "/" + name);
        LOGI << "Adding new memory layout" << fi.absoluteFilePath();
        if (!fi.dir().exists() && !fi.dir().mkpath(".")) {
            LOGE << "Failed to create memory layouts directory " << fi.dir().absolutePath();
            return;
        }
        QFile file(fi.absoluteFilePath());
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
        auto mode = QIODevice::NewOnly;
#else
        auto mode = QIODevice::WriteOnly | QIODevice::Truncate;
#endif
        if (!file.open(mode) || file.write(data) == -1) {
            LOGE << "Failed to add" << fi.absoluteFilePath();
            return;
        }
        beginInsertRows(parent, 0, 0);
        it->files.insert(it->files.begin(), {fi, git_sha, true});
        endInsertRows();
    }
    else // no update needed
        return;

    auto layout = std::make_unique<MemoryLayout>(it->files[0].fi);
    std::swap(it->memory_layout, layout);
    if (!layout || layout->git_sha() != it->memory_layout->git_sha()) // only send the signal if the layout actually changed (ignore cleanups)
        memoryLayoutUpdated(*it->memory_layout);
}

QStringList MemoryLayoutManager::get_supported_versions() const
{
    QStringList supported_versions;
    for (const auto &v: m_layouts)
        supported_versions.append(QString("%1 (%2)")
                .arg(v.name)
                .arg(v.checksum));
    return supported_versions;
}

const MemoryLayout *MemoryLayoutManager::get_memory_layout(const QString &checksum) const
{
    auto it = std::find_if(m_layouts.begin(), m_layouts.end(), [&checksum](const auto &version){
            return version.checksum == checksum;
        });
    if (it == m_layouts.end())
        return nullptr;
    else
        return it->memory_layout.get();
}
