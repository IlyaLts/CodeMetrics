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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "FileSelectorModel.h"

#define SETTINGS_FILENAME "Settings.ini"
#define PROJECTS_FILENAME "Projects.ini"
#define METRICS_FILENAME "Metrics.ini"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QTableWidgetItem;
class QStringListModel;
class QItemSelection;
class ProjectsList;
class DirsFirstProxyModel;

enum cursorState
{
    None,
    StringLiteral,
    CharacterLiteral,
    SingleLineComment,
    MultiLineComment
};

struct Language
{
    enum Type
    {
        Assembly,
        Basic,
        C,
        CSharp,
        CPP,
        CHeader,
        Clojure,
        CoffeeScript,
        D,
        FSharp,
        GLSL,
        Go,
        Groovy,
        Haskell,
        HLSL,
        Java,
        JavaScript,
        Kotlin,
        Lisp,
        Lua,
        ObjectC,
        PERL,
        Pascal,
        PHP,
        Python,
        R,
        Ruby,
        Rust,
        Scala,
        SQL,
        Swift,
        TypeScript,
        TypeCount,
        None
    } type;

    const char *name;
    const char *singleComment[16];
    const char *multipleCommentStart[16];
    const char *multipleCommentEnd[16];
    const char *ext[16];
};

struct SourceFile
{
    QString filename;
    Language::Type langType;
};

struct MetricsData
{
    int sourceFiles = 0;
    int lines = 0;
    int linesOfCode = 0;
    int commentLines = 0;
    int commentWords = 0;
    int blankLines = 0;
};

/*
===========================================================

    MainWindow

===========================================================
*/
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public Q_SLOTS:

    void addProject();
    void removeProject();
    void projectClicked(const QItemSelection &selected, const QItemSelection &deselected);
    void projectNameChanged(const QModelIndex &index);
    void count();
    void sort(int column);
    void scrollToCenter();

protected:

    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:

    void addPath(MetricsData &dataTotal, QList<SourceFile> &filesList, const QString &path, const QString &ext);
    void updateMetricsDifference() const;
    void showDifference(QTableWidgetItem *item, int current, int previous) const;
    bool checkForKeyword(const QString &line, int index, const char *keyword) const;
    Language::Type getLanguageType(const QString &ext) const;
    int getMetricsTableIndex(Language::Type index) const;

    bool counting = false;
    bool canUpdateDiff = false;
    bool scrollable = false;
    Ui::MainWindow *ui;
    QStringListModel *projectsListModel;
    FileSelectorModel *fileSelectorModel;
    DirsFirstProxyModel *proxyModel;
    QStringList projectNames;
    QList<QStringList> projectPathList;
    MetricsData dataCurrent[Language::TypeCount];
    MetricsData dataPrevious[Language::TypeCount];

    ProjectsList *projectsList;
};

#endif // MAINWINDOW_H
