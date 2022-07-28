/*-*-c++-*-*************************************************************************************************************
* Copyright 2016 Inesonic, LLC.
*
* MIT License:
*   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
*   documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
*   rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
*   permit persons to whom the Software is furnished to do so, subject to the following conditions:
*   
*   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
*   Software.
*   
*   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*   WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
*   OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
*   OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
********************************************************************************************************************//**
* \file
*
* This file implements the \ref Wh::WebHook class.
***********************************************************************************************************************/

#include <QtGlobal>
#include <QObject>
#include <QTimer>
#include <QCoreApplication>
#include <QString>
#include <QDateTime>
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#include <cstring>

#include <crypto_hmac.h>

#include "wh_web_hook.h"

namespace Wh {
    QByteArray WebHook::globalTimestampSecret;
    QUrl       WebHook::globalTimestampUrl;
    long long  WebHook::globalTimeDelta = 0;

    const Crypto::Hmac::Algorithm hmacAlgorithm = Crypto::Hmac::Algorithm::Sha256;
    const unsigned                hmacBlockLength = Crypto::Hmac::digestSize(hmacAlgorithm);

    WebHook::WebHook(QNetworkAccessManager* networkAccessManager, QObject* parent):QObject(parent) {
        currentNetworkAccessManager = networkAccessManager;
        configure();
    }


    WebHook::WebHook(
            QNetworkAccessManager* networkAccessManager,
            const QByteArray&      webhookSecret,
            QObject*               parent
        ):QObject(
            parent
        ) {
        currentNetworkAccessManager = networkAccessManager;
        currentSecret               = webhookSecret;
        configure();
    }


    WebHook::~WebHook() {}


    void WebHook::setTimestampSecret(const QByteArray& newTimestampSecret) {
        globalTimestampSecret = newTimestampSecret;
    }


    const QByteArray& WebHook::timestampSecret() {
        return globalTimestampSecret;
    }


    void WebHook::setTimestampUrl(const QUrl& timestampWebhookUrl) {
        globalTimestampUrl = timestampWebhookUrl;
    }


    const QUrl& WebHook::timestampUrl() {
        return globalTimestampUrl;
    }


    void WebHook::setTimeDelta(long long newTimeDelta) {
        globalTimeDelta = newTimeDelta;
    }


    long long WebHook::timeDelta() {
        return globalTimeDelta;
    }


    void WebHook::send(const QUrl& destinationUrl, const QJsonDocument& jsonDocument) {
        currentUrl       = destinationUrl;
        currentPayload   = jsonDocument.toJson(QJsonDocument::JsonFormat::Compact);
        remainingRetries = maximumNumberRetries;

        doSend();
    }


    void WebHook::send(const QUrl& destinationUrl, const QJsonObject& jsonObject) {
        send(destinationUrl, QJsonDocument(jsonObject));
    }


    void WebHook::forceTimeDeltaAdjustment() {
        remainingRetries = maximumNumberRetries;
        currentUrl = QUrl();

        doTimestampAdjustment();
    }


    void WebHook::jsonResponseWasReceived(const QJsonDocument& jsonDocument) {
        emit jsonResponseReceived(jsonDocument);
    }


    void WebHook::responseWasReceived(const QByteArray& rawData) {
        emit responseReceived(rawData);
    }


    void WebHook::failed(int networkError) {
        emit failedToSend(networkError);
    }


    void WebHook::timestampReplyReceived() {
        QNetworkReply::NetworkError networkError = pendingReply->error();

        if (networkError == QNetworkReply::NetworkError::NoError) {
            QByteArray receivedData = pendingReply->readAll();
            QString    payload = QString::fromUtf8(receivedData);

            pendingReply->deleteLater();
            pendingReply = Q_NULLPTR;

            bool       ok;
            long long  correction = payload.toLongLong(&ok);

            if (ok) {
                globalTimeDelta = correction;
                emit timeDeltaUpdated();

                if (!currentUrl.isEmpty()) {
                    resendTimer->start(1);
                }
            } else {
                failed(static_cast<int>(QNetworkReply::NetworkError::ProtocolFailure));
            }
        } else {
            pendingReply->deleteLater();
            pendingReply = Q_NULLPTR;

            if (remainingRetries > 0) {
                --remainingRetries;
                timeDeltaTimer->start(1);
            } else {
                failed(static_cast<int>(networkError));
                currentUrl = QUrl();
            }
        }
    }


    void WebHook::messageResponseReceived() {
        QNetworkReply::NetworkError networkError = pendingReply->error();

        if (networkError == QNetworkReply::NetworkError::NoError) {
            QByteArray receivedData = pendingReply->readAll();

            pendingReply->deleteLater();
            pendingReply = Q_NULLPTR;

            QJsonParseError parseError;
            QJsonDocument   jsonDocument = QJsonDocument::fromJson(receivedData, &parseError);
            if (parseError.error == QJsonParseError::NoError) {
                jsonResponseWasReceived(jsonDocument);
            }

            responseWasReceived(receivedData);
            currentUrl = QUrl();
        } else {
            pendingReply->deleteLater();
            pendingReply = Q_NULLPTR;

            if (remainingRetries > 0) {
                --remainingRetries;
                // Server returns a 403 if the hash didn't match.
                if (networkError == QNetworkReply::NetworkError::ContentAccessDenied && remainingRetries > 0) {
                    timeDeltaTimer->start(1);
                } else {
                    resendTimer->start(1);
                }
            } else {
                failed(static_cast<int>(networkError));
                currentUrl = QUrl();
            }
        }
    }


    void WebHook::doTimestampAdjustment() {
        QNetworkRequest request(globalTimestampUrl);
        request.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader, "Inesonic, LLC");
        request.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader, "application/json");
        request.setTransferTimeout();

        currentNetworkAccessManager->setRedirectPolicy(QNetworkRequest::RedirectPolicy::NoLessSafeRedirectPolicy);
        currentNetworkAccessManager->setStrictTransportSecurityEnabled(false);

        unsigned long long currentSystemTime = QDateTime::currentMSecsSinceEpoch();
        QByteArray data = QString::number(currentSystemTime).toUtf8();

        Crypto::Hmac hmac(globalTimestampSecret);
        hmac.addData(data);
        QByteArray hash = hmac.digest();

        QJsonObject json;
        json.insert(QString("data"), QString::fromLatin1(data.toBase64()));
        json.insert(QString("hash"), QString::fromLatin1(hash.toBase64()));
        QByteArray jsonPayload = QJsonDocument(json).toJson(QJsonDocument::JsonFormat::Compact);

        pendingReply = currentNetworkAccessManager->post(request, jsonPayload);
        pendingReply->setParent(this);

        connect(pendingReply, &QNetworkReply::finished, this, &WebHook::timestampReplyReceived);
    }


    void WebHook::doSend() {
        QNetworkRequest request(currentUrl);
        request.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader, "Inesonic, LLC");
        request.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader, "application/json");
        request.setTransferTimeout();

        currentNetworkAccessManager->setRedirectPolicy(QNetworkRequest::RedirectPolicy::NoLessSafeRedirectPolicy);
        currentNetworkAccessManager->setStrictTransportSecurityEnabled(false);

        QString    dateTime = QDateTime::currentDateTimeUtc().addMSecs(globalTimeDelta).toString("yyyyMMddhhmm");
        QByteArray secret   = currentSecret + dateTime.toUtf8();

        while (static_cast<unsigned>(secret.size()) < hmacBlockLength) {
            secret += currentSecret + dateTime.toUtf8();
        }

        unsigned secretSize = static_cast<unsigned>(secret.size());
        if (secretSize > (2 * hmacBlockLength)) {
            for (unsigned i=(2 * hmacBlockLength) ; i<secretSize ; ++i) {
                secret[i] = 0;
            }

            secret = secret.left(2 * hmacBlockLength);
        }

        Crypto::Hmac hmac(secret);
        hmac.addData(currentPayload);
        QByteArray hash = hmac.digest();

        for (unsigned i=0 ; i<(2 * hmacBlockLength) ; ++i) {
            secret[i] = 0;
        }

        QJsonObject json;
        json.insert(QString("data"), QString::fromLatin1(currentPayload.toBase64()));
        json.insert(QString("hash"), QString::fromLatin1(hash.toBase64()));
        QByteArray jsonPayload = QJsonDocument(json).toJson(QJsonDocument::JsonFormat::Compact);

        pendingReply = currentNetworkAccessManager->post(request, jsonPayload);
        pendingReply->setParent(this);

        connect(pendingReply, &QNetworkReply::finished, this, &WebHook::messageResponseReceived);
    }


    void WebHook::configure() {
        resendTimer = new QTimer(this);
        resendTimer->setSingleShot(true);

        timeDeltaTimer = new QTimer(this);
        timeDeltaTimer->setSingleShot(true);

        connect(resendTimer, &QTimer::timeout, this, &WebHook::doSend);
        connect(timeDeltaTimer, &QTimer::timeout, this, &WebHook::doTimestampAdjustment);
    }
}
