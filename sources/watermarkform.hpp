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

#ifndef WATERMARKFORM_HPP
#define WATERMARKFORM_HPP

#include <QWidget>
#include <QSettings>
#include "watermark.hpp"

namespace Ui {
class WatermarkForm;
}

// ========================================================
// class WatermarkForm
// ========================================================

class WatermarkForm : public QWidget {
    Q_OBJECT

public:
    explicit WatermarkForm(QWidget *parent = nullptr);
    ~WatermarkForm();

    QString watermarkName() const;
    bool watermarkOriginalSize() const;
    bool watermarkOriginalColor() const;
    int watermarkIndex() const;
    WatermarkAnchor watermarkAnchor() const;
    qreal watermarkSize() const;
    qreal watermarkMargin() const;
    int watermarkRotation() const;
    qreal watermarkAlpha() const;
    QColor watermarkColor() const;

public slots:
    void addWatermark();
    void addWatermark(const Watermark& watermark);
    void removeWatermark();

    void setWatermarkOriginalSize(bool original);
    void setWatermarkOriginalColor(bool original);
    void setWatermarkIndex(int index);
    void setWatermarkAnchor(WatermarkAnchor anchor);
    void setWatermarkSize(qreal size);
    void setWatermarkMargin(qreal margin);
    void setWatermarkRotation(int angle);
    void setWatermarkColor(const QColor& color);
    void setWatermarkAlpha(qreal alpha);

signals:
    void watermarkImageChanged(const QPixmap& image);
    void watermarkPositionChanged(WatermarkAnchor pos);
    void watermarkAlphaChanged(qreal opacity);
    void watermarkSizeChanged(qreal size);
    void watermarkMarginChanged(qreal margin);
    void watermarkRotationChanged(int angle);
    void watermarkColorChanged(const QColor& color);
    void watermarkResizeToggled(bool resize);
    void watermarkRotateToggled(bool rotate);
    void watermarkColorizeToggled(bool colorize);

private slots:
    void pickColor();
    void updateColor(const QColor& color);

private:
    void initSignals();
    void loadWatermarks();

private:
    Ui::WatermarkForm *ui;
    QColor m_color;
}; // class WatermarkForm

#endif // WATERMARKFORM_HPP
