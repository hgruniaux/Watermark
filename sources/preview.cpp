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

#include <QPainter>
#include "preview.hpp"
#include "editor.hpp"

// ========================================================
// class Preview
// ========================================================

Preview::Preview(QWidget *parent) : QWidget(parent)
{
    m_editor = nullptr;
}

void Preview::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    QPixmap image = m_editor->generate();
    QSize finalSize = image.size().scaled(size(), Qt::KeepAspectRatio);
    QRect rect = QRect(QPoint(width()/2 - finalSize.width()/2, height()/2 - finalSize.height()/2), finalSize);
    painter.drawPixmap(rect, image);
}
