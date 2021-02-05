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

#ifndef WATERMARKFORM_HPP
#define WATERMARKFORM_HPP

#include "watermark.hpp"
#include <QSettings>
#include <QWidget>

namespace Ui {
class WatermarkForm;
}

// ========================================================
// class WatermarkForm
// ========================================================

class WatermarkForm : public QWidget {
    Q_OBJECT

public:
    explicit WatermarkForm(QWidget* parent = nullptr);
    ~WatermarkForm();

    QString watermarkName() const;
    int watermarkIndex() const;
    WatermarkAnchor watermarkAnchor() const;
    qreal watermarkOpacity() const;
    bool watermarkUseSize() const;
    qreal watermarkSize() const;
    bool watermarkUseRotation() const;
    int watermarkRotation() const;
    bool watermarkUseColor() const;
    QColor watermarkColor() const;
    bool watermarkUseOffset() const;
    QPoint watermarkOffset() const;

public slots:
    void addWatermark();
    void addWatermark(const Watermark& watermark);
    void removeWatermark();

    void setWatermarkIndex(int index);
    void setWatermarkAnchor(WatermarkAnchor anchor);
    void setWatermarkOpacity(qreal alpha);
    void setWatermarkUseSize(bool use);
    void setWatermarkSize(qreal size);
    void setWatermarkUseRotation(bool use);
    void setWatermarkRotation(int angle);
    void setWatermarkUseColor(bool use);
    void setWatermarkColor(const QColor& color);
    void setWatermarkUseOffset(bool use);
    void setWatermarkOffset(const QPoint& offset);

signals:
    void watermarkImageChanged(const QPixmap& image);
    void watermarkEdited();

private slots:
    void pickColor();
    void updateColor(const QColor& color);

private:
    void initSignals();
    void loadWatermarks();

private:
    Ui::WatermarkForm* ui;
    QColor m_color;
}; // class WatermarkForm

#endif // WATERMARKFORM_HPP
