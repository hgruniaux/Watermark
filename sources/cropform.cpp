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

#include <QDebug>
#include "cropform.hpp"
#include "ui_cropform.h"

// ========================================================
// class CropForm
// ========================================================

CropForm::CropForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CropForm)
{
    m_image = QRect(0, 0, 0, 0);
    m_crop = QRect(0, 0, 0, 0);
    ui->setupUi(this);
    initSignals();
}

CropForm::~CropForm()
{
    delete ui;
}


bool CropForm::cropFixed() const
{
    return ui->fixedCheckBox->isChecked();
}

void CropForm::setImageRect(const QRect& rect)
{
    m_image = rect;
    ui->xEdit->setMinimum(rect.x());
    ui->xEdit->setMaximum(rect.x() - m_crop.width());
    ui->yEdit->setMinimum(rect.y());
    ui->yEdit->setMaximum(rect.y() - m_crop.height());
    ui->widthEdit->setMaximum(rect.width());
    ui->heightEdit->setMaximum(rect.height());
}
void CropForm::setCropRect(const QRect& crop)
{
    m_crop = crop;
    ui->xEdit->setMaximum(m_image.width() - crop.width());
    ui->xEdit->setValue(crop.x());
    ui->yEdit->setMaximum(m_image.height() - crop.height());
    ui->yEdit->setValue(crop.y());
    ui->widthEdit->setValue(crop.width());
    ui->heightEdit->setValue(crop.height());
}
void CropForm::setCropPosition(const QPoint& pos)
{
    setCropRect(QRect(pos, m_crop.size()));
}
void CropForm::setCropSize(const QSize& size)
{
    setCropRect(QRect(m_crop.topLeft(), size));
}
void CropForm::setCropFixed(bool fixed)
{
    ui->fixedCheckBox->setChecked(fixed);
}

void CropForm::initSignals()
{
    connect(ui->xEdit, QOverload<int>::of(&QSpinBox::valueChanged), [this](int x) { m_crop.moveTo(x, m_crop.y()); emit cropMoved(m_crop.topLeft()); });
    connect(ui->yEdit, QOverload<int>::of(&QSpinBox::valueChanged), [this](int y) { m_crop.moveTo(m_crop.x(), y); emit cropMoved(m_crop.topLeft()); });
    connect(ui->widthEdit, QOverload<int>::of(&QSpinBox::valueChanged), this, &CropForm::setWidth);
    connect(ui->heightEdit, QOverload<int>::of(&QSpinBox::valueChanged), this, &CropForm::setHeight);
    connect(ui->fixedCheckBox, &QCheckBox::toggled, [this]() { m_aspectRatio = double(m_crop.width()) / double(m_crop.height()); });
    connect(ui->comboBuiltinRatio, &QComboBox::currentTextChanged, this, &CropForm::setRatio);
    connect(this, &CropForm::cropResized, [this]() { emit cropEdited(m_crop);  });
    connect(this, &CropForm::cropMoved, [this]() { emit cropEdited(m_crop); });
    connect(this, &CropForm::cropEdited, [this]() { ui->comboBuiltinRatio->setCurrentIndex(0); });
}

void CropForm::setRatio(const QString& text)
{
    int index = text.indexOf(':');
    if(index < 0) return;
    int left = text.left(index).toInt();
    int right = text.mid(index+1).toInt();
    double ratio = double(left) / double(right);
    setCropSize(QSize(m_image.width(), m_image.height()/ratio));
}
void CropForm::setWidth(int w)
{
    if(ui->fixedCheckBox->isChecked()) {
        setCropSize(QSize(w, qRound(w / m_aspectRatio)));
    } else {
        setCropSize(QSize(w, m_crop.height()));
    }
    emit cropResized(m_crop.size());
}
void CropForm::setHeight(int h)
{
    if(ui->fixedCheckBox->isChecked()) {
        setCropSize(QSize(qRound(h * m_aspectRatio), h));
    } else {
        setCropSize(QSize(m_crop.width(), h));
    }
    emit cropResized(m_crop.size());
}
