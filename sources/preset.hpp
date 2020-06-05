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

#ifndef PRESET_HPP
#define PRESET_HPP

#include <QString>
#include <QVector>
#include <QRect>
#include <QColor>
#include "watermark.hpp"

// ========================================================
// class Preset
// ========================================================

class Preset {
public:
    QString name;
    struct {
        bool originalSize;
        bool originalColor;
        quint8 anchor;
        quint8 size;
        quint8 margin;
        quint16 rotation;
        quint8 alpha;
        quint16 index;
        QColor color;
    } watermark;
    struct {
        QRect rect;
        bool fixed;
    } crop;

public:
    bool save(const QString& file, QString* error = nullptr) const;
    bool load(const QString& file, QString* error = nullptr);
};

typedef QVector<Preset> PresetList;

// ========================================================
// class PresetManager
// ========================================================

class PresetManager {
public:
    static void makeDirectory();

    static QString directoryPath();
    static QDir directory();
    static QString filePath(const QString& name);

    static QFileInfoList presetFiles();
    static PresetList presets();

    static bool addPreset(const Preset& preset, QString* error = nullptr);
    static bool removePreset(const Preset& preset, QString* error = nullptr);
    static bool replacePreset(const Preset& before, const Preset& after, QString* error = nullptr);

    static Preset defaultPreset();

    static quint32 getMagicNumber();
    static quint8 getVersion();

private:
    PresetManager() = delete;
    PresetManager(const PresetManager&) = delete;
    PresetManager& operator=(const PresetManager&) = delete;
}; // class PresetManager

#endif // PRESET_HPP
