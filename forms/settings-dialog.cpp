/*
obs-websocket
Copyright (C) 2016-2017	Stéphane Lepin <stephane.lepin@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program. If not, see <https://www.gnu.org/licenses/>
*/

#include <obs-frontend-api.h>

#include "obs-websocket.h"
#include "Config.h"
#include "WSServer.h"
#include "settings-dialog.h"
#include "ui_settings-dialog.h"

#include <QtNetwork>
#include <QErrorMessage>

#include "curl_easy.h"
#include "curl_pair.h"
#include "curl_form.h"
#include "curl_exception.h"

using std::string;

using curl::curl_form;
using curl::curl_easy;
using curl::curl_pair;
using curl::curl_easy_exception;
using curl::curlcpp_traceback;

#define CHANGE_ME "changeme"

SettingsDialog::SettingsDialog(QWidget* parent) :
    QDialog(parent, Qt::Dialog),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    connect(ui->buttonBox, &QDialogButtonBox::accepted,
        this, &SettingsDialog::FormAccepted);
}

void SettingsDialog::showEvent(QShowEvent* event) {

    /*Config* conf = Config::Current();

    ui->serverEnabled->setChecked(conf->ServerEnabled);
    ui->serverPort->setValue(conf->ServerPort);

    ui->debugEnabled->setChecked(conf->DebugEnabled);
    ui->alertsEnabled->setChecked(conf->AlertsEnabled);

    ui->authRequired->setChecked(conf->AuthRequired);
    ui->password->setText(CHANGE_ME);*/
}

void SettingsDialog::ToggleShowHide() {
    if (!isVisible())
        setVisible(true);
    else
        setVisible(false);
}

void SettingsDialog::FormAccepted() {

    QNetworkRequest request(QUrl("http://127.0.0.1:8060/auth/token"));

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject body;

    body.insert("clientId", "a5d3a028a711f1617bbd31f2d23efb7fa753ae5b18a66e76e46309deb73687c7dfbdf0de0141544f2f79bcc3dc6f9503");
    body.insert("clientSecret", "fe1af4b85f753a15abd08fa47ff8cd8dc53f55df272048eece05fffb7e9b7dc211927740de6506ff15dca47f8d5c42a1");
    body.insert("grantType", "password");
    body.insert("username", ui->username->text());
    body.insert("password", ui->password->text());

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);

    connect(manager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(syncRequestFinished(QNetworkReply*)));

    QByteArray data = QJsonDocument(body).toJson();

    QNetworkReply *reply =  manager->post(request, data);

    while(!reply->isFinished()){
        qApp->processEvents();
    }

    QByteArray response_data = reply->readAll();
    QJsonDocument json = QJsonDocument::fromJson(response_data);

    QJsonObject response = json.object();

    reply->deleteLater();

    QByteArray token = response["token"].toString().toUtf8();
    const char *new_token = token;

    blog(LOG_INFO, "Response %s, token %s", response_data.data(), token.data());

    Config* conf = Config::Current();

    conf->token = new_token;

    conf->Save();
}

SettingsDialog::~SettingsDialog() {
    delete ui;
}
