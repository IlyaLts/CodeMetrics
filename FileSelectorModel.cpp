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

#include "FileSelectorModel.h"

/*
===================
FileSelectorModel::FileSelectorModel
===================
*/
FileSelectorModel::FileSelectorModel(QObject *parent) : QFileSystemModel(parent)
{
    connect(this, SIGNAL(directoryLoaded(QString)), this, SLOT(updateCheckboxes(QString)));
}

/*
===================
FileSelectorModel::data
===================
*/
QVariant FileSelectorModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::CheckStateRole && index.column() == 0)
    {
        if (checkedList.contains(index))
            return Qt::Checked;
        else if (partiallyCheckedList.contains(index))
            return Qt::PartiallyChecked;
        else
            return Qt::Unchecked;
    }

    return QFileSystemModel::data(index, role);
}

/*
===================
FileSelectorModel::setData
===================
*/
bool FileSelectorModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::CheckStateRole && index.column() == 0)
    {
        QModelIndexList allChildren;
        getAllChildren(allChildren, index);

        if (value == Qt::Checked)
        {
            checkedList.insert(index);
            partiallyCheckedList.remove(index);
            emit dataChanged(index, index);

            for (auto &child : allChildren)
            {
                checkedList.insert(child);
                partiallyCheckedList.remove(child);
            }

            if (!allChildren.isEmpty()) emit dataChanged(allChildren[0], allChildren[allChildren.size() - 1]);
        }
        else if (value == Qt::Unchecked)
        {
            checkedList.remove(index);
            partiallyCheckedList.remove(index);
            emit dataChanged(index, index);

            for (auto &child : allChildren)
            {
                checkedList.remove(child);
                partiallyCheckedList.remove(child);
            }

            if (!allChildren.isEmpty()) emit dataChanged(allChildren[0], allChildren[allChildren.size() - 1]);
        }

        // Sets parent checkboxes partially checked, checked, or unchecked depending on their children's checkboxes.
        QModelIndex parent = index.parent();

        while (parent.isValid())
        {
            QModelIndexList children;
            getChildren(children, parent);

            bool hasChecked = false;
            bool hasPartiallyChecked = false;
            bool hasUnchecked = false;

            for (auto &child : children)
            {
                 QVariant childValue = child.data(Qt::CheckStateRole);

                if (childValue == Qt::Checked)
                    hasChecked = true;
                else if (childValue == Qt::PartiallyChecked)
                    hasPartiallyChecked = true;
                else if (childValue == Qt::Unchecked)
                    hasUnchecked = true;
            }

            // Partially checked.
            if ((hasChecked && hasUnchecked) || hasPartiallyChecked || (hasChecked && canFetchMore(parent)))
            {
                checkedList.remove(parent);
                partiallyCheckedList.insert(parent);
            }
            // Unchecked.
            else if (!hasChecked && hasUnchecked)
            {
                checkedList.remove(parent);
                partiallyCheckedList.remove(parent);
            }
            // Checked.
            else if (hasChecked && !hasUnchecked)
            {
                checkedList.insert(parent);
                partiallyCheckedList.remove(parent);
            }

            emit dataChanged(parent, parent);
            parent = parent.parent();
        }
    }

    return QFileSystemModel::setData(index, value, role);
}

/*
===================
FileSelectorModel::flags
===================
*/
Qt::ItemFlags FileSelectorModel::flags(const QModelIndex &index) const
{
    return QFileSystemModel::flags(index) | Qt::ItemIsUserCheckable;
}

/*
===================
FileSelectorModel::setChecked
===================
*/
void FileSelectorModel::setChecked(const QStringList &pathList)
{
    for (int i = 0; i < rowCount(); i++)
        setData(index(i, 0, QModelIndex()), Qt::Unchecked, Qt::CheckStateRole);

    for (auto &path : pathList)
        setData(index(path), Qt::Checked, Qt::CheckStateRole);
}

/*
===================
FileSelectorModel::getPathList
===================
*/
void FileSelectorModel::getPathList(QStringList &list, const QModelIndex &parent) const
{
    for (int i = 0; i < rowCount(parent); i++)
    {
        QVariant value = index(i, 0, parent).data(Qt::CheckStateRole);

        if (value == Qt::Checked)
            list.push_back(filePath(index(i, 0, parent)));
        else if (value == Qt::PartiallyChecked)
            getPathList(list, index(i, 0, parent));
    }
}

/*
===================
FileSelectorModel::updateCheckboxes
===================
*/
void FileSelectorModel::updateCheckboxes(const QString &path)
{
    QModelIndex parent = index(path);
    QModelIndexList children;
    getChildren(children, parent);

    // Sets children checkboxes checked if a parent checkbox is checked too.
    if (parent.data(Qt::CheckStateRole) == Qt::Checked && !children.isEmpty())
    {
        for (auto &child : children)
            checkedList.insert(child);

        emit dataChanged(children[0], children[children.size() - 1]);
    }
}

/*
===================
FileSelectorModel::getChildren
===================
*/
void FileSelectorModel::getChildren(QModelIndexList &list, const QModelIndex &parent) const
{
    for (int i = 0; i < rowCount(parent); i++)
        list.push_back(parent.model()->index(i, 0, parent));
}

/*
===================
FileSelectorModel::getAllChildren
===================
*/
void FileSelectorModel::getAllChildren(QModelIndexList &list, const QModelIndex &parent) const
{
    for (int i = 0; i < rowCount(parent); i++)
    {
        list.push_back(parent.model()->index(i, 0, parent));
        getAllChildren(list, parent.model()->index(i, 0, parent));
    }
}
