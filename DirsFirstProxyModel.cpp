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

#include "DirsFirstProxyModel.h"

#include <QFileSystemModel>
#include <QFileInfo>

/*
===================
DirsFirstProxyModel::lessThan
===================
*/
bool DirsFirstProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    // Name
    if (sortColumn() == 0)
    {
        QFileSystemModel *fileSystem = qobject_cast<QFileSystemModel*>(sourceModel());
        QFileInfo leftFileInfo = fileSystem->fileInfo(left);
        QFileInfo rightFileInfo = fileSystem->fileInfo(right);

        // Sorts drives alphabetically by letter
        if (!left.parent().isValid() && !right.parent().isValid())
            return leftFileInfo.path().compare(rightFileInfo.path(), Qt::CaseInsensitive) < 0;

        // Moves directories to the top of the list
        if (!leftFileInfo.isDir() && rightFileInfo.isDir())
            return false;
        else if (leftFileInfo.isDir() && !rightFileInfo.isDir())
            return true;

        return leftFileInfo.fileName().compare(rightFileInfo.fileName(), Qt::CaseInsensitive) < 0;
    }

    return QSortFilterProxyModel::lessThan(left, right);
}
