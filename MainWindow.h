/*
===============================================================================
    Copyright (C) 2015-2017 Ilya Lyakhovets

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

#define CODE_METRICS_VERSION "1.7"
#define SETTINGS_FILENAME "Settings.ini"
#define PREVIOUS_METRICS_FILENAME "PreviousMetrics.dat"

namespace Ui {
class MainWindow;
}

class QTableWidgetItem;

enum cursorState_t
{
    NONE,
    IN_STRING,
    IN_C_STRING,
    IN_SINGLE_COMMENT,
    IN_MULTIPLE_COMMENT
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

    void operator +=(const MetricsData &other) { sourceFiles += other.sourceFiles; lines += other.lines; linesOfCode += other.linesOfCode; commentLines += other.commentLines; commentWords += other.commentWords; blankLines += other.blankLines; };

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

public slots:

    void slotBrowseButton();
    void slotCountButton();
    void slotUpdateMetricsData();
    void slotSort(int column);

protected:
    void resizeEvent(QResizeEvent *event);

private:

    int CheckForKeyword(const QString &line, int index, const char *keyword) const;
    void MakeDiff(QTableWidgetItem *item, int current, int previous) const;
    Language::type_t GetLanguageType(const QString &ext) const;
    int GetTableIndex(Language::type_t index) const;

    bool counting;
    Ui::MainWindow *ui;
    MetricsData dataCurrent[Language::NONE];
    MetricsData dataPrevious[Language::NONE];
};

#endif // MAINWINDOW_H
