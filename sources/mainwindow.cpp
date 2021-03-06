//
// MIT License
//
// Copyright (c) 2021 Hubert Gruniaux
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#include <QDropEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QImageReader>
#include <QImageWriter>
#include <QInputDialog>
#include <QMessageBox>
#include <QMimeData>
#include <QPainter>
#include <QProxyStyle>
#include <QStyleOption>

#include <QMimeDatabase>

#include "mainwindow.hpp"
#include "reportissuedialog.hpp"
#include "ui_aboutdialog.h"
#include "ui_mainwindow.h"

// ========================================================
// class MainWindow
// ========================================================

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    PresetManager::makeDirectory();
    WatermarkManager::makeDirectory();

    ui->setupUi(this);
    ui->editor->setCropForm(ui->cropForm);
    ui->editor->setWatermarkForm(ui->watermarkForm);

    initActions();
    initZoomWidget();
    initSignals();

    m_rubberBand = new QRubberBand(QRubberBand::Rectangle, ui->editor);
    m_rubberBand->hide();

    loadPresets();
    setImage(QPixmap());

    setAcceptDrops(true);

    ui->menuSavedPreset->removeAction(ui->action_4);
}
MainWindow::~MainWindow()
{
    delete ui;
}

QPixmap MainWindow::image() const
{
    return m_image;
}
QPixmap MainWindow::finalImage() const
{
    if (!m_image.isNull())
        return ui->editor->generate();
    else
        return QPixmap();
}
void MainWindow::setImage(const QPixmap& image)
{
    ui->actionSave->setEnabled(!image.isNull());
    ui->actionSaveFinish->setEnabled(!image.isNull());
    ui->actionSaveAs->setEnabled(!image.isNull());

    m_image = image;
    ui->editor->setImage(image);
    ui->cropForm->setImageRect(image.rect());
    ui->cropForm->setCropRect(image.rect());
}
void MainWindow::loadImage(const QString& path)
{
    QImageReader reader(path);
    reader.setAutoTransform(true);

    const QImage image = reader.read();
    if (image.isNull()) {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
            tr("Cannot load %1: %2")
                .arg(QDir::toNativeSeparators(path), reader.errorString()));
    } else {
        m_zoomSlider->setEnabled(true);
        m_path = path;
        setImage(QPixmap::fromImage(image));
        const QString message = tr("Opened \"%1\", %2x%3, Depth: %4")
                                    .arg(QDir::toNativeSeparators(path))
                                    .arg(image.width())
                                    .arg(image.height())
                                    .arg(image.depth());
        statusBar()->showMessage(message);
    }
}

Preset MainWindow::preset(const QString& name) const
{
    Preset preset;
    preset.name = name;
    preset.watermark.anchor = ui->watermarkForm->watermarkAnchor();
    preset.watermark.index = ui->watermarkForm->watermarkIndex();
    preset.watermark.opacity = qRound(ui->watermarkForm->watermarkOpacity() * 100);
    preset.watermark.useSize = ui->watermarkForm->watermarkUseSize();
    preset.watermark.size = qRound(ui->watermarkForm->watermarkSize() * 100);
    preset.watermark.useRotation = ui->watermarkForm->watermarkUseRotation();
    preset.watermark.rotation = ui->watermarkForm->watermarkRotation();
    preset.watermark.useColor = ui->watermarkForm->watermarkUseColor();
    preset.watermark.color = ui->watermarkForm->watermarkColor();
    preset.watermark.useOffset = ui->watermarkForm->watermarkUseOffset();
    preset.watermark.offset = ui->watermarkForm->watermarkOffset();
    preset.crop.rect = ui->cropForm->cropRect();
    preset.crop.fixed = ui->cropForm->cropFixed();
    return preset;
}
void MainWindow::setPreset(const Preset& preset)
{
    ui->watermarkForm->setWatermarkIndex(preset.watermark.index);
    ui->watermarkForm->setWatermarkAnchor(static_cast<WatermarkAnchor>(preset.watermark.anchor));
    ui->watermarkForm->setWatermarkOpacity(preset.watermark.opacity / 100.);
    ui->watermarkForm->setWatermarkUseSize(preset.watermark.useSize);
    ui->watermarkForm->setWatermarkSize(preset.watermark.size / 100.);
    ui->watermarkForm->setWatermarkUseRotation(preset.watermark.useRotation);
    ui->watermarkForm->setWatermarkRotation(preset.watermark.rotation);
    ui->watermarkForm->setWatermarkUseColor(preset.watermark.useColor);
    ui->watermarkForm->setWatermarkColor(preset.watermark.color);
    ui->watermarkForm->setWatermarkUseOffset(preset.watermark.useOffset);
    ui->watermarkForm->setWatermarkOffset(preset.watermark.offset);
    ui->cropForm->setCropRect(preset.crop.rect);
    ui->cropForm->setCropFixed(preset.crop.fixed);
}

void MainWindow::openImage()
{
    QMimeDatabase mimeDb;
    QStringList filters;
    for (const auto& mimeTypeData : QImageReader::supportedMimeTypes()) {
        QMimeType mimeType = mimeDb.mimeTypeForName(mimeTypeData);
        filters += mimeType.globPatterns().join(';');
    }

    QFileDialog fileDialog;

    fileDialog.setWindowTitle(tr("Open image"));
    fileDialog.setNameFilter(tr("Images (%0)").arg(filters.join(';')));
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog.setFileMode(QFileDialog::ExistingFile);

    if (fileDialog.exec() != QDialog::Accepted)
        return;

    QStringList selectedFiles = fileDialog.selectedFiles();
    if (selectedFiles.isEmpty())
        return;

    loadImage(selectedFiles[0]);
}
void MainWindow::saveImage()
{
    if (!m_image.isNull()) {
        QImageWriter writer(m_path);
        if (!writer.write(ui->editor->generate().toImage())) {
            QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                tr("Cannot save %1: %2")
                    .arg(QDir::toNativeSeparators(m_path), writer.errorString()));
        } else {
            const QString message = tr("Saved \"%1\"").arg(QDir::toNativeSeparators(m_path));
            statusBar()->showMessage(message);
        }
    }
}
void MainWindow::saveImageFinish()
{
    QString path = m_path;
    m_path.insert(m_path.lastIndexOf('.'), "." + ui->watermarkForm->watermarkName() + ".finish");
    saveImage();
    m_path = path;
}
void MainWindow::saveImageAs()
{
    if (!m_image.isNull()) {
        const QString path = QFileDialog::getSaveFileName(this,
            tr("Open image"),
            QString(),
            "Images (*.jpg *.png *.gif);; Others (*)");
        if (!path.isEmpty()) {
            QImageWriter writer(path);
            if (!writer.write(ui->editor->generate().toImage())) {
                QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                    tr("Cannot save %1: %2")
                        .arg(QDir::toNativeSeparators(path), writer.errorString()));
            } else {
                const QString message = tr("Saved \"%1\"").arg(QDir::toNativeSeparators(path));
                statusBar()->showMessage(message);
            }
        }
    }
}

void MainWindow::loadPresets()
{
    PresetList presets = PresetManager::presets();
    for (auto preset : presets) {
        addPreset(preset);
    }
}
bool MainWindow::savePreset()
{
    QString name = QInputDialog::getText(this, tr("Preset Name"), "Preset name:");
    if (name.isNull()) {
        ui->statusBar->showMessage(tr("Invalid preset name"));
        return false;
    }

    Preset preset = this->preset(name);
    QString error;
    if (!PresetManager::addPreset(preset, &error)) {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
            tr("Cannot save preset %1: %2")
                .arg(QDir::toNativeSeparators(name), error));
        return false;
    } else {
        statusBar()->showMessage(tr("Preset saved: \"%1\"").arg(name));
        addPreset(preset);
        return true;
    }
}
void MainWindow::addPreset(const Preset& preset)
{
    auto actions = ui->menuSavedPreset->actions();
    for (auto action : actions) {
        if (action && action->text() == preset.name) {
            action->disconnect();
            connect(action, &QAction::triggered, [this, preset]() {
                setPreset(preset);
            });

            return;
        }
    }

    ui->menuSavedPreset->addAction(preset.name, [this, preset]() {
        setPreset(preset);
    });
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);

    m_rubberBand->setGeometry(ui->editor->rect());
}
void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasImage()) {
        event->acceptProposedAction();
        return;
    } else if (event->mimeData()->hasUrls()) {
        QList<QUrl> urls = event->mimeData()->urls();
        if (urls.size() != 1) {
            event->ignore();
            return;
        } else {
            QByteArray format = QImageReader::imageFormat(urls.first().toLocalFile());
            if (QImageReader::supportedImageFormats().contains(format)) {
                event->acceptProposedAction();
                return;
            }
        }
    }

    event->ignore();
}
void MainWindow::dragMoveEvent(QDragMoveEvent* event)
{
    if (event->answerRect().intersects(ui->editor->rect())) {
        m_rubberBand->show();
        event->accept();
    } else {
        m_rubberBand->hide();
        event->ignore();
    }
}
void MainWindow::dragLeaveEvent(QDragLeaveEvent* event)
{
    m_rubberBand->hide();
}
void MainWindow::dropEvent(QDropEvent* event)
{
    if (event->mimeData()->hasImage()) {
        QImage image = qvariant_cast<QImage>(event->mimeData()->imageData());
        m_zoomSlider->setEnabled(true);
        m_path = "unknown";
        setImage(QPixmap::fromImage(image));
        const QString message = tr("Image dragged, %1x%2, Depth: %3")
                                    .arg(image.width())
                                    .arg(image.height())
                                    .arg(image.depth());
        statusBar()->showMessage(message);
    } else {
        QUrl url = event->mimeData()->urls().first();
        QString path = url.toLocalFile();
        loadImage(path);
    }

    m_rubberBand->hide();
}

void MainWindow::initActions()
{
    ui->actionOpen->setShortcut(QKeySequence::Open);
    ui->actionSave->setShortcut(QKeySequence::Save);
    ui->actionSaveAs->setShortcut(QKeySequence::SaveAs);
    ui->actionSettings->setShortcut(QKeySequence::Preferences);
    ui->actionExit->setShortcut(QKeySequence::Quit);

    // At start no image is loaded, therefore we can not save "nothing",
    // so save actions are disabled until an image is loaded.
    ui->actionSave->setEnabled(false);
    ui->actionSaveFinish->setEnabled(false);
    ui->actionSaveAs->setEnabled(false);

    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::openImage);
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::saveImage);
    connect(ui->actionSaveFinish, &QAction::triggered, this, &MainWindow::saveImageFinish);
    connect(ui->actionSaveAs, &QAction::triggered, this, &MainWindow::saveImageAs);
    connect(ui->actionSavePreset, &QAction::triggered, this, QOverload<>::of(&MainWindow::savePreset));
    connect(ui->actionReportIssue, &QAction::triggered, [this]() { auto dialog = new ReportIssueDialog(this); dialog->open(); });
    connect(ui->actionExit, &QAction::triggered, []() { qApp->exit(); });

    connect(ui->actionAbout, &QAction::triggered, [this]() {
        QDialog dialog(this);
        Ui::AboutDialog ui;
        ui.setupUi(&dialog);

#ifdef Q_OS_UNIX
        // On platforms using X11 (unixes platforms), we do not use icons8.com
        // icons, but instead the native provided icons. Thus we do not need
        // and we should not add disclaimer for icons8.com.
        ui.icons8Label->hide();
#endif // Q_OS_UNIX

        ui.labelVersion->setText(QApplication::applicationVersion());
        ui.labelOS->setText(QSysInfo::prettyProductName());
        ui.labelQt->setText(qVersion());

        dialog.exec();
    });

    connect(ui->actionAboutQt, &QAction::triggered, [this]() {
        QApplication::aboutQt();
    });
}
void MainWindow::initSignals()
{
    connect(ui->watermarkForm, &WatermarkForm::watermarkImageChanged, ui->editor, &Editor::setWatermarkImage);
    connect(ui->watermarkForm, &WatermarkForm::watermarkEdited, ui->editor, &Editor::updateWatermarkEditor);
    connect(ui->cropForm, &CropForm::cropRectEdited, ui->editor, &Editor::updateEditors);

    connect(m_zoomSlider, &QSlider::valueChanged, [this](int value) {
        ui->editor->blockSignals(true);
        ui->editor->zoom(value / 100.0);
        ui->editor->blockSignals(false);
    });

    connect(ui->editor, &Editor::zoomChanged, [this](qreal factor) {
        m_zoomSlider->blockSignals(true);
        m_zoomSlider->setValue(qRound(factor * 100.0));
        m_zoomSlider->blockSignals(false);
    });
}
void MainWindow::initZoomWidget()
{
    QWidget* widget = new QWidget(ui->statusBar);
    QHBoxLayout* layout = new QHBoxLayout(widget);
    m_zoomSlider = new QSlider(Qt::Horizontal, widget);
    m_zoomSlider->setMinimum(qRound(Editor::kMinZoomFactor * 100));
    m_zoomSlider->setMaximum(qRound(Editor::kMaxZoomFactor * 100));
    m_zoomSlider->setValue(qRound(ui->editor->scaleFactor() * 100.0));
    m_zoomSlider->setEnabled(false);
    layout->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Expanding));
    layout->addWidget(m_zoomSlider);
    widget->setLayout(layout);
    ui->statusBar->addPermanentWidget(widget);
}
