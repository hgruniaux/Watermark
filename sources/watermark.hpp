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

#ifndef WATERMARK_HPP
#define WATERMARK_HPP

#include <QString>
#include <QFileInfo>
#include <QPixmap>

// ========================================================
// enum WatermarkAnchor
// ========================================================

enum WatermarkAnchor {
    AnchorTop,
    AnchorTopLeft,
    AnchorTopRight,
    AnchorLeft,
    AnchorRight,
    AnchorBottom,
    AnchorBottomLeft,
    AnchorBottomRight,
    AnchorCenter,
    AnchorRepeated
}; // enum WatermarkAnchor

// ========================================================
// class Watermark
// ========================================================

class Watermark {
public:
    QString name;
    QPixmap image;

public:
    bool save(const QString& file, QString* error = nullptr) const;
    bool load(const QString& file, QString* error = nullptr);
}; // class Watermark

typedef QVector<Watermark> WatermarkList;

// ========================================================
// class WatermarkManager
// ========================================================

class WatermarkManager {
public:
    static void makeDirectory();

    static QString directoryPath();
    static QDir directory();
    static QString legacyDirectoryPath();
    static QDir legacyDirectory();
    static QString filePath(const QString& name);

    static QFileInfoList watermarkFiles();
    static WatermarkList watermarks();

    static bool addWatermark(const Watermark& watermark, QString* error = nullptr);
    static bool removeWatermark(const Watermark& watermark, QString* error = nullptr);
    static bool replaceWatermark(const Watermark& before, const Watermark& after, QString* error = nullptr);

    static Watermark defaultWatermark();

    static quint32 getMagicNumber();
    static quint8 getVersion();

private:
    WatermarkManager() = delete;
    WatermarkManager(const WatermarkManager&) = delete;
    WatermarkManager& operator=(const WatermarkManager&) = delete;
}; // class WatermarkManager

#endif // WATERMARK_HPP
