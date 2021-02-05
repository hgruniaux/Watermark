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

class CropForm;
class WatermarkForm;

// ========================================================
// class Editor
// ========================================================

class Editor : public QScrollArea {
    Q_OBJECT

public:
    Editor(QWidget* parent = nullptr);

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

    void setCropForm(CropForm* form);
    void setWatermarkForm(WatermarkForm* form);

    // Requests an update of the both crop editor and watermark editor.
    // Call this function only if the crop rectangle was updated.
    void updateEditors();
    // Requests an update only for the watermark editor. Call this function
    // if the crop editor is not needed to be updated (for example if the
    // crop rectangle was not updated).
    void updateWatermarkEditor();

    void setWatermarkImage(const QPixmap& image);

    void zoom(qreal factor);
    void zoomIn();
    void zoomOut();
    void normalSize();
    void fitToWindow(bool fit);

signals:
    void zoomChanged(qreal factor);

private:
    void scaleImage(qreal factor);
    void adjustScrollBar(QScrollBar* scrollBar, qreal factor);
    QPointF mapFactor() const;

private slots:
    void updateOverlaysPos();

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
    CropEditor* m_cropEditor;
    WatermarkEditor* m_watermarkEditor;
}; // class Editor

// ========================================================
// class CropEditor
// ========================================================

class CropEditor : public QWidget {
    Q_OBJECT

public:
    CropEditor(Editor* parent);

    inline Editor* editor() const { return static_cast<Editor*>(parent()); }

    inline CropForm* cropForm() const { return m_cropForm; }
    inline void setCropForm(CropForm* form) { m_cropForm = form; }

protected:
    virtual void paintEvent(QPaintEvent* event) override;
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void wheelEvent(QWheelEvent* event) override;

private:
    WatermarkAnchor resizeCorner(const QPoint& pos);

private:
    CropForm* m_cropForm;
    QPoint m_dragOrigin;
    WatermarkAnchor m_resizingCorner;
    bool m_isDragging;
    bool m_isResizing;
}; // class CropEditor

// ========================================================
// class WatermarkEditor
// ========================================================

class WatermarkEditor : public QWidget {
    Q_OBJECT

public:
    WatermarkEditor(Editor* parent);

    inline Editor* editor() const { return static_cast<Editor*>(parent()); }

    inline void setCropForm(CropForm* form) { m_cropForm = form; }
    inline void setWatermarkForm(WatermarkForm* form) { m_watermarkForm = form; }

    void drawWatermark(QPainter* painter, bool scaled = true);

public slots:
    void updatePosition();
    void setWatermark(const QPixmap& pixmap);

protected:
    virtual void paintEvent(QPaintEvent* event) override;
    virtual void wheelEvent(QWheelEvent* event) override;

private:
    void computeColoredWatermark(const QColor& color);

private:
    CropForm* m_cropForm;
    WatermarkForm* m_watermarkForm;

    QPoint m_cachedWatermarkPos;
    QPixmap m_watermark;

    bool m_hasCachedColoredWatermark;
    QColor m_cachedWatermarkColor;
    QPixmap m_coloredWatermark;
}; // class WatermarkEditor

#endif // EDITOR_HPP
