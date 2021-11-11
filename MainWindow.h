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

namespace Ui {
class MainWindow;
}

class QTableWidgetItem;
class QStringListModel;
class QItemSelection;
class ProjectsList;
class DirsFirstProxyModel;

enum cursorState_t
{
    STRING_LITERAL,
    CHARACTER_LITERAL,
    SINGLE_COMMENT,
    MULTIPLE_COMMENT,
    NONE
};

struct Language
{
    enum type_t
    {
        ASSEMBLY,
        C,
        CPP,
        CHEADER,
        OBJECTC,
        D,
        JAVA,
        CSHARP,
        FSHARP,
        SWIFT,
        GO,
        RUST,
        SCALA,
        GROOVY,
        SQL,
        LUA,
        LISP,
        PYTHON,
        KOTLIN,
        CLOJURE,
        TYPESCRIPT,
        COFFEESCRIPT,
        HASKELL,
        RUBY,
        R,
        PERL,
        JAVASCRIPT,
        PHP,
        PASCAL,
        BASIC,
        GLSL,
        HLSL,
        NONE
    } type;

    const char *name;
    const char *singleComment[16];
    const char *multipleCommentStart[16];
    const char *multipleCommentEnd[16];
    const char *ext[16];
};

struct SourceFile
{
    SourceFile(const QString &filename, Language::type_t lang) : filename(filename), lang(lang) {}

    QString filename;
    Language::type_t lang;
};

struct MetricsData
{
    MetricsData() : sourceFiles(0), lines(0), linesOfCode(0), commentLines(0), commentWords(0), blankLines(0) {}

    int sourceFiles;
    int lines;
    int linesOfCode;
    int commentLines;
    int commentWords;
    int blankLines;
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

protected:

    void resizeEvent(QResizeEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private:

    void updateMetricsDifference() const;
    void showDifference(QTableWidgetItem *item, int current, int previous) const;
    bool checkForKeyword(const QString &line, int index, const char *keyword) const;
    Language::type_t getLanguageType(const QString &ext) const;
    int getMetricsTableIndex(Language::type_t index) const;

    bool counting;
    Ui::MainWindow *ui;
    QStringListModel *projectsListModel;
    FileSelectorModel *fileSelectorModel;
    DirsFirstProxyModel *proxyModel;
    QStringList projectNames;
    QList<QStringList> projectPathList;
    MetricsData dataCurrent[Language::NONE];
    MetricsData dataPrevious[Language::NONE];

    ProjectsList *projectsList;
};

#endif // MAINWINDOW_H
