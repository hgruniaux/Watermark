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

#ifndef EDITOR_HPP
#define EDITOR_HPP

#include <QWidget>

#include "watermark.hpp"

class CropEditor;
class WatermarkEditor;

// ========================================================
// class Editor
// ========================================================

class Editor : public QWidget {
    Q_OBJECT

public:
    explicit Editor(QWidget *parent = nullptr);
    virtual ~Editor() override {}

    QPixmap generate() const;

    QPointF zoomFactor() const;
    QSize imageSize() const { return m_image.size(); }
    QRect mapTo(const QRect& rect) const;
    QPoint mapTo(const QPoint& point) const;
    QSize mapTo(const QSize& size) const;
    QRect mapFrom(const QRect& rect) const;
    QPoint mapFrom(const QPoint& point) const;
    QSize mapFrom(const QSize& size) const;

public slots:
    void setImage(const QPixmap& image);

    void setCropRect(const QRect& rect);
    void setCropSize(const QSize& size);
    void setCropPosition(const QPoint& pos);

    void setWatermarkImage(const QPixmap& image);
    void setWatermarkAnchor(WatermarkAnchor anchor);
    void setWatermarkAlpha(qreal alpha);
    void setWatermarkSize(qreal size);
    void setWatermarkMargin(qreal margin);
    void setWatermarkRotation(int angle);
    void setWatermarkColor(const QColor& color);
    void setWatermarkResize(bool resize);
    void setWatermarkRotate(bool rotate);
    void setWatermarkColorize(bool colorize);

    void zoom(qreal factor);

signals:
    void edited();
    void cropResized(const QSize& size);
    void cropMoved(const QPoint& position);
    void cropEdited();

protected:
    virtual void resizeEvent(QResizeEvent*) override;
    virtual void paintEvent(QPaintEvent*) override;

private:
    void updateEditors();

private:
    QPointF m_factor;
    qreal m_zoom;
    QRect m_imageRect;
    QPixmap m_image;
    CropEditor* m_croppingPreview;
    WatermarkEditor* m_watermarkPreview;
}; // class Editor

// ========================================================
// class CropEditor
// ========================================================

class CropEditor : public QWidget {
    Q_OBJECT
public:
    explicit CropEditor(QWidget *parent = nullptr);
    virtual ~CropEditor() override {}

    Editor* editor() const { return static_cast<Editor*>(parent()); }

signals:
    void cropResized(const QSize& size);
    void cropMoved(const QPoint& position);
    void cropEdited();

protected:
    virtual void paintEvent(QPaintEvent* event) override;
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;

private:
    WatermarkAnchor resizeCorner(const QPoint& pos);

private:
    friend class Editor;

private:
    bool m_dragging;
    bool m_resizing;
    QRect m_crop;
    QPoint m_dragOrigin;
    WatermarkAnchor m_resizingCorner;
}; // class CropEditor

// ========================================================
// class WatermarkEditor
// ========================================================

class WatermarkEditor : public QWidget {
    Q_OBJECT
public:
    explicit WatermarkEditor(QWidget *parent = nullptr);
    virtual ~WatermarkEditor() override {}

    Editor* editor() const { return static_cast<Editor*>(parent()); }
    void drawWatermark(QPainter* painter, bool scaled = true);

public slots:
    void updatePosition();
    void setCrop(const QRect& crop);
    void setWatermark(const QPixmap& pixmap);
    void setColor(const QColor& color);
    void setColorize(bool colorize);

protected:
    virtual void paintEvent(QPaintEvent* event) override;

private:
    friend class Editor;

    QMargins margins(const QSize& watermarkSize) const;

private:
    QRect m_crop;
    bool m_resize;
    qreal m_alpha;
    qreal m_size;
    qreal m_margin;
    bool m_rotate;
    int m_rotation;
    WatermarkAnchor m_anchor;
    QPoint m_pos;
    QPixmap m_watermark;
    QPixmap m_coloredWatermark;
    QColor m_color;
    bool m_colorize;
}; // class WatermarkEditor

#endif // EDITOR_HPP
