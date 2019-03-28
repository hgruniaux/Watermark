//
// MIT License
//
// Copyright (c) 2019 Hubert Gruniaux
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

#include <QFileDialog>
#include <QProxyStyle>
#include <QStyleOption>
#include <QInputDialog>
#include <QMessageBox>
#include <QPainter>
#include <QImageWriter>
#include <QImageReader>
#include <QFileInfo>

#include "aboutdialog.hpp"
#include "licensedialog.hpp"
#include "reportissuedialog.hpp"
#include "mainwindow.hpp"
#include "ui_mainwindow.h"

// ========================================================
// class IconnedDockStyle
// ========================================================

class IconnedDockStyle : public QProxyStyle {
public:
    void drawControl(ControlElement element, const QStyleOption* option,
        QPainter* painter, const QWidget* widget = 0) const override
    {
        if(element == QStyle::CE_DockWidgetTitle) {
            int width = pixelMetric(QStyle::PM_ToolBarIconSize);
            int margin = baseStyle()->pixelMetric(QStyle::PM_DockWidgetTitleMargin);
            QPoint iconPoint(margin + option->rect.left(), margin + option->rect.center().y() - width/2);

            painter->drawPixmap(iconPoint, widget->windowIcon().pixmap(width, width));
            const_cast<QStyleOption*>(option)->rect = option->rect.adjusted(width, 0, 0, 0);
        }
        baseStyle()->drawControl(element, option, painter, widget);
    }
}; // class IconnedDockStyle

// ========================================================
// class MainWindow
// ========================================================

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_style(new IconnedDockStyle)
{
    PresetManager::makeDirectory();
    WatermarkManager::makeDirectory();

    ui->setupUi(this);
    initActions();
    initZoomWidget();
    initSignals();

    loadPresets();
    setImage(QPixmap());

    ui->preview->setEditor(ui->editor);
    ui->menuSavedPreset->removeAction(ui->action_4);

    ui->dockWidgetCrop->setStyle(m_style);
    ui->dockWidgetPreview->setStyle(m_style);
    ui->dockWidgetWatermark->setStyle(m_style);
}
MainWindow::~MainWindow()
{
    delete ui;
    delete m_style;
}

QPixmap MainWindow::image() const
{
    return m_image;
}
QPixmap MainWindow::finalImage() const
{
    if(!m_image.isNull())
        return ui->editor->generate();
    else
        return QPixmap();
}
void MainWindow::setImage(const QPixmap& image)
{
    m_image = image;
    ui->editor->setImage(image);
    ui->cropForm->setImageRect(image.rect());
    ui->cropForm->setCropRect(image.rect());
}
void MainWindow::loadImage(const QString& path)
{
    QImageReader reader(path);
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
                .arg(QDir::toNativeSeparators(path)).arg(image.width()).arg(image.height()).arg(image.depth());
        statusBar()->showMessage(message);
    }
}

Preset MainWindow::preset(const QString& name) const
{
    Preset preset;
    preset.name = name;
    preset.watermark.originalSize = ui->watermarkForm->watermarkOriginalSize();
    preset.watermark.anchor = ui->watermarkForm->watermarkAnchor();
    preset.watermark.index = ui->watermarkForm->watermarkIndex();
    preset.watermark.size = qRound(ui->watermarkForm->watermarkSize()*100);
    preset.watermark.alpha = qRound(ui->watermarkForm->watermarkAlpha()*100);
    preset.watermark.color = ui->watermarkForm->watermarkColor();
    preset.crop.rect = ui->cropForm->cropRect();
    preset.crop.fixed = ui->cropForm->cropFixed();
    return preset;
}
void MainWindow::setPreset(const Preset& preset)
{
    ui->watermarkForm->setWatermarkOriginalSize(preset.watermark.originalSize);
    ui->watermarkForm->setWatermarkAlpha(preset.watermark.alpha/100.);
    ui->watermarkForm->setWatermarkSize(preset.watermark.size/100.);
    ui->watermarkForm->setWatermarkAnchor(static_cast<WatermarkAnchor>(preset.watermark.anchor));
    ui->watermarkForm->setWatermarkIndex(preset.watermark.index);
    ui->watermarkForm->setWatermarkColor(preset.watermark.color);
    ui->cropForm->setCropRect(preset.crop.rect);
    ui->cropForm->setCropFixed(preset.crop.fixed);
}

void MainWindow::openImage()
{
    const QString path = QFileDialog::getOpenFileName(this,
                                                      tr("Open image"),
                                                      QString(),
                                                      "Images (*.jpg *.png *.gif);; Others (*)");
    if(!path.isEmpty()) {
        loadImage(path);
    }
}
void MainWindow::saveImage()
{
    if(!m_image.isNull()) {
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
    if(!m_image.isNull()) {
        const QString path = QFileDialog::getSaveFileName(this,
                                                          tr("Open image"),
                                                          QString(),
                                                          "Images (*.jpg *.png *.gif);; Others (*)");
        if(!path.isEmpty()) {
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
    PresetList presets = PresetManager::getPresets();
    for(auto preset : presets) {
        addPreset(preset);
    }
}
bool MainWindow::savePreset()
{
    QString name = QInputDialog::getText(this, tr("Preset Name"), "Preset name:");
    if(name.isNull()) {
        ui->statusBar->showMessage(tr("Invalid preset name"));
        return false;
    }

    Preset preset = this->preset(name);
    QString error;
    if(!PresetManager::addPreset(preset, &error)) {
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
    for(auto action : actions) {
        if(action && action->text() == preset.name) {
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

void MainWindow::initActions()
{
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::openImage);
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::saveImage);
    connect(ui->actionSaveFinish, &QAction::triggered, this, &MainWindow::saveImageFinish);
    connect(ui->actionSaveAs, &QAction::triggered, this, &MainWindow::saveImageAs);
    connect(ui->actionSavePreset, &QAction::triggered, this, QOverload<>::of(&MainWindow::savePreset));
    connect(ui->actionLicense, &QAction::triggered, [this]() { auto dialog = new LicenseDialog(this); dialog->open(); });
    connect(ui->actionAbout, &QAction::triggered, [this]() { auto dialog = new AboutDialog(this); dialog->open(); });
    connect(ui->actionReportIssue, &QAction::triggered, [this]() { auto dialog = new ReportIssueDialog(this); dialog->open(); });
    connect(ui->actionExit, &QAction::triggered, []() { qApp->exit(); });
}
void MainWindow::initSignals()
{
    connect(ui->watermarkForm, &WatermarkForm::watermarkImageChanged, ui->editor, &Editor::setWatermarkImage);
    connect(ui->watermarkForm, &WatermarkForm::watermarkPositionChanged, ui->editor, &Editor::setWatermarkAnchor);
    connect(ui->watermarkForm, &WatermarkForm::watermarkAlphaChanged, ui->editor, &Editor::setWatermarkAlpha);
    connect(ui->watermarkForm, &WatermarkForm::watermarkSizeChanged, ui->editor, &Editor::setWatermarkSize);
    connect(ui->watermarkForm, &WatermarkForm::watermarkColorChanged, ui->editor, &Editor::setWatermarkColor);
    connect(ui->watermarkForm, &WatermarkForm::watermarkResizeToggled, ui->editor, &Editor::setWatermarkResize);
    connect(ui->watermarkForm, &WatermarkForm::watermarkColorizeToggled, ui->editor, &Editor::setWatermarkColorize);
    connect(ui->cropForm, &CropForm::cropResized, ui->editor, &Editor::setCropSize);
    connect(ui->cropForm, &CropForm::cropMoved, ui->editor, &Editor::setCropPosition);
    connect(ui->editor, &Editor::cropResized, ui->cropForm, &CropForm::setCropSize);
    connect(ui->editor, &Editor::cropMoved, ui->cropForm, &CropForm::setCropPosition);
    connect(ui->editor, &Editor::edited, ui->preview, QOverload<>::of(&QWidget::update));
    connect(m_zoomSlider, &QSlider::valueChanged, [this](int value) { qreal val = value/100.; ui->editor->zoom(val); });
}
void MainWindow::initZoomWidget()
{
    QWidget* widget = new QWidget(ui->statusBar);
    QHBoxLayout* layout = new QHBoxLayout(widget);
    m_zoomSlider = new QSlider(Qt::Horizontal, widget);
    m_zoomSlider->setMinimum(20);
    m_zoomSlider->setMaximum(200);
    m_zoomSlider->setValue(100);
    m_zoomSlider->setEnabled(false);
    layout->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Expanding));
    layout->addWidget(m_zoomSlider);
    widget->setLayout(layout);
    ui->statusBar->addPermanentWidget(widget);
}
