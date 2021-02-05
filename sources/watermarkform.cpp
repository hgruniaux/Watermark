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

#include "watermarkform.hpp"
#include "ui_watermarkform.h"
#include <QColorDialog>
#include <QDataStream>
#include <QFileDialog>
#include <QFileInfo>
#include <QImageReader>

// ========================================================
// class WatermarkForm
// ========================================================

static constexpr int kPixmapDataRole = 1256;
static constexpr int kTextDataRole = 1257;

WatermarkForm::WatermarkForm(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::WatermarkForm)
{
    ui->setupUi(this);

    ui->xOffsetSpinBox->setMinimum(INT_MIN);
    ui->xOffsetSpinBox->setMaximum(INT_MAX);
    ui->yOffsetSpinBox->setMinimum(INT_MIN);
    ui->yOffsetSpinBox->setMaximum(INT_MAX);

    setWatermarkAnchor(AnchorCenter);

    initSignals();
    loadWatermarks();
    updateColor(QColor());
}
WatermarkForm::~WatermarkForm()
{
    delete ui;
}

void WatermarkForm::addWatermark()
{
    const QString path = QFileDialog::getOpenFileName(this,
        tr("Open image"),
        QString(),
        "Images (*.jpg *.png *.gif);; Others (*)");
    if (!path.isEmpty()) {
        QFileInfo info = QFileInfo(path);
        QString name = info.baseName();
        QImageReader reader(path);
        const QImage image = reader.read();
        if (!image.isNull()) {
            const QPixmap pix = QPixmap::fromImage(image);
            Watermark watermark;
            watermark.name = name;
            watermark.image = pix;
            WatermarkManager::addWatermark(watermark);
            addWatermark(watermark);
        }
    }
}
void WatermarkForm::addWatermark(const Watermark& watermark)
{
    QListWidgetItem* item = new QListWidgetItem(ui->listWidget);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    item->setIcon(watermark.image.scaled(QSize(48, 48), Qt::KeepAspectRatio));
    item->setData(kPixmapDataRole, watermark.image);
    item->setData(kTextDataRole, watermark.name);
    item->setText(watermark.name);
    item->setToolTip(watermark.name);
}
void WatermarkForm::removeWatermark()
{
    if (ui->listWidget->currentRow() > 0) {
        QListWidgetItem* item = ui->listWidget->currentItem();
        if (item) {
            QString name = item->text();
            QPixmap pixmap = item->data(kPixmapDataRole).value<QPixmap>();
            ui->listWidget->removeItemWidget(item);
            Watermark watermark;
            watermark.name = name;
            watermark.image = pixmap;
            WatermarkManager::removeWatermark(watermark);
            delete item;
        }
    }
}

QString WatermarkForm::watermarkName() const
{
    if (ui->listWidget->currentRow() > 0) {
        return ui->listWidget->currentItem()->text();
    } else {
        return QString();
    }
}
int WatermarkForm::watermarkIndex() const
{
    return ui->listWidget->currentRow();
}
WatermarkAnchor WatermarkForm::watermarkAnchor() const
{
    return static_cast<WatermarkAnchor>(ui->comboPosition->currentIndex());
}
qreal WatermarkForm::watermarkOpacity() const
{
    return ui->opacitySpinBox->value() / 100.f;
}
bool WatermarkForm::watermarkUseSize() const
{
    return ui->sizeGroupBox->isChecked();
}
qreal WatermarkForm::watermarkSize() const
{
    return ui->sizeSpinBox->value() / 100.f;
}
bool WatermarkForm::watermarkUseRotation() const
{
    return ui->rotationGroupBox->isChecked();
}
int WatermarkForm::watermarkRotation() const
{
    return 359 - ui->rotationSpinBox->value();
}
bool WatermarkForm::watermarkUseColor() const
{
    return ui->colorGroupBox->isChecked();
}
QColor WatermarkForm::watermarkColor() const
{
    return m_color;
}
bool WatermarkForm::watermarkUseOffset() const
{
    return ui->offsetGroupBox->isChecked();
}
QPoint WatermarkForm::watermarkOffset() const
{
    return QPoint(
        ui->xOffsetSpinBox->value(),
        ui->yOffsetSpinBox->value());
}

void WatermarkForm::setWatermarkIndex(int index)
{
    ui->listWidget->setCurrentRow(qBound(0, index, ui->listWidget->count() - 1));
}
void WatermarkForm::setWatermarkOpacity(qreal alpha)
{
    ui->opacitySpinBox->setValue(qBound(0, qRound(alpha * 100), 100));
}
void WatermarkForm::setWatermarkUseSize(bool use)
{
    ui->sizeGroupBox->setChecked(use);
}
void WatermarkForm::setWatermarkSize(qreal size)
{
    ui->sizeSpinBox->setValue(qBound(0, qRound(size * 100), 100));
}
void WatermarkForm::setWatermarkAnchor(WatermarkAnchor anchor)
{
    ui->comboPosition->setCurrentIndex(static_cast<int>(anchor));
    switch (anchor) {
    case AnchorTopLeft:
        ui->radioTopLeft->setChecked(true);
        break;
    case AnchorTop:
        ui->radioTop->setChecked(true);
        break;
    case AnchorTopRight:
        ui->radioTopRight->setChecked(true);
        break;
    case AnchorLeft:
        ui->radioLeft->setChecked(true);
        break;
    case AnchorCenter:
        ui->radioCenter->setChecked(true);
        break;
    case AnchorRight:
        ui->radioRight->setChecked(true);
        break;
    case AnchorBottomLeft:
        ui->radioBottomLeft->setChecked(true);
        break;
    case AnchorBottom:
        ui->radioBottom->setChecked(true);
        break;
    case AnchorBottomRight:
        ui->radioBottomRight->setChecked(true);
        break;
    case AnchorRepeated:
        ui->radioRepeated->setChecked(true);
        break;
    default:
        break;
    }
}

void WatermarkForm::setWatermarkUseRotation(bool use)
{
    ui->rotationGroupBox->setChecked(use);
}
void WatermarkForm::setWatermarkRotation(int angle)
{
    ui->rotationSpinBox->setValue(qBound(0, qAbs(angle) % 360, 360));
}
void WatermarkForm::setWatermarkUseColor(bool use)
{
    ui->colorGroupBox->setChecked(use);
}
void WatermarkForm::setWatermarkColor(const QColor& color)
{
    updateColor(color);
    ui->colorOpacitySpinBox->setValue(qBound(0, qRound(color.alphaF() * 100), 100));
}
void WatermarkForm::setWatermarkUseOffset(bool use)
{
    ui->offsetGroupBox->setChecked(use);
}
void WatermarkForm::setWatermarkOffset(const QPoint& offset)
{
    ui->xOffsetSpinBox->setValue(offset.x());
    ui->yOffsetSpinBox->setValue(offset.y());
}

void WatermarkForm::pickColor()
{
    QColorDialog* dialog = new QColorDialog(this);
    dialog->setCurrentColor(m_color);
    connect(dialog, &QColorDialog::currentColorChanged, this, &WatermarkForm::updateColor);
    dialog->open();
}
void WatermarkForm::updateColor(const QColor& color)
{
    QColor c = color;
    c.setAlphaF(ui->colorOpacitySlider->value() / 100.);
    m_color = c;
    ui->colorPushButton->setStyleSheet(QString("background: %0").arg(c.name(QColor::HexRgb)));
    emit watermarkEdited();
}

void WatermarkForm::initSignals()
{
    connect(ui->listWidget, &QListWidget::currentItemChanged, [this](QListWidgetItem* item, QListWidgetItem*) {
        ui->buttonRemove->setEnabled(ui->listWidget->currentRow() > 0);
        QPixmap image = item->data(kPixmapDataRole).value<QPixmap>();
        emit watermarkImageChanged(image);
    });
    connect(ui->listWidget, &QListWidget::itemChanged, [this](QListWidgetItem* item) {
        QString name = item->data(kTextDataRole).toString();
        Watermark before;
        before.name = name;
        before.image = item->data(kPixmapDataRole).value<QPixmap>();
        if (name != item->text() && !name.isEmpty() && !item->text().isEmpty()) {
            Watermark after;
            after.name = item->text();
            after.image = before.image;
            WatermarkManager::replaceWatermark(before, after);
            item->setData(kTextDataRole, item->text());
        }
    });

    // "Opacity" category
    connect(ui->opacitySlider, &QSlider::valueChanged, ui->opacitySpinBox, &QSpinBox::setValue);
    connect(ui->opacitySpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) {
        ui->opacitySlider->setValue(value);
        emit watermarkEdited();
    });

    // "Size" category
    connect(ui->sizeGroupBox, &QGroupBox::toggled, [this](bool use) {
        emit watermarkEdited();
    });
    connect(ui->sizeSlider, &QSlider::valueChanged, ui->sizeSpinBox, &QSpinBox::setValue);
    connect(ui->sizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) {
        ui->sizeSlider->setValue(value);
        emit watermarkEdited();
    });

    // "Rotation" category
    connect(ui->rotationGroupBox, &QGroupBox::toggled, [this](bool use) {
        emit watermarkEdited();
    });
    connect(ui->rotationDial, &QDial::valueChanged, [this](int value) {
        ui->rotationSpinBox->blockSignals(true);
        if (value < 270) {
            ui->rotationSpinBox->setValue(359 - (90 + qAbs(value)));
        } else {
            ui->rotationSpinBox->setValue(359 - (value - 270));
        }
        ui->rotationSpinBox->blockSignals(false);
        emit watermarkEdited();
    });
    connect(ui->rotationSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) {
        ui->rotationDial->blockSignals(true);
        ui->rotationDial->setValue((359 - value) + 270);
        ui->rotationDial->blockSignals(false);
        emit watermarkEdited();
    });

    // "Color" category
    connect(ui->colorGroupBox, &QGroupBox::toggled, [this]() { emit WatermarkForm::watermarkEdited(); });
    connect(ui->colorOpacitySlider, &QSlider::valueChanged, ui->colorOpacitySpinBox, &QSpinBox::setValue);
    connect(ui->colorOpacitySpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) {
        ui->colorOpacitySlider->setValue(value);
        updateColor(m_color);
    });
    connect(ui->colorPushButton, &QPushButton::clicked, this, &WatermarkForm::pickColor);

    // "Offset" category
    connect(ui->offsetGroupBox, &QGroupBox::toggled, [this]() { emit WatermarkForm::watermarkEdited(); });
    connect(ui->xOffsetSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this]() { emit WatermarkForm::watermarkEdited(); });
    connect(ui->yOffsetSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this]() { emit WatermarkForm::watermarkEdited(); });

    connect(ui->buttonAdd, &QPushButton::clicked, this, QOverload<>::of(&WatermarkForm::addWatermark));
    connect(ui->buttonRemove, &QPushButton::clicked, this, &WatermarkForm::removeWatermark);

    connect(ui->radioTopLeft, &QRadioButton::toggled, [this](bool c) { if(c) ui->comboPosition->setCurrentIndex(AnchorTopLeft); });
    connect(ui->radioTop, &QRadioButton::toggled, [this](bool c) { if(c) ui->comboPosition->setCurrentIndex(AnchorTop); });
    connect(ui->radioTopRight, &QRadioButton::toggled, [this](bool c) { if(c) ui->comboPosition->setCurrentIndex(AnchorTopRight);; });
    connect(ui->radioLeft, &QRadioButton::toggled, [this](bool c) { if(c) ui->comboPosition->setCurrentIndex(AnchorLeft); });
    connect(ui->radioCenter, &QRadioButton::toggled, [this](bool c) { if(c) ui->comboPosition->setCurrentIndex(AnchorCenter); });
    connect(ui->radioRight, &QRadioButton::toggled, [this](bool c) { if(c) ui->comboPosition->setCurrentIndex(AnchorRight); });
    connect(ui->radioBottomLeft, &QRadioButton::toggled, [this](bool c) { if(c) ui->comboPosition->setCurrentIndex(AnchorBottomLeft); });
    connect(ui->radioBottom, &QRadioButton::toggled, [this](bool c) { if(c) ui->comboPosition->setCurrentIndex(AnchorBottom); });
    connect(ui->radioBottomRight, &QRadioButton::toggled, [this](bool c) { if(c) ui->comboPosition->setCurrentIndex(AnchorBottomRight); });
    connect(ui->radioRepeated, &QRadioButton::toggled, [this](bool c) { if(c) ui->comboPosition->setCurrentIndex(AnchorRepeated); });

    connect(ui->comboPosition, QOverload<int>::of(&QComboBox::currentIndexChanged), [this]() {
        setWatermarkAnchor((WatermarkAnchor)ui->comboPosition->currentIndex());
        emit WatermarkForm::watermarkEdited();
    });
}
void WatermarkForm::loadWatermarks()
{
    WatermarkList watermarks = WatermarkManager::watermarks();
    for (auto watermark : watermarks) {
        addWatermark(watermark);
    }
}
