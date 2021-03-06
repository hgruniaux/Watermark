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

#include <QApplication>
#include <QDataStream>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QSettings>
#include <QStandardPaths>

#include "watermark.hpp"

// ========================================================
// class Watermark
// ========================================================

bool Watermark::save(const QString& path, QString* error) const
{
    QFile file(path);
    QDataStream stream(&file);
    if (!file.open(QFile::WriteOnly)) {
        if (error)
            *error = QStringLiteral("Could not open file");
        return false;
    }

    // --- Header ---
    stream << WatermarkManager::getMagicNumber();
    stream << WatermarkManager::getVersion();
    stream << static_cast<quint8>(stream.version());
    // --- Body ---
    stream << name;
    stream << image;

    if (stream.status() != QDataStream::Ok) {
        if (error)
            *error = QStringLiteral("Internal error");
        return false;
    }

    if (error)
        *error = {};
    return true;
}
bool Watermark::load(const QString& path, QString* error)
{
    QFile file(path);
    QDataStream stream(&file);
    if (!file.open(QFile::ReadOnly)) {
        if (error)
            *error = QStringLiteral("Could not open file");
        return false;
    }

    // --- Header ---
    quint32 magicNumber;
    quint8 version;
    quint8 qtVersion;
    stream >> magicNumber;
    if (magicNumber != WatermarkManager::getMagicNumber()) {
        if (error)
            *error = QStringLiteral("Invalid magic number");
        return false;
    }
    stream >> version;
    if (version > WatermarkManager::getVersion()) {
        if (error)
            *error = QStringLiteral("Unsupported version");
        return false;
    }
    stream >> qtVersion;
    stream.setVersion(qtVersion);

    // --- Body ---
    stream >> name;
    stream >> image;

    if (stream.status() != QDataStream::Ok) {
        if (error)
            *error = QStringLiteral("Ill-formed watermark file");
        return false;
    }

    if (error)
        *error = {};
    return true;
}

// ========================================================
// class WatermarkManager
// ========================================================

void WatermarkManager::makeDirectory()
{
    QDir dir = directory();
    if (!dir.exists()) {
        dir.mkpath(".");
    }
}

QString WatermarkManager::directoryPath()
{
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return dataDir + "/watermarks";
}
QDir WatermarkManager::directory()
{
    return QDir(directoryPath());
}
QString WatermarkManager::legacyDirectoryPath()
{
    QString appDir = QApplication::applicationDirPath();
    return appDir + "/watermarks";
}
QDir WatermarkManager::legacyDirectory()
{
    return QDir(legacyDirectoryPath());
}
QString WatermarkManager::filePath(const QString& name)
{
    return directory().filePath(name + ".watermark");
}

QFileInfoList WatermarkManager::watermarkFiles()
{
    QFileInfoList files;

    // Find watermark files
    files << directory().entryInfoList({ "*.watermark" },
        QDir::Files | QDir::NoSymLinks);

    // Find legacy watermark files
    files << legacyDirectory().entryInfoList({ "*.watermark" },
        QDir::Files | QDir::NoSymLinks);

    return files;
}
WatermarkList WatermarkManager::watermarks()
{
    QFileInfoList files = watermarkFiles();
    WatermarkList watermarks;
    watermarks.reserve(files.size());
    for (auto file : files) {
        Watermark watermark;
        QString error;
        if (!watermark.load(file.absoluteFilePath(), &error)) {
            qWarning() << "Error: Invalid watermark file" << file
                       << ", " << error;
        } else {
            watermarks.append(watermark);
        }
    }
    return watermarks;
}

bool WatermarkManager::addWatermark(const Watermark& watermark, QString* error)
{
    QString file = filePath(watermark.name);
    return watermark.save(file, error);
}
bool WatermarkManager::removeWatermark(const Watermark& watermark, QString* error)
{
    QString file = filePath(watermark.name);
    if (QFile::exists(file)) {
        bool result = QFile::remove(file);
        if (!result) {
            if (error)
                *error = QStringLiteral("Failed to remove watermark file");
            return false;
        } else {
            if (error)
                *error = {};
            return true;
        }
    } else {
        if (error)
            *error = QStringLiteral("Watermark doesn't exists");
        return false;
    }
}
bool WatermarkManager::replaceWatermark(const Watermark& before, const Watermark& after, QString* error)
{
    if (!removeWatermark(before, error))
        return false;
    if (!addWatermark(after, error))
        return false;
    return true;
}

Watermark WatermarkManager::defaultWatermark()
{
    return Watermark();
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
