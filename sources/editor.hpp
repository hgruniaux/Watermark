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

#ifndef EDITOR_HPP
#define EDITOR_HPP

#include <QLabel>
#include <QScrollArea>

#include "watermark.hpp"

class CropEditor;
class WatermarkEditor;

// ========================================================
// class Editor
// ========================================================

class Editor : public QScrollArea {
    Q_OBJECT

public:
    explicit Editor(QWidget* parent = nullptr);
    virtual ~Editor() override { }

    QPixmap generate() const;

    QSize imageSize() const { return m_image.size(); }
    QPoint mapTo(const QPoint& point) const;
    QRect mapFrom(const QRect& rect) const;
    QRectF mapFrom(const QRectF& rect) const;
    QPointF mapFrom(const QPointF& point) const;
    QSizeF mapFrom(const QSizeF& point) const;

    static constexpr qreal kMinZoomFactor = 0.333; // 33%
    static constexpr qreal kMaxZoomFactor = 3.0; // 300%

    qreal scaleFactor() const { return m_scaleFactor; }

public slots:
    void setImage(const QPixmap& image);

    void setCropRect(const QRect& rect);
    void setCropSize(const QSize& size);
    void setCropPosition(const QPoint& pos);

    void setWatermarkImage(const QPixmap& image);
    void setWatermarkAnchor(WatermarkAnchor anchor);
    void setWatermarkOpacity(qreal opacity);
    void setWatermarkUseSize(bool use);
    void setWatermarkSize(qreal size);
    void setWatermarkUseRotation(bool use);
    void setWatermarkRotation(int angle);
    void setWatermarkUseColor(bool use);
    void setWatermarkColor(const QColor& color);
    void setWatermarkUseOffset(bool use);
    void setWatermarkOffset(const QPoint& offset);

    void zoom(qreal factor);
    void zoomIn();
    void zoomOut();
    void normalSize();
    void fitToWindow(bool fit);

private:
    void scaleImage(qreal factor);
    void adjustScrollBar(QScrollBar* scrollBar, qreal factor);
    QPointF mapFactor() const;

private slots:
    void updateOverlaysPos();

signals:
    void edited();
    void cropResized(const QSize& size);
    void cropMoved(const QPoint& position);
    void cropEdited();

    void zoomChanged(qreal factor);

protected:
    virtual void wheelEvent(QWheelEvent* event) override;

private:
    // An ugly hack to allow wheel events to be handled by the Editor
    // class even if it is received by the overlays (which should always
    // be the case, because they are in front of the editor). Normally
    // the Qt event propagation should permit to fix this problem, but
    // i failed to get it to work properly.
    // TODO: Fix this hack, and find a better solution
    friend class CropEditor;
    friend class WatermarkEditor;
    void processWheelEvent(QWheelEvent* event);

    friend class MoveResizeEventFilter;
    void updateOverlaysSize();

private:
    QLabel* m_imageLabel;
    qreal m_scaleFactor = 1.0;
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
    explicit CropEditor(QWidget* parent = nullptr);
    virtual ~CropEditor() override { }

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
    virtual void wheelEvent(QWheelEvent* event) override;

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
    explicit WatermarkEditor(QWidget* parent = nullptr);
    virtual ~WatermarkEditor() override { }

    Editor* editor() const { return static_cast<Editor*>(parent()); }
    void drawWatermark(QPainter* painter, bool scaled = true);

public slots:
    void updatePosition();
    void setCrop(const QRect& crop);
    void setWatermark(const QPixmap& pixmap);
    void setColor(const QColor& color);
    void setUseColor(bool colorize);

protected:
    virtual void paintEvent(QPaintEvent* event) override;
    virtual void wheelEvent(QWheelEvent* event) override;

private:
    friend class Editor;

private:
    QRect m_crop;
    WatermarkAnchor m_anchor = AnchorCenter;
    QPoint m_pos;
    QPixmap m_watermark;
    QPixmap m_coloredWatermark;

    qreal m_opacity = 1.0;
    bool m_useSize = false;
    qreal m_size = 1.0;
    bool m_useRotation = false;
    int m_rotation = 0;
    QColor m_color = QColor(0, 0, 0, 128);
    bool m_useColor = false;
    QPoint m_offset;
    bool m_useOffset = false;
}; // class WatermarkEditor

#endif // EDITOR_HPP
