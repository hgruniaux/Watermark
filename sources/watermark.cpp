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
#include "watermark.hpp"

// ========================================================
// class Watermark
// ========================================================

bool Watermark::save(const QString &path, QString* error) const
{
    QFile file(path);
    QDataStream stream(&file);
    if(!file.open(QFile::WriteOnly)) {
        if(error) *error = QString("Could not open file");
        return false;
    }

    // --- Header ---
    stream << WatermarkManager::getMagicNumber();
    stream << WatermarkManager::getVersion();
    stream << static_cast<quint8>(stream.version());
    // --- Body ---
    stream << name;
    stream << image;

    if(error) *error = QString();
    return true;
}
bool Watermark::load(const QString &path, QString* error)
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
    if(magicNumber != WatermarkManager::getMagicNumber()) {
        if(error) *error = QString("Invalid magic number");
        return false;
    }
    stream >> version;
    if(version > WatermarkManager::getVersion()) {
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
    stream >> image;

    if(error) *error = QString();
    return true;
}

// ========================================================
// class WatermarkManager
// ========================================================

void WatermarkManager::makeDirectory()
{
    QDir dir(getDirectory());
    if(!dir.exists()) {
        dir.mkpath(".");
    }
}

QString WatermarkManager::getDirectory()
{
    QString appDir = QApplication::applicationDirPath();
    QString settingsFile = appDir + QDir::separator() + "settings.ini";
    QString defaultDir = appDir + QDir::separator() + "watermarks";
    QSettings settings(settingsFile, QSettings::IniFormat);
    return settings.value("watermarks_directory", defaultDir).toString();
}
QString WatermarkManager::getFileExtension()
{
    return QString(".watermark");
}
QString WatermarkManager::getFile(const QString& name)
{
    return getDirectory() + QDir::separator() + name + getFileExtension();
}

QStringList WatermarkManager::getWatermarkFiles()
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
WatermarkList WatermarkManager::getWatermarks()
{
    QStringList files = getWatermarkFiles();
    WatermarkList watermarks; watermarks.reserve(files.size());
    for(auto file : files) {
        Watermark watermark;
        QString error;
        if(!watermark.load(file, &error)) {
            qWarning() << "Warning: Invalid preset file" << file
                       << ", " << error;
            watermarks.append(watermark);
        } else {
            watermarks.append(watermark);
        }
    }
    return watermarks;
}

bool WatermarkManager::addWatermark(const Watermark& watermark, QString* error)
{
    QString file = getFile(watermark.name);
    return watermark.save(file, error);
}
bool WatermarkManager::removeWatermark(const Watermark& watermark, QString* error)
{
    QString file = getFile(watermark.name);
    if(QFile::exists(file)) {
        bool result = QFile::remove(file);
        if(!result) {
            if(error) *error = QString("Failed to remove watermark file");
            return false;
        } else {
            if(error) *error = QString();
            return true;
        }
    } else {
        if(error) *error = QString("Watermark doesn't exists");
        return false;
    }
}
bool WatermarkManager::replaceWatermark(const Watermark& before, const Watermark& after, QString* error)
{
    if(!removeWatermark(before, error)) return false;
    if(!addWatermark(after, error)) return false;
    return true;
}

Watermark WatermarkManager::getDefaultWatermark()
{
    Watermark watermark;
    return watermark;
}

quint32 WatermarkManager::getMagicNumber()
{
    // Watermark Binary
    // 0x57: 'W'
    // 0x41: 'A'
    // 0x54: 'T'
    // 0x42: 'B'
    return 0x57415442;
}
quint8 WatermarkManager::getVersion()
{
    // Data Format version 1
    return 0x1;
}
