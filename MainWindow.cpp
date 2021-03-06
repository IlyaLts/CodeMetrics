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

#include <QSettings>
#include <QMessageBox>
#include <QFileDialog>
#include <QDirIterator>
#include <QFile>
#include <QTextStream>

#include "MainWindow.h"
#include "ui_MainWindow.h"

#define NUMBER_OF_METRICS 7

Language langList[Language::NONE] = {
{ Language::ASSEMBLY,    "Assembly",       {"//", ";", "%"},   {""},                   {""},                   {"asm", "nasm", "s"}},
{ Language::C,           "C",              {"//"},             {"/*"},                 {"*/"},                 {"c"}},
{ Language::CPP,         "C++",            {"//"},             {"/*"},                 {"*/"},                 {"cpp", "cc", "cxx", "c++", "inl", "ipp"}},
{ Language::CHEADER,     "C/C++ Header",   {"//"},             {"/*"},                 {"*/"},                 {"h", "hh", "hpp", "h++", "hpp", "hxx"}},
{ Language::OBJECTC,     "Object-C",       {"//"},             {"/*"},                 {"*/"},                 {"m", "mm"}},
{ Language::D,           "D",              {"//"},             {"/*", "/+"},           {"*/", "+/"},           {"d"}},
{ Language::JAVA,        "Java",           {"//"},             {"/*"},                 {"*/"},                 {"java"}},
{ Language::CSHARP,      "C#",             {"//"},             {"/*"},                 {"*/"},                 {"cs"}},
{ Language::FSHARP,      "F#",             {"//"},             {"/*", "(*"},           {"*/", "*)"},           {"fs" "fsx"}},
{ Language::SWIFT,       "Swift",          {"//"},             {"/*"},                 {"*/"},                 {"swift"}},
{ Language::LUA,         "Lua",            {"--"},             {"/*", "--[["},         {"*/", "]]"},           {"lua"}},
{ Language::GO,          "Go",             {"//"},             {"/*"},                 {"*/"},                 {"go"}},
{ Language::RUST,        "Rust",           {"//"},             {"/*"},                 {"*/"},                 {"rs"}},
{ Language::SCALA,       "Scala",          {"//"},             {"/*"},                 {"*/"},                 {"scala"}},
{ Language::GROOVY,      "Groovy",         {"//"},             {"/*"},                 {"*/"},                 {"groovy", "gvy", "gy", "gsh"}},
{ Language::SQL,         "SQL",            {"#", "--"},        {"/*"},                 {"*/"},                 {"sql"}},
{ Language::LISP,        "Lisp",           {"//"},             {"#|"},                 {"|#"},                 {"lisp"}},
{ Language::PYTHON,      "Python",         {"#"},              {"\"\"\"", "\'\'\'"},   {"\"\"\"", "\'\'\'"},   {"py"}},
{ Language::KOTLIN,      "Kotlin",         {"//"},             {"/*"},                 {"*/"},                 {"kt", "kts"}},
{ Language::CLOJURE,     "Clojure",        {";"},              {""},                   {""},                   {"clj", "cljs", "cljc", "edn"}},
{ Language::TYPESCRIPT,  "TypeScript",     {"//"},             {"/*"},                 {"*/"},                 {"ts", "tsx"}},
{ Language::COFFEESCRIPT,"CoffeeScript",   {"#"},              {"###"},                {"###"},                {"coffee", "litcoffee"}},
{ Language::HASKELL,     "Haskell",        {"--"},             {"{-"},                 {"-}"},                 {"hs", "lhs"}},
{ Language::RUBY,        "Ruby",           {"#"},              {"=begin"},             {"=end"},               {"rb", "rbw"}},
{ Language::R,           "R",              {"#"},              {""},                   {""},                   {"r"}},
{ Language::PERL,        "Perl",           {"#"},              {""},                   {""},                   {"pl", "pm", "perl", "t", "pod"}},
{ Language::JAVASCRIPT,  "JavaScript",     {"//"},             {"/*"},                 {"*/"},                 {"js", "json"}},
{ Language::PHP,         "PHP",            {"#"},              {"/*"},                 {"*/"},                 {"php", "phtml", "php3", "php4", "php5", "phps"}},
{ Language::PASCAL,      "Pascal",         {"//"},             {"(*", "{"},            {"*)", "}"},            {"pas", "p"}},
{ Language::BASIC,       "BASIC",          {"\\"},             {""},                   {""},                   {"bas", "vb"}}};

/*
===================
MainWindow::MainWindow
===================
*/
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    counting = false;
    QSettings settings(SETTINGS_FILENAME, QSettings::IniFormat);

    ui->setupUi(this);
    ui->centralWidget->setLayout(ui->verticalLayout);
    setWindowTitle(QString("Code Metrics"));
    setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
    resize(QSize(settings.value("width", 800).toInt(), settings.value("height", 400).toInt()));
    setWindowState(settings.value("fullscreen", false).toBool() ? Qt::WindowMaximized : Qt::WindowActive);
    ui->sourceCodeDirectoryLineEdit->setText(settings.value("sourceCodeDirectory").toString());

    for (int i = 0; i <= Language::NONE; i++)
    {
        QTableWidgetItem *item = new QTableWidgetItem();
        if (i < Language::NONE)
        {
            item->setText(QString(langList[i].name));
        }
        else
        {
            item->setText("Summary:");
            item->setBackground(QBrush(QColor(240, 240, 240)));
        }

        ui->metricsTableWidget->insertRow(i);
        ui->metricsTableWidget->setItem(i, 0, item);
        ui->metricsTableWidget->hideRow(i);

        for (int j = 0; j < NUMBER_OF_METRICS-1; j++)
        {
            QTableWidgetItem* newItem = new QTableWidgetItem();
            newItem->setTextAlignment(Qt::AlignCenter);
            if (i == Language::NONE) newItem->setBackground(QBrush(QColor(240, 240, 240)));
            ui->metricsTableWidget->setItem(i, j+1, newItem);
        }
    }

    ui->metricsTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    connect(ui->browseButton, SIGNAL(clicked()), SLOT(slotBrowseButton()));
    connect(ui->countButton, SIGNAL(clicked()), SLOT(slotCountButton()));
    connect(ui->metricsTableWidget->horizontalHeader(), SIGNAL(sectionClicked(int)), SLOT(slotSort(int)));
}

/*
===================
MainWindow::~MainWindow
===================
*/
MainWindow::~MainWindow()
{
    QSettings settings(SETTINGS_FILENAME, QSettings::IniFormat);
    settings.setValue("sourceCodeDirectory", ui->sourceCodeDirectoryLineEdit->text());
    settings.setValue("fullscreen", isMaximized());

    if (!isMaximized())
    {
        settings.setValue("width", size().width());
        settings.setValue("height", size().height());
    }

    delete ui;
}

/*
===================
MainWindow::slotBrowseButton
===================
*/
void MainWindow::slotBrowseButton()
{
    QString filename = QFileDialog::getExistingDirectory(this, "Browse For Folder", ui->sourceCodeDirectoryLineEdit->text(), QFileDialog::ShowDirsOnly);
    if (!filename.isEmpty()) ui->sourceCodeDirectoryLineEdit->setText(filename);
}

/*
===================
MainWindow::slotCountButton
===================
*/
void MainWindow::slotCountButton()
{
    QSettings metricsList(PREVIOUS_METRICS_FILENAME, QSettings::IniFormat);
    QList<SourceFile> filesList;
    MetricsData dataSum;

    // Strip whitespaces and '/', '\\' characters at the end of path
    QString path = ui->sourceCodeDirectoryLineEdit->text();
    while (path.size() && path[0] == ' ') path.remove(0, 1);
    while (path.size() && (path[path.size() - 1] == '/' || path[path.size() - 1] == '\\')) path.remove(path.size() - 1, 1);
    ui->sourceCodeDirectoryLineEdit->setText(path);

    if (path.isEmpty())
        return;

    // Widgets are off
    counting = true;
    ui->sourceCodeDirectoryLineEdit->setReadOnly(true);
    ui->browseButton->setEnabled(false);
    ui->countButton->setEnabled(false);

    // Reset all metrics data
    ui->progressBar->setValue(0);

    for (int i = 0; i < ui->metricsTableWidget->rowCount(); i++)
    {
        ui->metricsTableWidget->hideRow(i);

        for (int j = 1; j < ui->metricsTableWidget->columnCount(); j++)
        {
            ui->metricsTableWidget->item(i, j)->setText(nullptr);
            if (i != Language::NONE) ui->metricsTableWidget->item(i, j)->setBackground(QBrush(QColor("White")));
        }
    }

    memset(&dataCurrent, 0, sizeof(MetricsData) * Language::NONE);

    // Count files
    ui->progressBar->setFormat("Counting files...");
    QDirIterator sourceDirectory(path, QDir::Dirs | QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (sourceDirectory.hasNext())
    {
        sourceDirectory.next();

        if(sourceDirectory.fileInfo().isFile())
        {
            Language::type_t lang = GetLanguageType(sourceDirectory.fileInfo().completeSuffix());

            if (lang != Language::NONE)
            {
                filesList.append(SourceFile(sourceDirectory.fileInfo().filePath(), lang));
                ui->metricsTableWidget->showRow(GetTableIndex(lang));
                ui->metricsTableWidget->showRow(Language::NONE);
                dataCurrent[lang].sourceFiles++;
                dataSum.sourceFiles++;
                ui->metricsTableWidget->item(GetTableIndex(lang), 1)->setData(Qt::EditRole, dataCurrent[lang].sourceFiles);
                ui->metricsTableWidget->item(Language::NONE, 1)->setData(Qt::EditRole, dataSum.sourceFiles);
            }
        }

        QApplication::processEvents();
    }

    ui->progressBar->setFormat("%p%");

    // Count source lines
    for (int i = 0, files = 0; i < filesList.size(); i++)
    {
        Language::type_t lang = filesList[i].lang;

        // File
        if (lang != Language::NONE)
        {
            QFile file(filesList[i].filename);
            file.open(QIODevice::ReadOnly);
            QTextStream in(&file);

            cursorState_t cursor = NONE;

            // Read a file
            while (!in.atEnd())
            {
                bool isThereCommentLine = (cursor == IN_MULTIPLE_COMMENT ? true : false);
                bool isTherePureCodeLine = false;

                QString line = in.readLine();
                cursor = isThereCommentLine ? IN_MULTIPLE_COMMENT : NONE;

                dataCurrent[lang].lines++;
                dataSum.lines++;

                // Blank line
                if (line.simplified().isEmpty())
                {
                    dataCurrent[lang].blankLines++;
                    dataSum.blankLines++;
                }

                // Read a line
                for (int j = 0; j < line.length(); j++)
                {
                    int offset;

                    if (cursor == NONE)
                    {
                        for (int k = 0; langList[lang].singleComment[k] || langList[lang].multipleCommentStart[k]; k++)
                        {
                            // Single Comment
                            if (langList[lang].singleComment[k] && (offset = CheckForKeyword(line, j, langList[lang].singleComment[k])))
                            {
                                j += offset;
                                cursor = IN_SINGLE_COMMENT;
                                isThereCommentLine = true;
                                break;
                            }
                            // Multiple comment - begin
                            if (langList[lang].multipleCommentStart[k] && (offset = CheckForKeyword(line, j, langList[lang].multipleCommentStart[k])) && (lang != Language::RUBY || (j == 0 && !line[j + offset].isLetterOrNumber())))
                            {
                                j += offset;
                                cursor = IN_MULTIPLE_COMMENT;
                                isThereCommentLine = true;
                                break;
                            }
                        }

                        if (j >= line.length()) break;
                    }

                    // Multiple comment - end
                    if (cursor == IN_MULTIPLE_COMMENT)
                    {
                        for (int k = 0; langList[lang].multipleCommentEnd[k]; k++)
                        {
                            if ((offset = CheckForKeyword(line, j, langList[lang].multipleCommentEnd[k])) && (lang != Language::RUBY || (j == 0 && !line[j + offset].isLetterOrNumber())))
                            {
                                j += offset;
                                cursor = NONE;
                                isThereCommentLine = true;
                                break;
                            }
                        }

                        if (j >= line.length()) break;
                    }

                    // Comment words
                    if ((cursor == IN_SINGLE_COMMENT || cursor == IN_MULTIPLE_COMMENT) && ((j == 0 && line[j].isLetter()) || (j > 0 && !line[j-1].isLetter() && line[j].isLetter())))
                    {
                        dataCurrent[lang].commentWords++;
                        dataSum.commentWords++;
                    }

                    if (cursor != IN_SINGLE_COMMENT && cursor != IN_MULTIPLE_COMMENT)
                    {
                        // Pure code
                        if (line[j] >= '!' && line[j] <= '~') isTherePureCodeLine = true;

                        // Ignore comment symbols in string
                        if (!CheckForKeyword(line, j - 1, "\\") && CheckForKeyword(line, j, "\"") && cursor != IN_C_STRING)
                            cursor = (cursor == IN_STRING ? NONE : IN_STRING);

                        // Ignore comment symbols in character string
                        if (!CheckForKeyword(line, j - 1, "\\") && CheckForKeyword(line, j, "\'") && cursor != IN_STRING)
                            cursor = (cursor == IN_C_STRING ? NONE : IN_C_STRING);
                    }
                }

                if (isThereCommentLine)
                {
                    dataCurrent[lang].commentLines++;
                    dataSum.commentLines++;
                }
                if (isTherePureCodeLine)
                {
                    dataCurrent[lang].linesOfCode++;
                    dataSum.linesOfCode++;
                }
            }

            file.close();

            files++;
            ui->progressBar->setValue((float) files / filesList.size() * 100);

            ui->metricsTableWidget->item(GetTableIndex(lang), 2)->setData(Qt::EditRole, dataCurrent[lang].lines);
            ui->metricsTableWidget->item(GetTableIndex(lang), 3)->setData(Qt::EditRole, dataCurrent[lang].linesOfCode);
            ui->metricsTableWidget->item(GetTableIndex(lang), 4)->setData(Qt::EditRole, dataCurrent[lang].commentLines);
            ui->metricsTableWidget->item(GetTableIndex(lang), 5)->setData(Qt::EditRole, dataCurrent[lang].commentWords);
            ui->metricsTableWidget->item(GetTableIndex(lang), 6)->setData(Qt::EditRole, dataCurrent[lang].blankLines);

            ui->metricsTableWidget->item(Language::NONE, 2)->setData(Qt::EditRole, dataSum.lines);
            ui->metricsTableWidget->item(Language::NONE, 3)->setData(Qt::EditRole, dataSum.linesOfCode);
            ui->metricsTableWidget->item(Language::NONE, 4)->setData(Qt::EditRole, dataSum.commentLines);
            ui->metricsTableWidget->item(Language::NONE, 5)->setData(Qt::EditRole, dataSum.commentWords);
            ui->metricsTableWidget->item(Language::NONE, 6)->setData(Qt::EditRole, dataSum.blankLines);
        }

        QApplication::processEvents();
    }

    // Read all previous metrics data
    for (int i = 0; i < Language::NONE; i++)
    {
        dataPrevious[i].sourceFiles = metricsList.value(QString("%1_%2_SourceFiles").arg(path).arg(i), dataCurrent[i].sourceFiles).toInt();
        dataPrevious[i].lines = metricsList.value(QString("%1_%2_Lines").arg(path).arg(i), dataCurrent[i].lines).toInt();
        dataPrevious[i].linesOfCode = metricsList.value(QString("%1_%2_LinesOfCode").arg(path).arg(i), dataCurrent[i].linesOfCode).toInt();
        dataPrevious[i].commentLines = metricsList.value(QString("%1_%2_CommentLines").arg(path).arg(i), dataCurrent[i].commentLines).toInt();
        dataPrevious[i].commentWords = metricsList.value(QString("%1_%2_CommentWords").arg(path).arg(i), dataCurrent[i].commentWords).toInt();
        dataPrevious[i].blankLines = metricsList.value(QString("%1_%2_BlankLines").arg(path).arg(i), dataCurrent[i].blankLines).toInt();
    }

    counting = false;
    UpdateMetricsDataDifference();

    // Save all current metrics data for diff in the future
    for (int i = 0; i < Language::NONE; i++)
    {
        metricsList.setValue(QString("%1_%2_SourceFiles").arg(path).arg(i), dataCurrent[i].sourceFiles);
        metricsList.setValue(QString("%1_%2_Lines").arg(path).arg(i), dataCurrent[i].lines);
        metricsList.setValue(QString("%1_%2_LinesOfCode").arg(path).arg(i), dataCurrent[i].linesOfCode);
        metricsList.setValue(QString("%1_%2_CommentLines").arg(path).arg(i), dataCurrent[i].commentLines);
        metricsList.setValue(QString("%1_%2_CommentWords").arg(path).arg(i), dataCurrent[i].commentWords);
        metricsList.setValue(QString("%1_%2_BlankLines").arg(path).arg(i), dataCurrent[i].blankLines);
    }

    // Widgets are on
    ui->progressBar->setFormat("Done.");
    ui->sourceCodeDirectoryLineEdit->setReadOnly(false);
    ui->browseButton->setEnabled(true);
    ui->countButton->setEnabled(true);
}

/*
===================
MainWindow::slotSort
===================
*/
void MainWindow::slotSort(int column)
{
    QString temp[NUMBER_OF_METRICS];
    bool flag = false;

    for (int i = 0; i < ui->metricsTableWidget->rowCount(); i++)
        if (!ui->metricsTableWidget->isRowHidden(i))
            flag = true;

    if (!flag) return;

    for (int i = 0; i < NUMBER_OF_METRICS; i++)
        temp[i] = ui->metricsTableWidget->item(Language::NONE, i)->text();

    ui->metricsTableWidget->removeRow(Language::NONE);

    for (int i = 0; i < Language::NONE; i++)
    {
        ui->metricsTableWidget->item(GetTableIndex((Language::type_t) i), 1)->setData(Qt::EditRole, dataCurrent[i].sourceFiles);
        ui->metricsTableWidget->item(GetTableIndex((Language::type_t) i), 2)->setData(Qt::EditRole, dataCurrent[i].lines);
        ui->metricsTableWidget->item(GetTableIndex((Language::type_t) i), 3)->setData(Qt::EditRole, dataCurrent[i].linesOfCode);
        ui->metricsTableWidget->item(GetTableIndex((Language::type_t) i), 4)->setData(Qt::EditRole, dataCurrent[i].commentLines);
        ui->metricsTableWidget->item(GetTableIndex((Language::type_t) i), 5)->setData(Qt::EditRole, dataCurrent[i].commentWords);
        ui->metricsTableWidget->item(GetTableIndex((Language::type_t) i), 6)->setData(Qt::EditRole, dataCurrent[i].blankLines);
    }

    ui->metricsTableWidget->sortByColumn(column, ui->metricsTableWidget->horizontalHeader()->sortIndicatorOrder());

    QTableWidgetItem *item = new QTableWidgetItem;
    ui->metricsTableWidget->insertRow(Language::NONE);
    ui->metricsTableWidget->setItem(Language::NONE, 0, item);

    for (int i = 0; i < NUMBER_OF_METRICS; i++)
    {
        QTableWidgetItem* newItem = new QTableWidgetItem();
        newItem->setText(temp[i]);
        if (i) newItem->setTextAlignment(Qt::AlignCenter);
        newItem->setBackground(QBrush(QColor(240, 240, 240)));
        ui->metricsTableWidget->setItem(Language::NONE, i, newItem);
    }

    UpdateMetricsDataDifference();
}

/*
===================
MainWindow::resizeEvent
===================
*/
void MainWindow::resizeEvent(QResizeEvent *)
{
    repaint();
    QApplication::processEvents();
}

/*
===================
MainWindow::UpdateMetricsDataDifference
===================
*/
void MainWindow::UpdateMetricsDataDifference() const
{
    if (counting) return;

    for (int i = 0; i < Language::NONE; i++)
    {
        MakeDiff(ui->metricsTableWidget->item(GetTableIndex((Language::type_t) i), 1), dataCurrent[i].sourceFiles, dataPrevious[i].sourceFiles);
        MakeDiff(ui->metricsTableWidget->item(GetTableIndex((Language::type_t) i), 2), dataCurrent[i].lines, dataPrevious[i].lines);
        MakeDiff(ui->metricsTableWidget->item(GetTableIndex((Language::type_t) i), 3), dataCurrent[i].linesOfCode, dataPrevious[i].linesOfCode);
        MakeDiff(ui->metricsTableWidget->item(GetTableIndex((Language::type_t) i), 4), dataCurrent[i].commentLines, dataPrevious[i].commentLines);
        MakeDiff(ui->metricsTableWidget->item(GetTableIndex((Language::type_t) i), 5), dataCurrent[i].commentWords, dataPrevious[i].commentWords);
        MakeDiff(ui->metricsTableWidget->item(GetTableIndex((Language::type_t) i), 6), dataCurrent[i].blankLines, dataPrevious[i].blankLines);
    }
}

/*
===================
MainWindow::MakeDiff
===================
*/
void MainWindow::MakeDiff(QTableWidgetItem *item, int current, int previous) const
{
    if (current > previous)
    {
        item->setText(QString("%1 (+%2)").arg(current).arg(current - previous));
        item->setBackground(QBrush(QColor(192, 255, 192)));
    }
    else if (current < previous)
    {
        item->setText(QString("%1 (-%2)").arg(current).arg(previous - current));
        item->setBackground(QBrush(QColor(255, 192, 192)));
    }
    else
    {
        item->setText(QString("%1").arg(current));
        item->setBackground(QBrush(QColor(255, 255, 255)));
    }
}

/*
===================
MainWindow::CheckForKeyword
===================
*/
int MainWindow::CheckForKeyword(const QString &line, int index, const char *keyword) const
{
    int len = 0;
    if (!keyword) return 0;
    while (keyword[len] != '\0') len++;

    if (index < 0 || index + len > line.length())
        return 0;

    for (int i = 0; i < len; i++)
        if (line[index + i] != keyword[i])
            return 0;

    return len;
}

/*
===================
MainWindow::GetLanguageType
===================
*/
Language::type_t MainWindow::GetLanguageType(const QString &ext) const
{
    for (int i = 0; i < Language::NONE; i++)
        for (int j = 0; langList[i].ext[j]; j++)
            if (!ext.compare(langList[i].ext[j], Qt::CaseInsensitive))
                return (Language::type_t) i;

    return Language::NONE;
}

/*
===================
MainWindow::GetTableIndex
===================
*/
int MainWindow::GetTableIndex(Language::type_t type) const
{
    for (int i = 0; i < Language::NONE; i++)
        if (!QString::compare(langList[(int) type].name, ui->metricsTableWidget->item(i, 0)->text(), Qt::CaseInsensitive))
            return i;

    return -1;
}
