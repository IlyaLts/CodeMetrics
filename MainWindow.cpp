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

#include <QFile>
#include <QDirIterator>
#include <QTextStream>
#include <QFileIconProvider>
#include <QStringListModel>
#include <QSettings>
#include <QStandardPaths>
#include <QCloseEvent>

#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "DirsFirstProxyModel.h"
#include "FileSelectorModel.h"
#include "ProjectsList.h"

#define NUMBER_OF_METRICS 7

Language langList[Language::TypeCount] = {
{ Language::Assembly,    "Assembly",       {"//", ";", "#"},   {"/*"},                 {"*/"},                 {"asm", "nasm", "s"}},
{ Language::Basic,       "BASIC",          {"'", "REM"},       {},                     {},                     {"bas", "vb"}},
{ Language::C,           "C",              {"//"},             {"/*"},                 {"*/"},                 {"c"}},
{ Language::CSharp,      "C#",             {"//"},             {"/*"},                 {"*/"},                 {"cs"}},
{ Language::CPP,         "C++",            {"//"},             {"/*"},                 {"*/"},                 {"cpp", "cc", "cxx", "c++", "inl", "ipp"}},
{ Language::CHeader,     "C/C++ Header",   {"//"},             {"/*"},                 {"*/"},                 {"h", "hh", "hpp", "h++", "hpp", "hxx"}},
{ Language::Clojure,     "Clojure",        {";"},              {},                     {},                     {"clj", "cljs", "cljc", "edn"}},
{ Language::CoffeeScript,"CoffeeScript",   {"#"},              {"###"},                {"###"},                {"coffee", "litcoffee"}},
{ Language::D,           "D",              {"//"},             {"/*", "/+"},           {"*/", "+/"},           {"d"}},
{ Language::FSharp,      "F#",             {"//"},             {"/*", "(*"},           {"*/", "*)"},           {"fs" "fsx"}},
{ Language::GLSL,        "GLSL",           {"//"},             {"/*"},                 {"*/"},                 {"vert", "tesc", "tese", "geom", "frag", "comp", "glsl", "glslv"}},
{ Language::Go,          "Go",             {"//"},             {"/*"},                 {"*/"},                 {"go"}},
{ Language::Groovy,      "Groovy",         {"//"},             {"/*"},                 {"*/"},                 {"groovy", "gvy", "gy", "gsh"}},
{ Language::Haskell,     "Haskell",        {"--"},             {"{-"},                 {"-}"},                 {"hs", "lhs"}},
{ Language::HLSL,        "HLSL",           {"//"},             {"/*"},                 {"*/"},                 {"hlsl"}},
{ Language::Java,        "Java",           {"//"},             {"/*"},                 {"*/"},                 {"java"}},
{ Language::JavaScript,  "JavaScript",     {"//"},             {"/*"},                 {"*/"},                 {"js", "json"}},
{ Language::Kotlin,      "Kotlin",         {"//"},             {"/*"},                 {"*/"},                 {"kt", "kts"}},
{ Language::Lisp,        "Lisp",           {"//"},             {"#|"},                 {"|#"},                 {"lisp"}},
{ Language::Lua,         "Lua",            {"--"},             {"/*", "--[["},         {"*/", "]]"},           {"lua"}},
{ Language::ObjectC,     "Object-C",       {"//"},             {"/*"},                 {"*/"},                 {"m", "mm"}},
{ Language::PERL,        "Perl",           {"#"},              {},                     {},                     {"pl", "pm", "perl", "t", "pod"}},
{ Language::Pascal,      "Pascal",         {"//"},             {"(*", "{"},            {"*)", "}"},            {"pas", "p"}},
{ Language::PHP,         "PHP",            {"#"},              {"/*"},                 {"*/"},                 {"php", "phtml", "php3", "php4", "php5", "phps"}},
{ Language::Python,      "Python",         {"#"},              {"\"\"\"", "\'\'\'"},   {"\"\"\"", "\'\'\'"},   {"py"}},
{ Language::R,           "R",              {"#"},              {},                     {},                     {"r"}},
{ Language::Ruby,        "Ruby",           {"#"},              {"=begin"},             {"=end"},               {"rb", "rbw"}},
{ Language::Rust,        "Rust",           {"//"},             {"/*"},                 {"*/"},                 {"rs"}},
{ Language::Scala,       "Scala",          {"//"},             {"/*"},                 {"*/"},                 {"scala"}},
{ Language::SQL,         "SQL",            {"#", "--"},        {"/*"},                 {"*/"},                 {"sql"}},
{ Language::Swift,       "Swift",          {"//"},             {"/*"},                 {"*/"},                 {"swift"}},
{ Language::TypeScript,  "TypeScript",     {"//"},             {"/*"},                 {"*/"},                 {"ts", "tsx"}}};

/*
===================
MainWindow::MainWindow
===================
*/
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    QSettings settings(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/" + SETTINGS_FILENAME, QSettings::IniFormat);

    ui->setupUi(this);
    ui->centralWidget->setLayout(ui->mainLayout);
    setWindowTitle(QString("Code Metrics"));
    setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
    resize(QSize(settings.value("Width", 800).toInt(), settings.value("Height", 400).toInt()));
    setWindowState(settings.value("Fullscreen", false).toBool() ? Qt::WindowMaximized : Qt::WindowActive);

    QVariantList hListDefault({ui->projectsLayout->minimumSize().width(), 999999});
    QVariantList vListDefault({ui->verticalSplitter->height() / 2, ui->verticalSplitter->height() / 2});
    QVariantList hList = settings.value("HorizontalSplitter", hListDefault).value<QVariantList>();
    QVariantList vList = settings.value("VerticalSplitter", vListDefault).value<QVariantList>();
    QList<int> hSizes, vSizes;

    for (auto &variant : hList)
        hSizes.push_back(variant.toInt());

    for (auto &variant : vList)
        vSizes.push_back(variant.toInt());

    ui->horizontalSplitter->setSizes(hSizes);
    ui->verticalSplitter->setSizes(vSizes);
    ui->horizontalSplitter->setStretchFactor(0, 0);
    ui->horizontalSplitter->setStretchFactor(1, 1);

    projectsListModel = new QStringListModel;
    ui->projectsList->setModel(projectsListModel);

    fileSelectorModel = new FileSelectorModel(this);
    fileSelectorModel->setIconProvider(new QFileIconProvider);
    fileSelectorModel->setReadOnly(true);
    fileSelectorModel->setFilter(QDir::AllEntries | QDir::NoSymLinks | QDir::NoDotAndDotDot);
    fileSelectorModel->setRootPath(QDir::currentPath());

    proxyModel = new DirsFirstProxyModel(this);
    proxyModel->setSourceModel(fileSelectorModel);

    ui->fileSelector->setModel(proxyModel);
    proxyModel->sort(0, Qt::AscendingOrder);
    ui->fileSelector->header()->setSizeAdjustPolicy(QHeaderView::AdjustToContents);
    ui->fileSelector->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->fileSelector->header()->adjustSize();

    ui->metricsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    for (int i = 0; i < Language::TypeCount + 1; i++)
    {
        QTableWidgetItem *item = new QTableWidgetItem();

        if (i != Language::TypeCount)
        {
            item->setText(QString(langList[i].name));
            item->setData(Qt::UserRole, i);
        }
        else
        {
            item->setText("Total:");
            item->setBackground(QBrush(QColor(240, 240, 240)));
        }

        ui->metricsTable->insertRow(i);
        ui->metricsTable->setItem(i, 0, item);
        ui->metricsTable->hideRow(i);

        for (int j = 1; j < NUMBER_OF_METRICS; j++)
        {
            QTableWidgetItem* newItem = new QTableWidgetItem();
            newItem->setTextAlignment(Qt::Alignment(Qt::AlignCenter));
            if (i == Language::TypeCount) newItem->setBackground(QBrush(QColor(240, 240, 240)));
            ui->metricsTable->setItem(i, j, newItem);
        }
    }

    QSettings projects(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/" + PROJECTS_FILENAME, QSettings::IniFormat);
    projectNames = projects.allKeys();
    for (auto &name : projectNames) projectPathList.push_back(projects.value(name).toStringList());
    projectsListModel->setStringList(projectNames);

// Fixes missing horizontal borders in headers on Windows
#ifdef Q_OS_WIN
    ui->fileSelector->header()->setStyleSheet("QHeaderView::section {"
                                              "padding-top: 4px;"
                                              "padding-bottom: -4px;"
                                              "padding-left: 4px;}");

    setStyleSheet("QHeaderView::section {"
                  "border-top:0px solid #D8D8D8;"
                  "border-left:0px solid #D8D8D8;"
                  "border-right:1px solid #D8D8D8;"
                  "border-bottom: 1px solid #D8D8D8;"
                  "background-color: #FAFAFA;}");
#endif

    connect(ui->addButton, SIGNAL(clicked()), SLOT(addProject()));
    connect(ui->removeButton, SIGNAL(clicked()), SLOT(removeProject()));
    connect(ui->countButton, SIGNAL(clicked()), SLOT(count()));
    connect(ui->metricsTable->horizontalHeader(), SIGNAL(sectionClicked(int)), SLOT(sort(int)));
    connect(ui->projectsList->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(projectClicked(QItemSelection,QItemSelection)));
    connect(ui->projectsList->model(), SIGNAL(dataChanged(QModelIndex,QModelIndex,QList<int>)), SLOT(projectNameChanged(QModelIndex)));
    connect(ui->projectsList, SIGNAL(deletePressed()), SLOT(removeProject()));
    connect(fileSelectorModel, SIGNAL(directoryLoaded(QString)), SLOT(scrollToCenter()));
    connect(ui->fileSelector, &QTreeView::expanded, this, [this](){ scrollable = false; });
}

/*
===================
MainWindow::~MainWindow
===================
*/
MainWindow::~MainWindow()
{
    QSettings settings(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/" + SETTINGS_FILENAME, QSettings::IniFormat);
    QVariantList hSizes, vSizes;

    for (auto &size : ui->horizontalSplitter->sizes())
        hSizes.push_back(size);

    for (auto &size : ui->verticalSplitter->sizes())
        vSizes.push_back(size);

    settings.setValue("HorizontalSplitter", hSizes);
    settings.setValue("VerticalSplitter", vSizes);
    settings.setValue("Fullscreen", isMaximized());

    if (!isMaximized())
    {
        settings.setValue("Width", size().width());
        settings.setValue("Height", size().height());
    }

    QSettings projects(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/" + PROJECTS_FILENAME, QSettings::IniFormat);
    projects.clear();

    if (!ui->projectsList->selectionModel()->selectedIndexes().isEmpty())
    {
        QStringList pathList;
        fileSelectorModel->getPathList(pathList);
        projectPathList[ui->projectsList->selectionModel()->selectedIndexes()[0].row()] = pathList;
    }

    for (int i = 0; i < projectNames.size(); i++)
        projects.setValue(projectNames[i], projectPathList[i]);

    delete ui;
}

/*
===================
MainWindow::addProject
===================
*/
void MainWindow::addProject()
{
    QStringList pathList;
    fileSelectorModel->getPathList(pathList);

    if (pathList.isEmpty())
        return;

    QString newName("New project");

    for (int i = 2; projectNames.contains(newName); i++)
        newName = QString("New project (%1)").arg(i);

    projectNames.push_back(newName);
    projectPathList.push_back(pathList);
    projectsListModel->setStringList(projectNames);

    // Prevents reloading of newly added projects, as they're already loaded
    ui->projectsList->selectionModel()->blockSignals(true);
    ui->projectsList->setCurrentIndex(ui->projectsList->model()->index(ui->projectsList->model()->rowCount() - 1, 0));
    ui->projectsList->selectionModel()->blockSignals(false);
}

/*
===================
MainWindow::removeProject
===================
*/
void MainWindow::removeProject()
{
    if (ui->projectsList->selectionModel()->selectedIndexes().isEmpty())
        return;

    for (auto &index : ui->projectsList->selectionModel()->selectedIndexes())
    {
        QSettings metricsData(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/" + METRICS_FILENAME, QSettings::IniFormat);
        int currentRow = index.row();

        ui->projectsList->model()->removeRow(currentRow);

        // Removes project metrics
        for (int i = 0; i < Language::TypeCount; i++)
        {
            QString langName(langList[i].name);
            langName.replace('/', ' ');

            metricsData.remove(QString("%1-%2-SourceFiles").arg(projectNames[currentRow], langName));
            metricsData.remove(QString("%1-%2-Lines").arg(projectNames[currentRow], langName));
            metricsData.remove(QString("%1-%2-LinesOfCode").arg(projectNames[currentRow], langName));
            metricsData.remove(QString("%1-%2-CommentLines").arg(projectNames[currentRow], langName));
            metricsData.remove(QString("%1-%2-CommentWords").arg(projectNames[currentRow], langName));
            metricsData.remove(QString("%1-%2-BlankLines").arg(projectNames[currentRow], langName));
        }

        projectNames.removeAt(currentRow);
        projectPathList.removeAt(currentRow);
    }

    fileSelectorModel->setData(fileSelectorModel->index(0, 0, QModelIndex()), Qt::Unchecked, Qt::CheckStateRole);
    ui->fileSelector->collapseAll();
    ui->projectsList->selectionModel()->reset();
}

/*
===================
MainWindow::projectClicked
===================
*/
void MainWindow::projectClicked(const QItemSelection &selected, const QItemSelection &deselected)
{
    ui->fileSelector->collapseAll();

    // Resets the selector when clicking on an empty area
    if (selected.indexes().isEmpty())
    {
        for (int i = 0; i < fileSelectorModel->rowCount(); i++)
            fileSelectorModel->setData(fileSelectorModel->index(i, 0, QModelIndex()), Qt::Unchecked, Qt::CheckStateRole);

        return;
    }

    // Saves the path list of a previously selected project
    for (auto &index : deselected.indexes())
    {
        QStringList pathList;
        fileSelectorModel->getPathList(pathList);
        projectPathList[index.row()] = pathList;
    }

    fileSelectorModel->setChecked(projectPathList[selected.indexes()[0].row()]);

    // Expands all directories with checked checkboxes
    for (auto &path : projectPathList[selected.indexes()[0].row()])
    {
        QModelIndex curIndex = fileSelectorModel->index(path).parent();

        while (curIndex.isValid())
        {
            ui->fileSelector->expand(proxyModel->mapFromSource(curIndex));
            curIndex = curIndex.parent();
        }
    }

    scrollable = true;
    scrollToCenter();
}

/*
===================
MainWindow::projectNameChanged
===================
*/
void MainWindow::projectNameChanged(const QModelIndex &index)
{
    QString newName(index.data(Qt::DisplayRole).toString());
    newName.remove('/');
    newName.remove('\\');
    int row = index.row();

    // Reverts to original name if project name already exists
    if (newName.compare(projectNames[row], Qt::CaseInsensitive) && (newName.isEmpty() || projectNames.contains(newName, Qt::CaseInsensitive)))
    {
        ui->projectsList->model()->setData(index, projectNames[row], Qt::DisplayRole);
        return;
    }

    QSettings metricsData(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/" + METRICS_FILENAME, QSettings::IniFormat);

    // Saves metrics data under a new project name
    for (int i = 0; i < Language::TypeCount; i++)
    {
        MetricsData data;

        QString langName(langList[i].name);
        langName.replace('/', ' ');

        data.sourceFiles = metricsData.value(QString("%1-%2-SourceFiles").arg(projectNames[row], langName), -1).toInt();
        data.lines = metricsData.value(QString("%1-%2-Lines").arg(projectNames[row], langName), -1).toInt();
        data.linesOfCode = metricsData.value(QString("%1-%2-LinesOfCode").arg(projectNames[row], langName), -1).toInt();
        data.commentLines = metricsData.value(QString("%1-%2-CommentLines").arg(projectNames[row], langName), -1).toInt();
        data.commentWords = metricsData.value(QString("%1-%2-CommentWords").arg(projectNames[row], langName), -1).toInt();
        data.blankLines = metricsData.value(QString("%1-%2-BlankLines").arg(projectNames[row], langName), -1).toInt();

        if (data.sourceFiles >= 0)
            metricsData.setValue(QString("%1-%2-SourceFiles").arg(newName, langName), data.sourceFiles);

        if (data.lines >= 0)
            metricsData.setValue(QString("%1-%2-Lines").arg(newName, langName), data.lines);

        if (data.linesOfCode >= 0)
            metricsData.setValue(QString("%1-%2-LinesOfCode").arg(newName, langName), data.linesOfCode);

        if (data.commentLines >= 0)
            metricsData.setValue(QString("%1-%2-CommentLines").arg(newName, langName), data.commentLines);

        if (data.commentWords >= 0)
            metricsData.setValue(QString("%1-%2-CommentWords").arg(newName, langName), data.commentWords);

        if (data.blankLines >= 0)
            metricsData.setValue(QString("%1-%2-BlankLines").arg(newName, langName), data.blankLines);

        metricsData.remove(QString("%1-%2-SourceFiles").arg(projectNames[row], langName));
        metricsData.remove(QString("%1-%2-Lines").arg(projectNames[row], langName));
        metricsData.remove(QString("%1-%2-LinesOfCode").arg(projectNames[row], langName));
        metricsData.remove(QString("%1-%2-CommentLines").arg(projectNames[row], langName));
        metricsData.remove(QString("%1-%2-CommentWords").arg(projectNames[row], langName));
        metricsData.remove(QString("%1-%2-BlankLines").arg(projectNames[row], langName));
    }

    projectNames[row] = newName;
}

/*
===================
MainWindow::count
===================
*/
void MainWindow::count()
{
    if (counting)
    {
        counting = false;
        canUpdateDiff = false;
        return;
    }

    // Widgets are off
    ui->projectsList->setEnabled(false);
    ui->fileSelector->setEnabled(false);
    ui->addButton->setEnabled(false);
    ui->removeButton->setEnabled(false);
    ui->countButton->setText("Stop");
    counting = true;

    // Resets all metrics data
    ui->progressBar->setValue(0);
    memset(&dataCurrent, 0, sizeof(MetricsData) * Language::TypeCount);

    for (int i = 0; i < ui->metricsTable->rowCount(); i++)
    {
        ui->metricsTable->hideRow(i);

        for (int j = 1; j < ui->metricsTable->columnCount(); j++)
        {
            ui->metricsTable->item(i, j)->setText(QString());

            if (i != Language::TypeCount)
                ui->metricsTable->item(i, j)->setBackground(QBrush(QColor("White")));
        }
    }

    ui->progressBar->setFormat("Counting files...");

    MetricsData dataTotal;
    QList<SourceFile> filesList;
    QList<QString> pathList;
    fileSelectorModel->getPathList(pathList);

    // Counts files
    for (auto &path : pathList)
    {
        QFileInfo fileInfo(path);

        if (!fileInfo.exists())
            continue;

        if (!counting)
            break;

        if (fileInfo.isFile())
        {
            addPath(dataTotal, filesList, path, fileInfo.completeSuffix());
        }
        else if (fileInfo.isDir())
        {
            QDirIterator sourceDirectory(path, QDir::Dirs | QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);

            while (sourceDirectory.hasNext() && counting)
            {
                sourceDirectory.next();

                if(sourceDirectory.fileInfo().isFile())
                    addPath(dataTotal, filesList, sourceDirectory.fileInfo().filePath(), sourceDirectory.fileInfo().completeSuffix());
            }
        }
    }

    ui->progressBar->setFormat("%p%");

    // Counts source lines
    for (int i = 0, files = 0; i < filesList.size() && counting; i++)
    {
        Language::Type langType = filesList[i].langType;

        if (langType != Language::None)
        {
            QFile file(filesList[i].filename);
            file.open(QIODevice::ReadOnly);
            if (!file.isOpen())
                continue;

            QTextStream in(&file);
            cursorState cursorState = None;

            // Reads a file
            while (!in.atEnd())
            {
                bool isThereCommentLine = (cursorState == MultiLineComment ? true : false);
                bool isThereCodeLine = false;
                bool narrowLiteral = false;

                QString line = in.readLine();
                cursorState = isThereCommentLine ? MultiLineComment : None;

                dataCurrent[langType].lines++;
                dataTotal.lines++;

                // Blank line
                if (line.simplified().isEmpty())
                {
                    dataCurrent[langType].blankLines++;
                    dataTotal.blankLines++;
                }

                // Reads a line
                for (int j = 0; j < line.length(); j++)
                {
                    if (cursorState == None)
                    {
                        for (int k = 0; langList[langType].singleComment[k] || langList[langType].multipleCommentStart[k]; k++)
                        {
                            // Single Comment
                            if (langList[langType].singleComment[k] && checkForKeyword(line, j, langList[langType].singleComment[k]))
                            {
                                j += strlen(langList[langType].singleComment[k]);
                                cursorState = SingleLineComment;
                                isThereCommentLine = true;
                                break;
                            }

                            // Multiple comment - begin
                            if (langList[langType].multipleCommentStart[k] && checkForKeyword(line, j, langList[langType].multipleCommentStart[k]) && (langType != Language::Ruby || (j == 0 && !line[j + strlen(langList[langType].multipleCommentStart[k])].isLetterOrNumber())))
                            {
                                j += strlen(langList[langType].multipleCommentStart[k]);
                                cursorState = MultiLineComment;
                                isThereCommentLine = true;
                                break;
                            }
                        }

                        if (j >= line.length()) break;
                    }

                    // Multiple comment - end
                    if (cursorState == MultiLineComment)
                    {
                        for (int k = 0; langList[langType].multipleCommentEnd[k]; k++)
                        {
                            if (checkForKeyword(line, j, langList[langType].multipleCommentEnd[k]) && (langType != Language::Ruby || (j == 0 && !line[j + strlen(langList[langType].multipleCommentEnd[k])].isLetterOrNumber())))
                            {
                                j += strlen(langList[langType].multipleCommentEnd[k]);
                                cursorState = None;
                                isThereCommentLine = true;
                                break;
                            }
                        }

                        if (j >= line.length()) break;
                    }

                    // Comment words
                    if ((cursorState == SingleLineComment || cursorState == MultiLineComment) && ((j == 0 && line[j].isLetter()) || (j > 0 && !line[j - 1].isLetter() && line[j].isLetter())))
                    {
                        dataCurrent[langType].commentWords++;
                        dataTotal.commentWords++;
                    }

                    if (cursorState != SingleLineComment && cursorState != MultiLineComment)
                    {
                        // A line of code
                        if (line[j] >= '!' && line[j] <= '~') isThereCodeLine = true;

                        // String literal
                        if (cursorState != CharacterLiteral && !narrowLiteral && checkForKeyword(line, j, "\""))
                            cursorState = (cursorState == StringLiteral ? None : StringLiteral);

                        // Character literal
                        if (cursorState != StringLiteral && !narrowLiteral && checkForKeyword(line, j, "\'"))
                            cursorState = (cursorState == CharacterLiteral ? None : CharacterLiteral);

                        // Escape Character
                        if (cursorState == CharacterLiteral || cursorState == StringLiteral)
                        {
                            if (checkForKeyword(line, j, "\\"))
                                narrowLiteral = !narrowLiteral;
                            else
                                narrowLiteral = false;
                        }
                    }
                }

                if (isThereCommentLine)
                {
                    dataCurrent[langType].commentLines++;
                    dataTotal.commentLines++;
                }

                if (isThereCodeLine)
                {
                    dataCurrent[langType].linesOfCode++;
                    dataTotal.linesOfCode++;
                }
            }

            file.close();

            files++;
            ui->progressBar->setValue((float) files / filesList.size() * 100);

            int row = getMetricsTableIndex(langType);

            ui->metricsTable->item(row, 2)->setData(Qt::EditRole, dataCurrent[langType].lines);
            ui->metricsTable->item(row, 3)->setData(Qt::EditRole, dataCurrent[langType].linesOfCode);
            ui->metricsTable->item(row, 4)->setData(Qt::EditRole, dataCurrent[langType].commentLines);
            ui->metricsTable->item(row, 5)->setData(Qt::EditRole, dataCurrent[langType].commentWords);
            ui->metricsTable->item(row, 6)->setData(Qt::EditRole, dataCurrent[langType].blankLines);

            ui->metricsTable->item(Language::TypeCount, 2)->setData(Qt::EditRole, dataTotal.lines);
            ui->metricsTable->item(Language::TypeCount, 3)->setData(Qt::EditRole, dataTotal.linesOfCode);
            ui->metricsTable->item(Language::TypeCount, 4)->setData(Qt::EditRole, dataTotal.commentLines);
            ui->metricsTable->item(Language::TypeCount, 5)->setData(Qt::EditRole, dataTotal.commentWords);
            ui->metricsTable->item(Language::TypeCount, 6)->setData(Qt::EditRole, dataTotal.blankLines);
        }

        QApplication::processEvents();
    }

    if (counting)
    {
        if (ui->projectsList->selectionModel()->isSelected(ui->projectsList->currentIndex()))
        {
            QSettings metricsData(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/" + METRICS_FILENAME, QSettings::IniFormat);
            int currentRow = ui->projectsList->currentIndex().row();

            // Updates previous metrics with new data
            for (int i = 0; i < Language::TypeCount; i++)
            {
                QString langName(langList[i].name);

                // Removes backslashes, as QSettings interprets them as special characters
                langName.replace('/', ' ');

                dataPrevious[i].sourceFiles = metricsData.value(QString("%1-%2-SourceFiles").arg(projectNames[currentRow], langName), dataCurrent[i].sourceFiles).toInt();
                dataPrevious[i].lines = metricsData.value(QString("%1-%2-Lines").arg(projectNames[currentRow], langName), dataCurrent[i].lines).toInt();
                dataPrevious[i].linesOfCode = metricsData.value(QString("%1-%2-LinesOfCode").arg(projectNames[currentRow], langName), dataCurrent[i].linesOfCode).toInt();
                dataPrevious[i].commentLines = metricsData.value(QString("%1-%2-CommentLines").arg(projectNames[currentRow], langName), dataCurrent[i].commentLines).toInt();
                dataPrevious[i].commentWords = metricsData.value(QString("%1-%2-CommentWords").arg(projectNames[currentRow], langName), dataCurrent[i].commentWords).toInt();
                dataPrevious[i].blankLines = metricsData.value(QString("%1-%2-BlankLines").arg(projectNames[currentRow], langName), dataCurrent[i].blankLines).toInt();

                metricsData.setValue(QString("%1-%2-SourceFiles").arg(projectNames[currentRow], langName), dataCurrent[i].sourceFiles);
                metricsData.setValue(QString("%1-%2-Lines").arg(projectNames[currentRow], langName), dataCurrent[i].lines);
                metricsData.setValue(QString("%1-%2-LinesOfCode").arg(projectNames[currentRow], langName), dataCurrent[i].linesOfCode);
                metricsData.setValue(QString("%1-%2-CommentLines").arg(projectNames[currentRow], langName), dataCurrent[i].commentLines);
                metricsData.setValue(QString("%1-%2-CommentWords").arg(projectNames[currentRow], langName), dataCurrent[i].commentWords);
                metricsData.setValue(QString("%1-%2-BlankLines").arg(projectNames[currentRow], langName), dataCurrent[i].blankLines);
            }

            updateMetricsDifference();
        }

        if (!filesList.isEmpty())
            ui->progressBar->setFormat("Done.");
        else
            ui->progressBar->setFormat("No source files have been found!");

        canUpdateDiff = true;
    }

    // Widgets are on
    ui->projectsList->setEnabled(true);
    ui->fileSelector->setEnabled(true);
    ui->addButton->setEnabled(true);
    ui->removeButton->setEnabled(true);
    ui->countButton->setText("Count");
    counting = false;
}

/*
===================
MainWindow::sort
===================
*/
void MainWindow::sort(int column)
{
    QString totalData[NUMBER_OF_METRICS];

    // Sorts only when at least one row is visible
    for (int i = 0; i < ui->metricsTable->rowCount(); i++)
    {
        if (!ui->metricsTable->isRowHidden(i))
        {
            // Removes metric differences for sorting
            for (int j = 0; j < Language::TypeCount; j++)
            {
                int row = getMetricsTableIndex((Language::Type) j);

                ui->metricsTable->item(row, 1)->setData(Qt::EditRole, dataCurrent[j].sourceFiles);
                ui->metricsTable->item(row, 2)->setData(Qt::EditRole, dataCurrent[j].lines);
                ui->metricsTable->item(row, 3)->setData(Qt::EditRole, dataCurrent[j].linesOfCode);
                ui->metricsTable->item(row, 4)->setData(Qt::EditRole, dataCurrent[j].commentLines);
                ui->metricsTable->item(row, 5)->setData(Qt::EditRole, dataCurrent[j].commentWords);
                ui->metricsTable->item(row, 6)->setData(Qt::EditRole, dataCurrent[j].blankLines);
            }

            for (int j = 0; j < NUMBER_OF_METRICS; j++)
                totalData[j] = ui->metricsTable->item(Language::TypeCount, j)->text();

            ui->metricsTable->removeRow(Language::TypeCount);
            ui->metricsTable->sortByColumn(column, ui->metricsTable->horizontalHeader()->sortIndicatorOrder());
            ui->metricsTable->insertRow(Language::TypeCount);

            for (int j = 0; j < NUMBER_OF_METRICS; j++)
            {
                QTableWidgetItem *totalItem = new QTableWidgetItem();

                if (j)
                    totalItem->setTextAlignment(Qt::Alignment(Qt::AlignCenter));

                totalItem->setText(totalData[j]);
                totalItem->setBackground(QBrush(QColor(240, 240, 240)));                
                ui->metricsTable->setItem(Language::TypeCount, j, totalItem);
            }

            if (!counting && canUpdateDiff)
                updateMetricsDifference();

            return;
        }
    }
}

/*
===================
MainWindow::scrollToCenter
===================
*/
void MainWindow::scrollToCenter()
{
    if (!scrollable || ui->projectsList->selectionModel()->selectedIndexes().isEmpty())
        return;

    int row = ui->projectsList->selectionModel()->selectedIndexes()[0].row();

    if (projectPathList[row].isEmpty())
        return;

    QModelIndex index = fileSelectorModel->index(projectPathList[row][0]);
    ui->fileSelector->scrollTo(proxyModel->mapFromSource(index), QAbstractItemView::PositionAtCenter);
}

/*
===================
MainWindow::closeEvent
===================
*/
void MainWindow::closeEvent(QCloseEvent *event)
{
    counting = false;
    QMainWindow::closeEvent(event);
}

/*
===================
MainWindow::resizeEvent
===================
*/
void MainWindow::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    repaint();
    QApplication::processEvents();
}

/*
===================
MainWindow::addPath
===================
*/
void MainWindow::addPath(MetricsData &dataTotal, QList<SourceFile> &filesList, const QString &path, const QString &ext)
{
    for (int i = 0; i < Language::TypeCount; i++)
    {
        for (int j = 0; langList[i].ext[j]; j++)
        {
            if (ext.compare(langList[i].ext[j], Qt::CaseInsensitive))
                continue;

            Language::Type langType = static_cast<Language::Type>(i);
            int row = getMetricsTableIndex(langType);

            filesList.append(SourceFile{path, langType});
            dataCurrent[langType].sourceFiles++;
            dataTotal.sourceFiles++;
            ui->metricsTable->showRow(row);
            ui->metricsTable->showRow(Language::TypeCount);
            ui->metricsTable->item(row, 1)->setData(Qt::EditRole, dataCurrent[langType].sourceFiles);
            ui->metricsTable->item(Language::TypeCount, 1)->setData(Qt::EditRole, dataTotal.sourceFiles);
            QApplication::processEvents();
            return;
        }
    }
}

/*
===================
MainWindow::updateMetricsDifference
===================
*/
void MainWindow::updateMetricsDifference() const
{
    for (int i = 0; i < Language::TypeCount; i++)
    {
        int row = getMetricsTableIndex(static_cast<Language::Type>(i));

        showDifference(ui->metricsTable->item(row, 1), dataCurrent[i].sourceFiles, dataPrevious[i].sourceFiles);
        showDifference(ui->metricsTable->item(row, 2), dataCurrent[i].lines, dataPrevious[i].lines);
        showDifference(ui->metricsTable->item(row, 3), dataCurrent[i].linesOfCode, dataPrevious[i].linesOfCode);
        showDifference(ui->metricsTable->item(row, 4), dataCurrent[i].commentLines, dataPrevious[i].commentLines);
        showDifference(ui->metricsTable->item(row, 5), dataCurrent[i].commentWords, dataPrevious[i].commentWords);
        showDifference(ui->metricsTable->item(row, 6), dataCurrent[i].blankLines, dataPrevious[i].blankLines);
    }
}

/*
===================
MainWindow::showDifference
===================
*/
void MainWindow::showDifference(QTableWidgetItem *item, int current, int previous) const
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
MainWindow::checkForKeyword
===================
*/
bool MainWindow::checkForKeyword(const QString &line, int index, const char *keyword) const
{
    int len = 0;

    while (keyword[len] != '\0')
        len++;

    if (len <= 0 || index < 0 || index + len > line.length())
        return false;

    for (int i = 0; i < len; i++)
        if (line[index + i] != keyword[i])
            return false;

    return true;
}

/*
===================
MainWindow::getMetricsTableIndex
===================
*/
int MainWindow::getMetricsTableIndex(Language::Type type) const
{
    for (int i = 0; i < ui->metricsTable->rowCount(); i++)
        if (ui->metricsTable->item(i, 0)->data(Qt::UserRole).toInt() == type)
            return i;

    return -1;
}
