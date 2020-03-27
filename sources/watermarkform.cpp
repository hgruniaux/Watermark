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

#include <QFileInfo>
#include <QFileDialog>
#include <QColorDialog>
#include <QDataStream>
#include <QImageReader>
#include "watermarkform.hpp"
#include "ui_watermarkform.h"

// ========================================================
// class WatermarkForm
// ========================================================

static constexpr int kPixmapDataRole = 1256;
static constexpr int kTextDataRole = 1257;

WatermarkForm::WatermarkForm(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::WatermarkForm)
{
    ui->setupUi(this);
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
    }
    else {
        return QString();
    }
}
bool WatermarkForm::watermarkOriginalSize() const
{
    return ui->checkOriginalSize->isChecked();
}
bool WatermarkForm::watermarkOriginalColor() const
{
    return ui->checkOriginalColor->isChecked();
}
int WatermarkForm::watermarkIndex() const
{
    return ui->listWidget->currentRow();
}
WatermarkAnchor WatermarkForm::watermarkAnchor() const
{
    return static_cast<WatermarkAnchor>(ui->comboPosition->currentIndex());
}
qreal WatermarkForm::watermarkSize() const
{
    return ui->spinSize->value() / 100.f;
}
qreal WatermarkForm::watermarkMargin() const
{
    return ui->spinMargin->value() / 100.f;
}
int WatermarkForm::watermarkRotation() const
{
    return ui->spinRotation->value();
}
qreal WatermarkForm::watermarkAlpha() const
{
    return ui->spinOpacity->value() / 100.f;
}
QColor WatermarkForm::watermarkColor() const
{
    return m_color;
}

void WatermarkForm::setWatermarkOriginalSize(bool original)
{
    ui->checkOriginalSize->setChecked(original);
}
void WatermarkForm::setWatermarkOriginalColor(bool original)
{
    ui->checkOriginalColor->setChecked(original);
}
void WatermarkForm::setWatermarkIndex(int index)
{
    ui->listWidget->setCurrentRow(qBound(0, index, ui->listWidget->count() - 1));
}
void WatermarkForm::setWatermarkSize(qreal size)
{
    ui->spinSize->setValue(qBound(0, qRound(size * 100), 100));
}
void WatermarkForm::setWatermarkMargin(qreal margin)
{
    ui->spinMargin->setValue(qBound(0, qRound(margin * 100), 100));
}
void WatermarkForm::setWatermarkRotation(int angle)
{
    ui->spinRotation->setValue(qBound(0, qAbs(angle) % 360, 360));
}
void WatermarkForm::setWatermarkAlpha(qreal alpha)
{
    ui->spinOpacity->setValue(qBound(0, qRound(alpha * 100), 100));
}
void WatermarkForm::setWatermarkColor(const QColor& color)
{
    updateColor(color);
    ui->spinColorOpacity->setValue(qBound(0, qRound(color.alphaF() * 100), 100));
}
void WatermarkForm::setWatermarkAnchor(WatermarkAnchor anchor)
{
    ui->comboPosition->setCurrentIndex(static_cast<int>(anchor));
    switch (anchor) {
    case AnchorTopLeft: ui->radioTopLeft->setChecked(true); break;
    case AnchorTop: ui->radioTop->setChecked(true); break;
    case AnchorTopRight: ui->radioTopRight->setChecked(true); break;
    case AnchorLeft: ui->radioLeft->setChecked(true); break;
    case AnchorCenter: ui->radioCenter->setChecked(true); break;
    case AnchorRight: ui->radioRight->setChecked(true); break;
    case AnchorBottomLeft: ui->radioBottomLeft->setChecked(true); break;
    case AnchorBottom: ui->radioBottom->setChecked(true); break;
    case AnchorBottomRight: ui->radioBottomRight->setChecked(true); break;
    case AnchorRepeated: ui->radioRepeated->setChecked(true); break;
    default: break;
    }
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
    c.setAlphaF(ui->spinColorOpacity->value() / 100.);
    m_color = c;
    ui->buttonPickColor->setStyleSheet(QString("background: %0").arg(c.name(QColor::HexRgb)));
    emit watermarkColorChanged(c);
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

    connect(ui->sliderOpacity, &QSlider::valueChanged, ui->spinOpacity, &QSpinBox::setValue);
    connect(ui->spinOpacity, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) { ui->sliderOpacity->setValue(value); emit watermarkAlphaChanged(value / 100.); });

    connect(ui->sliderSize, &QSlider::valueChanged, ui->spinSize, &QSpinBox::setValue);
    connect(ui->spinSize, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) { ui->sliderSize->setValue(value); emit watermarkSizeChanged(value / 100.); });

    connect(ui->sliderMargin, &QSlider::valueChanged, ui->spinMargin, &QSpinBox::setValue);
    connect(ui->spinMargin, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) { ui->sliderMargin->setValue(value); emit watermarkMarginChanged(value / 100.); });

    connect(ui->dialRotation, &QDial::valueChanged, [this](int value) {
        ui->spinRotation->blockSignals(true);
        qDebug("%d", value);
        if (value < 270) {
            ui->spinRotation->setValue(359 - (90 + qAbs(value)));
        }
        else {
            ui->spinRotation->setValue(359 - (value - 270));
        }
        ui->spinRotation->blockSignals(false);
        emit watermarkRotationChanged(359 - ui->spinRotation->value());
        });
    connect(ui->spinRotation, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) {
        ui->dialRotation->blockSignals(true);
        ui->dialRotation->setValue((359 - value) + 270);
        ui->dialRotation->blockSignals(false);
        emit watermarkRotationChanged(359 - value);
        });

    connect(ui->sliderColorOpacity, &QSlider::valueChanged, ui->spinColorOpacity, &QSpinBox::setValue);
    connect(ui->spinColorOpacity, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) { ui->sliderColorOpacity->setValue(value); updateColor(m_color); });
    connect(ui->buttonPickColor, &QPushButton::clicked, this, &WatermarkForm::pickColor);

    connect(ui->checkOriginalSize, &QCheckBox::toggled, [this](bool v) { emit watermarkResizeToggled(!v); });
    connect(ui->checkOriginalSize, &QCheckBox::toggled, [this](bool v) { ui->sliderSize->setEnabled(!v); ui->spinSize->setEnabled(!v); });

    connect(ui->checkOriginalRotation, &QCheckBox::toggled, [this](bool v) { emit watermarkRotateToggled(!v); });
    connect(ui->checkOriginalRotation, &QCheckBox::toggled, [this](bool v) { ui->dialRotation->setEnabled(!v); ui->spinRotation->setEnabled(!v); });

    connect(ui->checkOriginalColor, &QCheckBox::toggled, [this](bool v) { emit watermarkColorizeToggled(!v); });
    connect(ui->checkOriginalColor, &QCheckBox::toggled, [this](bool v) { ui->sliderColorOpacity->setEnabled(!v); ui->spinColorOpacity->setEnabled(!v); ui->buttonPickColor->setEnabled(!v); });
    connect(ui->checkOriginalColor, &QCheckBox::toggled, [this](bool v) { if (!v) updateColor(m_color); });

    connect(ui->buttonAdd, &QPushButton::clicked, this, QOverload<>::of(&WatermarkForm::addWatermark));
    connect(ui->buttonRemove, &QPushButton::clicked, this, &WatermarkForm::removeWatermark);

    connect(ui->comboPosition, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int i) { emit watermarkPositionChanged(static_cast<WatermarkAnchor>(i)); });
    connect(ui->radioTopLeft, &QRadioButton::toggled, [this](bool c) { if (c) emit watermarkPositionChanged(AnchorTopLeft); });
    connect(ui->radioTop, &QRadioButton::toggled, [this](bool c) { if (c) emit watermarkPositionChanged(AnchorTop); });
    connect(ui->radioTopRight, &QRadioButton::toggled, [this](bool c) { if (c) emit watermarkPositionChanged(AnchorTopRight); });
    connect(ui->radioLeft, &QRadioButton::toggled, [this](bool c) { if (c) emit watermarkPositionChanged(AnchorLeft); });
    connect(ui->radioCenter, &QRadioButton::toggled, [this](bool c) { if (c) emit watermarkPositionChanged(AnchorCenter); });
    connect(ui->radioRight, &QRadioButton::toggled, [this](bool c) { if (c) emit watermarkPositionChanged(AnchorRight); });
    connect(ui->radioBottomLeft, &QRadioButton::toggled, [this](bool c) { if (c) emit watermarkPositionChanged(AnchorBottomLeft); });
    connect(ui->radioBottom, &QRadioButton::toggled, [this](bool c) { if (c) emit watermarkPositionChanged(AnchorBottom); });
    connect(ui->radioBottomRight, &QRadioButton::toggled, [this](bool c) { if (c) emit watermarkPositionChanged(AnchorBottomRight); });
    connect(ui->radioRepeated, &QRadioButton::toggled, [this](bool c) { if (c) emit watermarkPositionChanged(AnchorRepeated); });
    connect(this, &WatermarkForm::watermarkPositionChanged, [this](WatermarkAnchor anchor) { setWatermarkAnchor(anchor); });
}
void WatermarkForm::loadWatermarks()
{
    WatermarkList watermarks = WatermarkManager::watermarks();
    for (auto watermark : watermarks) {
        addWatermark(watermark);
    }
}
