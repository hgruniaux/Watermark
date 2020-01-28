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

#include <QDesktopServices>
#include "reportissuedialog.hpp"
#include "ui_reportissuedialog.h"

ReportIssueDialog::ReportIssueDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ReportIssueDialog)
{
    ui->setupUi(this);
    connect(ui->buttonCancel, &QPushButton::clicked, this, &ReportIssueDialog::close);
    connect(ui->buttonSend, &QPushButton::clicked, this, &ReportIssueDialog::send);
}

ReportIssueDialog::~ReportIssueDialog()
{
    delete ui;
}

void ReportIssueDialog::send()
{
    QString body = generateBody();
    QString url = QString("https://github.com/%0/%1/issues/new?body=%2")
                  .arg("hgruniaux").arg("Watermark").arg(body);
    QDesktopServices::openUrl(QUrl(url));
    close();
}
QString ReportIssueDialog::generateBody()
{
    QString header = QString("**Version**: _%0_\n**Qt**: _%1_\n**OS**: _%2_\n")
                     .arg(ui->checkIncludeVersion->isChecked() ? QApplication::applicationVersion() : "Unknown")
                     .arg(ui->checkIncludeQtVersion->isChecked() ? qVersion() : "Unknown")
                     .arg(ui->checkIncludeOS->isChecked() ? QSysInfo::prettyProductName() : "Unknown");
    QString description = QString("**Description**:\n_%0_\n")
                         .arg(ui->editDescription->toPlainText());
    QString body = QString("%0\n%1")
                   .arg(header)
                   .arg(description);
    return body;
}
