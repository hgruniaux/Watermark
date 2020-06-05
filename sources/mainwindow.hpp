//
// MIT License
//
// Copyright (c) 2020 Hubert Gruniaux
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

#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include "preset.hpp"
#include <QMainWindow>
#include <QPixmap>
#include <QRubberBand>
#include <QSlider>

namespace Ui {
class MainWindow;
}

// ========================================================
// class MainWindow
// ========================================================

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    QPixmap image() const;
    QPixmap finalImage() const;
    void setImage(const QPixmap& image);
    void loadImage(const QString& path);

    Preset preset(const QString& name = QString()) const;
    void setPreset(const Preset& preset);

public slots:
    void openImage();
    void saveImage();
    void saveImageFinish();
    void saveImageAs();

    void loadPresets();
    bool savePreset();
    void addPreset(const Preset& preset);

protected:
    void resizeEvent(QResizeEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private:
    void initActions();
    void initSignals();
    void initZoomWidget();

private:
    Ui::MainWindow* ui;
    QRubberBand* m_rubberBand;
    QPixmap m_image;
    QString m_path;
    QSlider* m_zoomSlider;
}; // class MainWindow

#endif // MAINWINDOW_HPP
