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

#include "ProjectsList.h"
#include <QMouseEvent>

/*
===================
ProjectsList::mousePressEvent
===================
*/
void ProjectsList::mousePressEvent(QMouseEvent *event)
{
    QModelIndex item = indexAt(event->pos());
    QListView::mousePressEvent(event);

    // Clears selection when a user clicks on an empty area
    if (!item.isValid()) clearSelection();
}

/*
===================
ProjectsList::keyPressEvent
===================
*/
void ProjectsList::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Delete && selectionModel()->selectedIndexes().size())
        emit deletePressed();

    QListView::keyPressEvent(event);
}
