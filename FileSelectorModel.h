/*
===============================================================================
    Copyright (C) 2015-2021 Ilya Lyakhovets

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
===============================================================================
*/

#ifndef FILESELECTORMODEL_H
#define FILESELECTORMODEL_H

#include <QFileSystemModel>

/*
===========================================================

    FileSelectorModel

===========================================================
*/
class FileSelectorModel : public QFileSystemModel
{
    Q_OBJECT

public:

    explicit FileSelectorModel(QObject *parent = nullptr);

    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    void setChecked(const QStringList &pathList);
    void getPathList(QStringList &pathList, const QModelIndex &parent = QModelIndex()) const;

private Q_SLOTS:

    void updateCheckboxes(const QString &path);

private:

    void getChildren(QModelIndexList &list, const QModelIndex &parent) const;
    void getAllChildren(QModelIndexList &list, const QModelIndex &parent) const;

    QSet<QPersistentModelIndex> checkedList;
    QSet<QPersistentModelIndex> partiallyCheckedList;
};

#endif // FILESELECTORMODEL_H
