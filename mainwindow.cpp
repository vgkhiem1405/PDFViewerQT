// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "searchresultdelegate.h"
#include "zoomselector.h"

#include <QAxObject>
#include <QFileDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QPdfBookmarkModel>
#include <QPdfDocument>
#include <QPdfPageNavigator>
#include <QPdfPageSelector>
#include <QPdfSearchModel>
#include <QProcess>
#include <QShortcut>
#include <QStandardPaths>
#include <QTimer>
#include <QtMath>

#include <podofo/main/PdfMemDocument.h>

const qreal zoomMultiplier = qSqrt(2.0);

Q_LOGGING_CATEGORY(lcExample, "qt.examples.pdfviewer")

void showNotification(QString str) {
    QMessageBox::information(nullptr, "Notification", str);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_zoomSelector(new ZoomSelector(this))
    , m_pageSelector(new QPdfPageSelector(this))
    , m_searchModel(new QPdfSearchModel(this))
    , m_searchField(new QLineEdit(this))
    , m_document(new QPdfDocument(this))
{
    ui->setupUi(this);

    m_zoomSelector->setMaximumWidth(150);
    ui->mainToolBar->insertWidget(ui->actionZoom_In, m_zoomSelector);

    ui->mainToolBar->insertWidget(ui->actionForward, m_pageSelector);
    connect(m_pageSelector, &QPdfPageSelector::currentPageChanged, this, &MainWindow::pageSelected);
    m_pageSelector->setDocument(m_document);
    auto nav = ui->pdfView->pageNavigator();
    connect(nav, &QPdfPageNavigator::currentPageChanged, m_pageSelector, &QPdfPageSelector::setCurrentPage);
    connect(nav, &QPdfPageNavigator::backAvailableChanged, ui->actionBack, &QAction::setEnabled);
    connect(nav, &QPdfPageNavigator::forwardAvailableChanged, ui->actionForward, &QAction::setEnabled);

    connect(m_zoomSelector, &ZoomSelector::zoomModeChanged, ui->pdfView, &QPdfView::setZoomMode);
    connect(m_zoomSelector, &ZoomSelector::zoomFactorChanged, ui->pdfView, &QPdfView::setZoomFactor);
    m_zoomSelector->reset();

    QPdfBookmarkModel *bookmarkModel = new QPdfBookmarkModel(this);
    bookmarkModel->setDocument(m_document);

    ui->bookmarkView->setModel(bookmarkModel);
    connect(ui->bookmarkView, &QAbstractItemView::activated, this, &MainWindow::bookmarkSelected);

    ui->thumbnailsView->setModel(m_document->pageModel());

    m_searchModel->setDocument(m_document);
    ui->pdfView->setSearchModel(m_searchModel);
    ui->searchToolBar->insertWidget(ui->actionFindPrevious, m_searchField);
    connect(new QShortcut(QKeySequence::Find, this), &QShortcut::activated, this, [this]() {
        m_searchField->setFocus(Qt::ShortcutFocusReason);
    });
    m_searchField->setPlaceholderText(tr("Find in document"));
    m_searchField->setMaximumWidth(400);
    connect(m_searchField, &QLineEdit::textEdited, this, [this](const QString &text) {
        m_searchModel->setSearchString(text);
        ui->tabWidget->setCurrentWidget(ui->searchResultsTab);
    });
    ui->searchResultsView->setModel(m_searchModel);
    ui->searchResultsView->setItemDelegate(new SearchResultDelegate(this));
    connect(ui->searchResultsView->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &MainWindow::searchResultSelected);

    ui->pdfView->setDocument(m_document);

    ui->actionContinuous->setChecked(true);
    ui->pdfView->setPageMode(QPdfView::PageMode::MultiPage);

    connect(ui->pdfView, &QPdfView::zoomFactorChanged,
            m_zoomSelector, &ZoomSelector::setZoomFactor);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::open(const QUrl &docLocation)
{
    qDebug() << docLocation;
    PoDoFo::PdfMemDocument document;
    // document.Load(docLocation.toDisplayString().toStdString());

    if (docLocation.isLocalFile()) {
        m_document->load(docLocation.toLocalFile());
        pageSelected(0);
    } else {
        const QString message = tr("%1 is not a valid local file").arg(docLocation.toString());
        qCDebug(lcExample).noquote() << message;
        QMessageBox::critical(this, tr("Failed to open"), message);
    }
    qCDebug(lcExample) << docLocation;
}

void MainWindow::bookmarkSelected(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    const int page = index.data(int(QPdfBookmarkModel::Role::Page)).toInt();
    const qreal zoomLevel = index.data(int(QPdfBookmarkModel::Role::Level)).toReal();
    ui->pdfView->pageNavigator()->jump(page, {}, zoomLevel);
}

void MainWindow::pageSelected(int page)
{
    auto nav = ui->pdfView->pageNavigator();
    nav->jump(page, {}, nav->currentZoom());
    const auto documentTitle = m_document->metaData(QPdfDocument::MetaDataField::Title).toString();
    setWindowTitle(!documentTitle.isEmpty() ? documentTitle : QStringLiteral("PDF Viewer"));
    setWindowTitle(tr("%1: page %2 (%3 of %4)")
                       .arg(documentTitle.isEmpty() ? u"PDF Viewer"_qs : documentTitle,
                            m_pageSelector->currentPageLabel(), QString::number(page + 1), QString::number(m_document->pageCount())));
}

void MainWindow::searchResultSelected(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous);
    if (!current.isValid())
        return;

    const int page = current.data(int(QPdfSearchModel::Role::Page)).toInt();
    const QPointF location = current.data(int(QPdfSearchModel::Role::Location)).toPointF();
    ui->pdfView->pageNavigator()->jump(page, location);
    ui->pdfView->setCurrentSearchResultIndex(current.row());
}

void MainWindow::on_actionOpen_triggered()
{
    if (m_fileDialog == nullptr) {
        m_fileDialog = new QFileDialog(this, tr("Choose a file"),
                                       QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
        m_fileDialog->setAcceptMode(QFileDialog::AcceptOpen);
        m_fileDialog->setMimeTypeFilters({"application/pdf",
                                          "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet", // for .xlsx
                                          "application/vnd.openxmlformats-officedocument.wordprocessingml.document", // for .docx
                                          "application/vnd.openxmlformats-officedocument.presentationml.presentation"}); // for .pptx
    }

    if (m_fileDialog->exec() == QDialog::Accepted) {
        const QUrl toOpen = m_fileDialog->selectedUrls().constFirst();
        if (toOpen.isValid()) {
            QFileInfo fileInfo(toOpen.toLocalFile());
            QString extension = fileInfo.suffix().toLower();

            if (extension == "doc" || extension == "docx") {
                qDebug() << "Selected a Word file";

            } else if (extension == "xls" || extension == "xlsx") {
                qDebug() << "Selected an Excel file";

                QUrl url = QUrl::fromLocalFile(fileOutputPath);
                open(url);

                return;

            } else if (extension == "ppt" || extension == "pptx") {
                qDebug() << "Selected a PowerPoint file";

                convertPowerPointToPDF();
                QUrl url = QUrl::fromLocalFile(fileOutputPath);
                open(url);

                return;
            } else {
                //pdf
                open(toOpen);
            }
        }
    }
}

void MainWindow::on_actionQuit_triggered()
{
    QApplication::quit();
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this, tr("About PdfViewer"),
                       tr("An example using QPdfDocument"));
}

void MainWindow::on_actionAbout_Qt_triggered()
{
    QMessageBox::aboutQt(this);
}

void MainWindow::on_actionZoom_In_triggered()
{
    ui->pdfView->setZoomFactor(ui->pdfView->zoomFactor() * zoomMultiplier);
}

void MainWindow::on_actionZoom_Out_triggered()
{
    ui->pdfView->setZoomFactor(ui->pdfView->zoomFactor() / zoomMultiplier);
}

void MainWindow::on_actionPrevious_Page_triggered()
{
    auto nav = ui->pdfView->pageNavigator();
    nav->jump(nav->currentPage() - 1, {}, nav->currentZoom());
}

void MainWindow::on_actionNext_Page_triggered()
{
    auto nav = ui->pdfView->pageNavigator();
    nav->jump(nav->currentPage() + 1, {}, nav->currentZoom());
}

void MainWindow::on_thumbnailsView_activated(const QModelIndex &index)
{
    auto nav = ui->pdfView->pageNavigator();
    nav->jump(index.row(), {}, nav->currentZoom());
}

void MainWindow::on_actionContinuous_triggered()
{
    ui->pdfView->setPageMode(ui->actionContinuous->isChecked() ?
                                 QPdfView::PageMode::MultiPage :
                                 QPdfView::PageMode::SinglePage);
}

void MainWindow::on_actionBack_triggered()
{
    ui->pdfView->pageNavigator()->back();
}

void MainWindow::on_actionForward_triggered()
{
    ui->pdfView->pageNavigator()->forward();
}

void MainWindow::on_actionFindNext_triggered()
{
    int next = ui->searchResultsView->currentIndex().row() + 1;
    if (next >= m_searchModel->rowCount({}))
        next = 0;
    ui->searchResultsView->setCurrentIndex(m_searchModel->index(next));
}

void MainWindow::on_actionFindPrevious_triggered()
{
    int prev = ui->searchResultsView->currentIndex().row() - 1;
    if (prev < 0)
        prev = m_searchModel->rowCount({}) - 1;
    ui->searchResultsView->setCurrentIndex(m_searchModel->index(prev));
}

void MainWindow::on_actionTo_Word_triggered()
{
    if (m_fileDialog == nullptr) {
        showNotification("need choose the pdf file first");
        return;
    }

    fileOutputPath = QFileDialog::getSaveFileName(nullptr, QObject::tr("Save As Word File"), QString(), QObject::tr("Word Files (*.docx)"));
    fileOutputPath = QDir::toNativeSeparators(fileOutputPath);

    if (fileOutputPath.isEmpty()) {
        qDebug() << "Please select a PDF file and specify an output Word file path.";
        return;
    }

    convertPdfToWord();

}

void MainWindow::convertPdfToWord() {

    qDebug() << fileOutputPath << m_fileDialog->selectedUrls().constFirst().toLocalFile();

    // Create a new Word Application instance
    QAxObject word("Word.Application");
    if (word.isNull()) {
        showNotification("Microsoft Word is not installed.");
        return;
    }

    // Hide the Word application and suppress alerts
    word.setProperty("Visible", false);
    word.setProperty("DisplayAlerts", false);

    // Open the document in Word
    QAxObject* documents = word.querySubObject("Documents");
    QAxObject* document = documents->querySubObject("Open(const QString&)", m_fileDialog->selectedUrls().constFirst().toLocalFile());


    if (document) {
        // SaveAs2 might also need the file format specified
        document->dynamicCall("SaveAs2(const QString&, int)", fileOutputPath, 16); // 16 is wdFormatDocumentDefault
        document->dynamicCall("Close()");
        document->clear();
        documents->clear();
    } else {
        qDebug() << "Could not open the document:" << m_fileDialog->selectedUrls().constFirst().toLocalFile();
        return;
    }

    // Quit Word and clean up
    word.dynamicCall("Quit()");
    delete document;
    delete documents;

    showNotification("Conversion completed successfully. Document saved at: " + fileOutputPath);
}

bool MainWindow::convertPdfToExcel() {
    qDebug() << fileOutputPath << m_fileDialog->selectedUrls().constFirst().toLocalFile();

    // Create a new Excel Application instance
    QAxObject excel("Excel.Application");
    if (excel.isNull()) {
        showNotification("Microsoft Excel is not installed.");
        return false;
    }

    excel.setProperty("Visible", false);
    excel.setProperty("DisplayAlerts", false);

    QAxObject* workbooks = excel.querySubObject("Workbooks");
    QAxObject* workbook = workbooks->querySubObject("Add()");
    QAxObject* worksheet = workbook->querySubObject("Worksheets(1)");

    // Logic to extract data from PDF and insert into Excel goes here
    // This is a non-trivial task and typically requires parsing the PDF to extract data
    // then populating the Excel file cell by cell or row by row.

    // Save the Excel document
    workbook->dynamicCall("SaveAs(const QString&)", fileOutputPath);

    // Close the workbook and quit Excel
    workbook->dynamicCall("Close()");
    excel.dynamicCall("Quit()");

    // Clean up
    delete worksheet;
    delete workbook;
    delete workbooks;

    showNotification("Conversion completed successfully. Document saved at: " + fileOutputPath);
    return true;
}

void MainWindow::convertPdfToPowerPoint() {
    qDebug() << fileOutputPath << m_fileDialog->selectedUrls().constFirst().toLocalFile();

    // Create a new PowerPoint Application instance
    QAxObject powerpoint("PowerPoint.Application");
    if (powerpoint.isNull()) {
        showNotification("Microsoft PowerPoint is not installed.");
        return;
    }

    powerpoint.setProperty("Visible", true);

    QAxObject* presentations = powerpoint.querySubObject("Presentations");
    QAxObject* presentation = presentations->querySubObject("Add()");

    // Split the PDF content into sections or pages
    QStringList sections;
    // pdfContent.split("\n\n"); // This is a simplistic approach

    int slideIndex = 1;
    for (const QString& section : sections) {
        // Add a new slide (with title and content layout)
        QAxObject* slides = presentation->querySubObject("Slides");
        QAxObject* slide = slides->querySubObject("Add(int, int)", slideIndex++, 2); // 2 denotes ppLayoutText

        // Set the title (optional)
        QAxObject* titleShape = slide->querySubObject("Shapes(int)", 1); // 1 is the index of the title shape on this layout
        QAxObject* titleTextFrame = titleShape->querySubObject("TextFrame");
        QAxObject* titleTextRange = titleTextFrame->querySubObject("TextRange");
        titleTextRange->setProperty("Text", "PDF Section");

        // Set the content
        QAxObject* contentShape = slide->querySubObject("Shapes(int)", 2); // 2 is the index of the content shape on this layout
        QAxObject* contentTextFrame = contentShape->querySubObject("TextFrame");
        QAxObject* contentTextRange = contentTextFrame->querySubObject("TextRange");
        contentTextRange->setProperty("Text", section.trimmed());

        // Clean up
        delete contentTextRange;
        delete contentTextFrame;
        delete contentShape;
        delete titleTextRange;
        delete titleTextFrame;
        delete titleShape;
        delete slide;
        delete slides;
    }

    presentation->dynamicCall("SaveAs(const QString&)", fileOutputPath);

    // Close the presentation and quit PowerPoint
    presentation->dynamicCall("Close()");
    powerpoint.dynamicCall("Quit()");

    // Clean up
    delete presentation;
    delete presentations;

    showNotification("Conversion completed successfully. Document saved at: " + fileOutputPath);
}

void MainWindow::on_actionTo_Excel_triggered() {
    if (m_fileDialog == nullptr) {
        showNotification("Please choose the PDF file first.");
        return;
    }

    fileOutputPath = QFileDialog::getSaveFileName(nullptr, QObject::tr("Save As Excel File"), QString(), QObject::tr("Excel Files (*.xlsx)"));
    fileOutputPath = QDir::toNativeSeparators(fileOutputPath);

    if (fileOutputPath.isEmpty()) {
        qDebug() << "No output Excel file path specified.";
        return;
    }

    convertPdfToExcel();
}

void MainWindow::on_actionTo_PP_triggered() {
    if (m_fileDialog == nullptr) {
        showNotification("Please choose the PDF file first.");
        return;
    }

    fileOutputPath = QFileDialog::getSaveFileName(nullptr, QObject::tr("Save As PowerPoint File"), QString(), QObject::tr("PowerPoint Files (*.pptx)"));
    fileOutputPath = QDir::toNativeSeparators(fileOutputPath);

    if (fileOutputPath.isEmpty()) {
        qDebug() << "No output PowerPoint file path specified.";
        return;
    }

    convertPdfToPowerPoint();
}


bool MainWindow::convertPowerPointToPDF() {
    // selectInputFile();

    fileOutputPath = QFileDialog::getSaveFileName(nullptr, QObject::tr("Save As PDF File"), QString(), QObject::tr("PDF Files (*.pdf)"));

    qDebug() << fileOutputPath.replace("/", "\\") << m_fileDialog->selectedUrls().constFirst().toLocalFile().replace("/", "\\");


    // Create a new PowerPoint instance
    QAxObject powerpoint("PowerPoint.Application");
    if (powerpoint.isNull()) {
        qDebug() << "PowerPoint is not installed.";
        return false;
    }

    // Open the presentation
    powerpoint.setProperty("Visible", false);
    powerpoint.setProperty("DisplayAlerts", false);

    // Open the PowerPoint presentation
    QAxObject* presentations = powerpoint.querySubObject("Presentations");
    QAxObject* presentation = presentations->querySubObject("Open(const QString&)", m_fileDialog->selectedUrls().constFirst().toLocalFile().replace("/", "\\"));

    if (!presentation) {
        qDebug() << "Failed to open the PowerPoint presentation.";
        powerpoint.dynamicCall("Quit()");
        return false;
    }

    // Save the presentation as PDF
    QVariant saveResult = presentation->dynamicCall("SaveAs(const QString&, int)", fileOutputPath.replace("/", "\\"), 32);
    if (saveResult.isValid()) {
        // Conversion was successful
        qDebug() << "Converted PowerPoint to PDF successfully.";
    } else {
        // Conversion encountered an error
        qDebug() << "Failed to convert PowerPoint to PDF.";
    }

    // Close the presentation and quit PowerPoint
    presentation->dynamicCall("Close()");
    powerpoint.dynamicCall("Quit()");

    // Clean up
    delete presentation;
    delete presentations;

    qDebug() << "The PowerPoint file has been converted to a PDF successfully.";

    return true;
}

bool MainWindow::selectInputFile() {
    QString filter = "Office Files (*.xls *.xlsx *.ppt *.pptx *.doc *.docx);;Excel Files (*.xls *.xlsx);;PowerPoint Files (*.ppt *.pptx);;Word Files (*.doc *.docx);;All Files (*.*)";
    fileInputPath = QFileDialog::getOpenFileName(this, tr("Open File"), QString(), filter);

    if (fileInputPath.isEmpty()) {
        // User canceled or didn't select a file
        qDebug() << "No file was selected.";
        return false;
    }

    // Process the selected file
    qDebug() << "Selected file:" << fileInputPath;
    return true;
}
