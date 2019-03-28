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

#include <QDataStream>
#include <QApplication>
#include <QSettings>
#include <QDebug>
#include <QFile>
#include <QDir>
#include "preset.hpp"

// ========================================================
// class Preset
// ========================================================

bool Preset::save(const QString &path, QString* error) const
{
    QFile file(path);
    QDataStream stream(&file);
    if(!file.open(QFile::WriteOnly)) {
        if(error) *error = QString("Could not open file");
        return false;
    }

    // --- Header ---
    stream << PresetManager::getMagicNumber();
    stream << PresetManager::getVersion();
    stream << static_cast<quint8>(stream.version());
    // --- Body ---
    stream << name;
    stream << watermark.originalSize;
    stream << watermark.anchor;
    stream << watermark.size;
    stream << watermark.alpha;
    stream << watermark.index;
    stream << watermark.color;
    stream << crop.rect;
    stream << crop.fixed;

    if(error) *error = QString();
    return true;
}
bool Preset::load(const QString &path, QString* error)
{
    QFile file(path);
    QDataStream stream(&file);
    if(!file.open(QFile::ReadOnly)) {
        if(error) *error = QString("Could not open file");
        return false;
    }

    // --- Header ---
    quint32 magicNumber;
    quint8 version;
    quint8 qtVersion;
    stream >> magicNumber;
    if(magicNumber != PresetManager::getMagicNumber()) {
        if(error) *error = QString("Invalid magic number");
        return false;
    }
    stream >> version;
    if(version > PresetManager::getVersion()) {
        if(error) *error = QString("Unsupported version");
        return false;
    }
    stream >> qtVersion;
    if(qtVersion > QDataStream::Qt_5_12) {
        if(error) *error = QString("Unsupported Qt version");
        return false;
    }
    // --- Body ---
    stream >> name;
    stream >> watermark.originalSize;
    stream >> watermark.anchor;
    stream >> watermark.size;
    stream >> watermark.alpha;
    stream >> watermark.index;
    stream >> watermark.color;
    stream >> crop.rect;
    stream >> crop.fixed;

    if(error) *error = QString();
    return true;
}

// ========================================================
// class PresetManager
// ========================================================

void PresetManager::makeDirectory()
{
    QDir dir(getDirectory());
    if(!dir.exists()) {
        dir.mkpath(".");
    }
}

QString PresetManager::getDirectory()
{
    QString appDir = QApplication::applicationDirPath();
    QString settingsFile = appDir + QDir::separator() + "settings.ini";
    QString defaultDir = appDir + QDir::separator() + "presets";
    QSettings settings(settingsFile, QSettings::IniFormat);
    return settings.value("presets_directory", defaultDir).toString();
}
QString PresetManager::getFileExtension()
{
    return QString(".preset");
}
QString PresetManager::getFile(const QString& name)
{
    return getDirectory() + QDir::separator() + name + getFileExtension();
}

QStringList PresetManager::getPresetFiles()
{
    QString directoryPath(getDirectory());
    QDir directory(directoryPath);
    QStringList files = directory.entryList({"*" + getFileExtension()},
                                            QDir::Files | QDir::NoSymLinks);
    for(auto& file : files) {
        file.prepend(directoryPath + QDir::separator());
    }
    return files;
}
PresetList PresetManager::getPresets()
{
    QStringList files = getPresetFiles();
    PresetList presets; presets.reserve(files.size());
    for(auto file : files) {
        Preset preset;
        QString error;
        if(!preset.load(file, &error)) {
            qWarning() << "Warning: Invalid preset file" << file
                       << ", " << error;
            presets.append(preset);
        } else {
            presets.append(preset);
        }
    }
    return presets;
}

bool PresetManager::addPreset(const Preset& preset, QString* error)
{
    QString file = getFile(preset.name);
    return preset.save(file, error);
}
bool PresetManager::removePreset(const Preset& preset, QString* error)
{
    QString file = getFile(preset.name);
    if(QFile::exists(file)) {
        bool result = QFile::remove(file);
        if(!result) {
            if(error) *error = QString("Failed to remove preset file");
            return false;
        } else {
            if(error) *error = QString();
            return true;
        }
    } else {
        if(error) *error = QString("Preset doesn't exists");
        return false;
    }
}
bool PresetManager::replacePreset(const Preset& before, const Preset& after, QString* error)
{
    if(!removePreset(before, error)) return false;
    if(!addPreset(after, error)) return false;
    return true;
}

Preset PresetManager::getDefaultPreset()
{
    Preset preset;
    return preset;
}

quint32 PresetManager::getMagicNumber()
{
    // Preset Binary
    // 0x50: 'P'
    // 0x52: 'R'
    // 0x45: 'E'
    // 0x42: 'B'
    return 0x50524542;
}
quint8 PresetManager::getVersion()
{
    // Data Format version 1
    return 0x1;
}
