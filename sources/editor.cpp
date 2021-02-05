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

#include <QDebug>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QScrollBar>
#include <QtMath>

#include <QDebug>

#include "cropform.hpp"
#include "editor.hpp"
#include "watermarkform.hpp"

class MoveResizeEventFilter : public QObject {
public:
    MoveResizeEventFilter(QObject* parent)
        : QObject(parent)
    {
    }

protected:
    bool eventFilter(QObject* obj, QEvent* event) override
    {
        switch (event->type()) {
        case QEvent::Resize:
            static_cast<Editor*>(parent())->updateOverlaysSize();
            break;
        case QEvent::Move:
            static_cast<Editor*>(parent())->updateOverlaysPos();
            break;
        default:
            break;
        }

        return QObject::eventFilter(obj, event);
    }
};

// ========================================================
// class Editor
// ========================================================

Editor::Editor(QWidget* parent)
    : QScrollArea(parent)
    , m_imageLabel(new QLabel(this))
{
    m_imageLabel->installEventFilter(new MoveResizeEventFilter(this));

    m_imageLabel->setBackgroundRole(QPalette::Base);
    m_imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    m_imageLabel->setScaledContents(true);

    setBackgroundRole(QPalette::Dark);
    setAlignment(Qt::AlignCenter);
    setWidget(m_imageLabel);

    m_image = QPixmap();
    m_watermarkEditor = new WatermarkEditor(this);
    m_cropEditor = new CropEditor(this);
}

QPixmap Editor::generate() const
{
    if (m_image.isNull()) {
        return QPixmap();
    } else {
        QRect crop = m_cropEditor->cropForm()->cropRect();
        QPixmap result = m_image.copy(crop);
        QPainter painter(&result);
        m_watermarkEditor->drawWatermark(&painter, false);
        return result;
    }
}

QPoint Editor::mapTo(const QPoint& point) const
{
    QPointF factor = mapFactor();
    return QPoint(qRound(point.x() / factor.x()), qRound(point.y() / factor.y()));
}
QRect Editor::mapFrom(const QRect& rect) const
{
    return mapFrom(QRectF(rect)).toRect();
}
QRectF Editor::mapFrom(const QRectF& rect) const
{
    QPointF factor = mapFactor();
    return QRectF(rect.x() * factor.x(), rect.y() * factor.y(),
        rect.width() * factor.x(), rect.height() * factor.y());
}
QPointF Editor::mapFrom(const QPointF& point) const
{
    QPointF factor = mapFactor();
    return QPointF(point.x() * factor.x(), point.y() * factor.y());
}
QSizeF Editor::mapFrom(const QSizeF& size) const
{
    QPointF factor = mapFactor();
    return QSizeF(size.width() * factor.x(), size.height() * factor.y());
}

void Editor::setImage(const QPixmap& image)
{
    m_imageLabel->setPixmap(image);

    m_image = image;

    m_watermarkEditor->setVisible(!image.isNull());
    m_cropEditor->setVisible(!image.isNull());

    emit edited();

    qreal scaleFactor;
    if (image.size().width() > image.size().height()) {
        scaleFactor = (width() * 0.75) / (qreal)image.size().width();
    } else {
        scaleFactor = (height() * 0.75) / (qreal)image.size().height();
    }

    zoom(scaleFactor);
}

void Editor::setCropForm(CropForm* form)
{
    m_cropEditor->setCropForm(form);
    m_watermarkEditor->setCropForm(form);
}
void Editor::setWatermarkForm(WatermarkForm* form)
{
    m_watermarkEditor->setWatermarkForm(form);
}

void Editor::updateEditors()
{
    m_cropEditor->update();
    m_watermarkEditor->update();
}
void Editor::updateWatermarkEditor()
{
    m_watermarkEditor->update();
}

void Editor::setWatermarkImage(const QPixmap& image)
{
    m_watermarkEditor->setWatermark(image);
    emit edited();
}

void Editor::zoom(qreal factor)
{
    m_scaleFactor = qBound(kMinZoomFactor, factor, kMaxZoomFactor);
    scaleImage(1.0);
}
void Editor::zoomIn()
{
    scaleImage(1.25);
}
void Editor::zoomOut()
{
    scaleImage(0.8);
}
void Editor::normalSize()
{
    m_imageLabel->adjustSize();
    m_scaleFactor = 1.0;
}
void Editor::fitToWindow(bool fit)
{
    setWidgetResizable(fit);
    if (!fit)
        normalSize();
}

void Editor::scaleImage(qreal factor)
{
    if (m_scaleFactor <= kMinZoomFactor && factor < 1)
        return;
    if (m_scaleFactor >= kMaxZoomFactor && factor > 1)
        return;

    m_scaleFactor *= factor;
    m_imageLabel->resize(m_scaleFactor * m_imageLabel->pixmap(Qt::ReturnByValue).size());

    adjustScrollBar(horizontalScrollBar(), factor);
    adjustScrollBar(verticalScrollBar(), factor);

    emit zoomChanged(m_scaleFactor);
}
void Editor::adjustScrollBar(QScrollBar* scrollBar, qreal factor)
{
    scrollBar->setValue(int(factor * scrollBar->value()
        + ((factor - 1) * scrollBar->pageStep() / 2)));
}
QPointF Editor::mapFactor() const
{
    return QPointF(m_imageLabel->width() / (qreal)m_image.width(),
        m_imageLabel->height() / (qreal)m_image.height());
}

void Editor::updateOverlaysPos()
{
    m_cropEditor->move(m_imageLabel->pos() + QPoint(1, 1));
    m_watermarkEditor->move(m_imageLabel->pos() + QPoint(1, 1));
}
void Editor::updateOverlaysSize()
{
    m_cropEditor->resize(m_imageLabel->size());
    m_watermarkEditor->resize(m_imageLabel->size());
}

void Editor::wheelEvent(QWheelEvent* event)
{
    QPoint delta = event->angleDelta();

    if (delta.y() == 0 || !event->modifiers().testFlag(Qt::ControlModifier)) {
        event->ignore();
        return;
    }

    if (delta.y() > 0)
        zoomIn();
    else
        zoomOut();

    QScrollArea::wheelEvent(event);
}
void Editor::processWheelEvent(QWheelEvent* event)
{
    wheelEvent(event);
}

// ========================================================
// class CropEditor
// ========================================================

CropEditor::CropEditor(Editor* parent)
    : QWidget(parent)
    , m_cropForm(nullptr)
    , m_isDragging(false)
    , m_isResizing(false)
    , m_dragOrigin()
{
    setMouseTracking(true);
}

void CropEditor::paintEvent(QPaintEvent*)
{
    const QColor backgroundColor = QColor(0, 0, 0, 128);
    const QColor handleColor = QColor(0xEE, 0xEE, 0xEE);
    const QColor borderColor = QColor(0xEE, 0xEE, 0xEE, 128);
    const QColor gridColor = QColor(0xEE, 0xEE, 0xEE, 128);
    const QColor textColor = QColor(0xFF, 0xFF, 0xFF);

    QPainter painter(this);
    QRectF crop = editor()->mapFrom(QRectF(m_cropForm->cropRect()));

    { // ===== Background =====
        painter.setBrush(backgroundColor);
        painter.setPen(Qt::NoPen);

        const qreal left = crop.x();
        const qreal top = crop.y();
        const qreal right = crop.x() + crop.width();
        const qreal bottom = crop.y() + crop.height();

        painter.drawRect(QRectF(0, 0, width(), top));
        painter.drawRect(QRectF(0, bottom, width(), height() - bottom));
        painter.drawRect(QRectF(0, top, left, crop.height()));
        painter.drawRect(QRectF(right, top, width() - right, crop.height()));
    }

    { // ===== Simple Border =====
        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(borderColor, 2));
        painter.drawRect(crop);
    }

    { // ===== Border =====
        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(handleColor, 3));

        const qreal length = qMin<qreal>(20, qMin(crop.width(), crop.height()));
        const qreal left = crop.x();
        const qreal top = crop.y();
        const qreal right = crop.x() + crop.width() - 1;
        const qreal bottom = crop.y() + crop.height() - 1;
        const qreal startX = crop.x() + (crop.width() / 2) - length / 2;
        const qreal endX = crop.x() + (crop.width() / 2) + length / 2;
        const qreal startY = crop.y() + (crop.height() / 2) - length / 2;
        const qreal endY = crop.y() + (crop.height() / 2) + length / 2;

        { // ===== Corners =====
            // Top left corner
            painter.drawLine(QPointF(left, top), QPointF(left + length, top));
            painter.drawLine(QPointF(left, top), QPointF(left, top + length));
            // Top right corner
            painter.drawLine(QPointF(right, top), QPointF(right - length, top));
            painter.drawLine(QPointF(right, top), QPointF(right, top + length));
            // Bottom left corner
            painter.drawLine(QPointF(left, bottom), QPointF(left + length, bottom));
            painter.drawLine(QPointF(left, bottom), QPointF(left, bottom - length));
            // Bottom right corner
            painter.drawLine(QPointF(right, bottom), QPointF(right - length, bottom));
            painter.drawLine(QPointF(right, bottom), QPointF(right, bottom - length));
        }
        { // ===== Lines =====
            // Top line
            painter.drawLine(QPointF(startX, top), QPointF(endX, top));
            // Left line
            painter.drawLine(QPointF(left, startY), QPointF(left, endY));
            // Bottom line
            painter.drawLine(QPointF(startX, bottom), QPointF(endX, bottom));
            // Right line
            painter.drawLine(QPointF(right, startY), QPointF(right, endY));
        }
    }

    { // ===== Grid =====
        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(gridColor, 1));

        const qreal left = crop.x();
        const qreal top = crop.y();
        const qreal right = crop.x() + crop.width() - 1;
        const qreal bottom = crop.y() + crop.height() - 1;
        const qreal firstX = crop.x() + (crop.width() * 1 / 3);
        const qreal secondX = crop.x() + (crop.width() * 2 / 3);
        const qreal firstY = crop.y() + (crop.height() * 1 / 3);
        const qreal secondY = crop.y() + (crop.height() * 2 / 3);

        painter.drawLine(QPointF(firstX, top), QPointF(firstX, bottom));
        painter.drawLine(QPointF(secondX, top), QPointF(secondX, bottom));
        painter.drawLine(QPointF(left, firstY), QPointF(right, firstY));
        painter.drawLine(QPointF(left, secondY), QPointF(right, secondY));
    }

    { // ===== Text =====
        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(textColor, 1));

        QString format = QStringLiteral("%0 px");
        QSize cropSize = m_cropForm->cropRect().size();
        QString widthText = format.arg(cropSize.width());
        QString heightText = format.arg(cropSize.height());

        QFontMetricsF metrics(painter.font());
        const qreal padding = 5; // Additional padding to use around width and height text (in pixels)
        qreal widthY = crop.center().y() - crop.height() / 2.0;
        qreal heightX = crop.center().x() - crop.width() / 2.0;

        // We adjust the position of the text if it is outside the widget
        // area so that it is always visible.
        if (widthY < (metrics.height() + padding)) {
            // Outside of the widget area
            widthY += metrics.lineSpacing() + padding;
        } else {
            widthY -= metrics.lineSpacing() - metrics.ascent() + padding;
        }

        if (heightX < (metrics.height() + padding)) {
            // Outside of the widget area
            heightX += metrics.lineSpacing() + padding;
        } else {
            heightX -= metrics.lineSpacing() - metrics.ascent() + padding;
        }

        // Draw the crop width text
        QTransform t1;
        t1.translate(crop.center().x() - (metrics.horizontalAdvance(widthText) / 2), widthY);
        painter.setTransform(t1);
        painter.drawText(0, 0, widthText);

        // Draw the crop height text
        QTransform t2;
        t2.translate(heightX, crop.center().y() + (metrics.horizontalAdvance(heightText) / 2));
        t2.rotateRadians(-M_PI_2);
        painter.setTransform(t2);
        painter.drawText(0, 0, heightText);
    }
}
void CropEditor::mousePressEvent(QMouseEvent* event)
{
    auto corner = resizeCorner(event->pos());
    switch (corner) {
    case AnchorTopLeft:
    case AnchorBottomRight:
    case AnchorTopRight:
    case AnchorBottomLeft:
    case AnchorLeft:
    case AnchorRight:
    case AnchorTop:
    case AnchorBottom:
        m_isResizing = true;
        m_resizingCorner = corner;
        break;
    default: {
        const QRect crop = editor()->mapFrom(m_cropForm->cropRect());
        if (crop.contains(event->pos())) {
            setCursor(Qt::ClosedHandCursor);
            m_isDragging = true;
            m_dragOrigin = editor()->mapTo(event->pos() - crop.topLeft());
        }
    } break;
    }
}
void CropEditor::mouseMoveEvent(QMouseEvent* event)
{
    QRect cropRect = m_cropForm->cropRect();
    QSize size = editor()->imageSize();
    if (m_isDragging) {
        QPoint temp = editor()->mapTo(event->pos()) - m_dragOrigin;
        QPoint pos = QPoint(qBound(0, (size.width() - cropRect.width()), temp.x()),
            qBound(0, (size.height() - cropRect.height()), temp.y()));
        cropRect.moveTo(pos);
        m_cropForm->setCropRect(cropRect);
    } else if (m_isResizing) {
        QPoint tl = cropRect.topLeft();
        QPoint br = cropRect.bottomRight();
        QPoint pos = editor()->mapTo(event->pos());
        pos = QPoint(qBound(0, pos.x(), size.width() - 1), qBound(0, pos.y(), size.height() - 1));

        switch (m_resizingCorner) {
        case AnchorTopLeft:
            cropRect.setTopLeft(QPoint(qMin(pos.x(), br.x()), qMin(pos.y(), br.y())));
            break;
        case AnchorTopRight:
            cropRect.setTopRight(QPoint(qMax(pos.x(), tl.x()), qMin(pos.y(), br.y())));
            break;
        case AnchorBottomLeft:
            cropRect.setBottomLeft(QPoint(qMin(pos.x(), br.x()), qMax(pos.y(), tl.y())));
            break;
        case AnchorBottomRight:
            cropRect.setBottomRight(QPoint(qMax(pos.x(), tl.x()), qMax(pos.y(), tl.y())));
            break;
        case AnchorTop:
            cropRect.setTop(qMin(pos.y(), br.y()));
            break;
        case AnchorBottom:
            cropRect.setBottom(qMax(pos.y(), tl.y()));
            break;
        case AnchorLeft:
            cropRect.setLeft(qMin(pos.x(), br.x()));
            break;
        case AnchorRight:
            cropRect.setRight(qMax(pos.x(), tl.x()));
            break;
        default:
            break;
        }

        switch (m_resizingCorner) {
        case AnchorTopLeft:
            cropRect.moveBottomRight(cropRect.bottomRight());
            break;
        case AnchorTopRight:
            cropRect.moveBottomLeft(cropRect.bottomLeft());
            break;
        case AnchorBottomLeft:
            cropRect.moveTopRight(cropRect.topRight());
            break;
        case AnchorBottomRight:
            cropRect.moveTopLeft(cropRect.topLeft());
            break;
        case AnchorTop:
            cropRect.moveBottom(cropRect.bottom());
            break;
        case AnchorBottom:
            cropRect.moveTop(cropRect.top());
            break;
        case AnchorLeft:
            cropRect.moveRight(cropRect.right());
            break;
        case AnchorRight:
            cropRect.moveLeft(cropRect.left());
            break;
        default:
            break;
        }

        cropRect.setWidth(qMin(cropRect.width(), size.width() - cropRect.x()));
        cropRect.setHeight(qMin(cropRect.height(), size.height() - cropRect.y()));

        m_cropForm->setCropRect(cropRect);
        update();
    } else {
        auto anchor = resizeCorner(event->pos());
        switch (anchor) {
        case AnchorTopLeft:
        case AnchorBottomRight:
            setCursor(Qt::SizeFDiagCursor);
            break;
        case AnchorTopRight:
        case AnchorBottomLeft:
            setCursor(Qt::SizeBDiagCursor);
            break;
        case AnchorLeft:
        case AnchorRight:
            setCursor(Qt::SizeHorCursor);
            break;
        case AnchorTop:
        case AnchorBottom:
            setCursor(Qt::SizeVerCursor);
            break;
        default:
            setCursor(Qt::ArrowCursor);
            break;
        }
    }
}
void CropEditor::mouseReleaseEvent(QMouseEvent*)
{
    setCursor(Qt::ArrowCursor);
    m_isDragging = false;
    m_isResizing = false;
}
void CropEditor::wheelEvent(QWheelEvent* event)
{
    editor()->processWheelEvent(event);
}

WatermarkAnchor CropEditor::resizeCorner(const QPoint& pos)
{
    const QRect crop = editor()->mapFrom(m_cropForm->cropRect());
    const int length = qMin(20, qMin(crop.width(), crop.height()));
    const QSize resizeSize(length, length);
    const QSize vResizer(length, 15);
    const QSize hResizer(15, length);
    const QPoint halfSize(resizeSize.width() / 2, resizeSize.height() / 2);
    const QPoint vHalfSize(vResizer.width() / 2, vResizer.height() / 2);
    const QPoint hHalfSize(hResizer.width() / 2, hResizer.height() / 2);
    if (QRect(crop.topLeft() - halfSize, resizeSize).contains(pos)) { // Top Left
        return AnchorTopLeft;
    } else if (QRect(crop.topRight() - halfSize, resizeSize).contains(pos)) { // Top Right
        return AnchorTopRight;
    } else if (QRect(crop.bottomLeft() - halfSize, resizeSize).contains(pos)) { // Bottom Left
        return AnchorBottomLeft;
    } else if (QRect(crop.bottomRight() - halfSize, resizeSize).contains(pos)) { // Bottom Right
        return AnchorBottomRight;
    } else if (QRect(QPoint(crop.left(), crop.y() + crop.height() / 2) - vHalfSize, vResizer).contains(pos)) { // Left
        return AnchorLeft;
    } else if (QRect(QPoint(crop.right(), crop.y() + crop.height() / 2) - vHalfSize, vResizer).contains(pos)) { // Right
        return AnchorRight;
    } else if (QRect(QPoint(crop.x() + crop.width() / 2, crop.top()) - hHalfSize, hResizer).contains(pos)) { // Top
        return AnchorTop;
    } else if (QRect(QPoint(crop.x() + crop.width() / 2, crop.bottom()) - hHalfSize, hResizer).contains(pos)) { // Bottom
        return AnchorBottom;
    } else {
        return AnchorCenter;
    }
}

// ========================================================
// class WatermarkEditor
// ========================================================

WatermarkEditor::WatermarkEditor(Editor* parent)
    : QWidget(parent)
    , m_cropForm(nullptr)
    , m_watermarkForm(nullptr)
{
}

void WatermarkEditor::drawWatermark(QPainter* painter, bool scaled)
{
    if (m_watermark.isNull())
        return;

    // Computes the watermark position
    QRectF cropRect = scaled ? editor()->mapFrom(QRectF(m_cropForm->cropRect())) : m_cropForm->cropRect();
    QRectF rect;
    QSizeF size;
    if (m_watermarkForm->watermarkUseSize()) {
        qreal scaleFactor = m_watermarkForm->watermarkSize();
        size = m_watermark.size().scaled(cropRect.width() / 3.0, cropRect.height() / 3.0, Qt::KeepAspectRatio);
        size = size.scaled(size * scaleFactor, Qt::KeepAspectRatio);
    } else {
        size = scaled ? editor()->mapFrom(QSizeF(m_watermark.size())) : QSizeF(m_watermark.size());
    }

    if (m_watermarkForm->watermarkAnchor() != AnchorRepeated) {
        updatePosition();
        rect = QRectF(scaled ? editor()->mapFrom(QPointF(m_cachedWatermarkPos)) : QPointF(m_cachedWatermarkPos), size);
    }

    int rotation = 0;
    if (m_watermarkForm->watermarkUseRotation())
        rotation = m_watermarkForm->watermarkRotation();

    // Draw the watermark
    QPixmap image;
    if (m_watermarkForm->watermarkUseColor()) {
        QColor color = m_watermarkForm->watermarkColor();
        if (!m_hasCachedColoredWatermark || m_cachedWatermarkColor != color)
            computeColoredWatermark(color);

        image = m_coloredWatermark.scaled(size.toSize());
    } else {
        image = m_watermark.scaled(size.toSize());
    }

    if (scaled) {
        painter->translate(cropRect.topLeft());
    }

    painter->setOpacity(m_watermarkForm->watermarkOpacity());
    painter->setClipRect(QRectF(0, 0, cropRect.width(), cropRect.height()));
    if (m_watermarkForm->watermarkAnchor() != AnchorRepeated) {
        const qreal width = size.width();
        const qreal height = size.height();

        qreal translatedX = (rect.x() + width / 2.0);
        qreal translatedY = (rect.y() + height / 2.0);

        if (m_watermarkForm->watermarkUseOffset()) {
            QPoint offset = m_watermarkForm->watermarkOffset();
            translatedX += offset.x();
            translatedY += offset.y();
        }

        painter->translate(translatedX, translatedY);
        painter->rotate(rotation);
        painter->drawPixmap(QRectF(-width / 2.0, -height / 2.0, width, height), image, image.rect());
    } else {
        QBrush brush(image);
        painter->setBrush(brush);
        painter->setPen(Qt::NoPen);

        const qreal width = cropRect.size().width() * 2.0;
        const qreal height = cropRect.size().height() * 2.0;

        qreal translatedX = cropRect.size().width() / 2.0;
        qreal translatedY = cropRect.size().height() / 2.0;

        if (m_watermarkForm->watermarkUseOffset()) {
            QPoint offset = m_watermarkForm->watermarkOffset();
            translatedX += offset.x();
            translatedY += offset.y();
        }

        painter->translate(translatedX, translatedY);
        painter->rotate(rotation);
        painter->drawRect(QRectF(-translatedX * 2, -translatedY * 2, width, height));
    }
}

void WatermarkEditor::updatePosition()
{
    const QRect cropRect = m_cropForm->cropRect();
    const int halfCropWidth = cropRect.width() / 2;
    const int halfCropHeight = cropRect.height() / 2;

    QSize size;
    if (m_watermarkForm->watermarkUseSize()) {
        qreal scaleFactor = m_watermarkForm->watermarkSize();
        size = m_watermark.size().scaled(cropRect.width() * 1 / 3, cropRect.height() * 1 / 3, Qt::KeepAspectRatio);
        size = size.scaled(size * scaleFactor, Qt::KeepAspectRatio);
    } else {
        size = m_watermark.size();
    }

    const int halfWidth = size.width() / 2;
    const int halfHeight = size.height() / 2;

    switch (m_watermarkForm->watermarkAnchor()) {
    case AnchorRepeated:
        m_cachedWatermarkPos = QPoint(0, 0);
        break;
    case AnchorTop:
        m_cachedWatermarkPos = QPoint(halfCropWidth - halfWidth, 0);
        break;
    case AnchorTopLeft:
        m_cachedWatermarkPos = QPoint(0, 0);
        break;
    case AnchorTopRight:
        m_cachedWatermarkPos = QPoint(cropRect.width() - size.width(), 0);
        break;
    case AnchorLeft:
        m_cachedWatermarkPos = QPoint(0, halfCropHeight - halfHeight);
        break;
    case AnchorRight:
        m_cachedWatermarkPos = QPoint(cropRect.width() - size.width(), halfCropHeight - halfHeight);
        break;
    case AnchorBottom:
        m_cachedWatermarkPos = QPoint(halfCropWidth - halfWidth, cropRect.height() - size.height());
        break;
    case AnchorBottomLeft:
        m_cachedWatermarkPos = QPoint(0, cropRect.height() - size.height());
        break;
    case AnchorBottomRight:
        m_cachedWatermarkPos = QPoint(cropRect.width() - size.width(), cropRect.height() - size.height());
        break;
    case AnchorCenter:
        m_cachedWatermarkPos = QPoint(halfCropWidth - halfWidth, halfCropHeight - halfHeight);
        break;
    }
}
void WatermarkEditor::setWatermark(const QPixmap& pixmap)
{
    m_watermark = pixmap;
    updatePosition();
    update();
}

void WatermarkEditor::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    drawWatermark(&painter);
}
void WatermarkEditor::wheelEvent(QWheelEvent* event)
{
    editor()->processWheelEvent(event);
}

void WatermarkEditor::computeColoredWatermark(const QColor& color)
{
    m_coloredWatermark = m_watermark;
    if (!m_coloredWatermark.isNull()) {
        QPainter painter(&m_coloredWatermark);
        painter.setCompositionMode(QPainter::CompositionMode_SourceAtop);
        painter.fillRect(m_coloredWatermark.rect(), color);
        painter.end();
    }

    m_cachedWatermarkColor = color;
    m_hasCachedColoredWatermark = true;
}
