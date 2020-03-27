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

#include "mainwindow.hpp"
#include <QApplication>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationDisplayName("Watermark");
    a.setApplicationName("watermark");
    a.setApplicationVersion("1.1.0");

    QTranslator translator;

    if (translator.load(QLocale(), QLatin1String("qt"), QLatin1String("_"), QLatin1String("./translations/"), QLatin1String(".qm"))) {
        a.installTranslator(&translator);
    }

    if (translator.load(QLocale(), QLatin1String("wm"), QLatin1String("_"), QLatin1String("./translations/"), QLatin1String(".qm"))) {
        a.installTranslator(&translator);
    }

    MainWindow w;
    w.showMaximized();

    if(a.arguments().size() > 1) {
        w.loadImage(a.arguments()[1]);
    }

    return a.exec();
}
