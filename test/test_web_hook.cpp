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
* This file implements tests for the \ref WebHook class.
***********************************************************************************************************************/

#include <QDebug>
#include <QObject>
#include <QtTest/QtTest>
#include <QEventLoop>
#include <QTimer>
#include <QByteArray>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QByteArray>

#include <cstdint>

#include <wh_web_hook.h>

#include "test_web_hook.h"

// Fill me in with the time-stamp webhook secret.
static const std::uint8_t tsSecretData[64] = {
    0x13, 0xDF, 0x36, 0x03,   0x22, 0xAD, 0x3A, 0x99,
    0xBC, 0x8F, 0x38, 0x14,   0xDA, 0x35, 0x25, 0x16,
    0xB8, 0x23, 0x5B, 0x3D,   0xB7, 0xA3, 0xDE, 0xF3,
    0x5F, 0x46, 0x9D, 0x84,   0x21, 0x8A, 0x07, 0xB0,
    0x50, 0xD4, 0xE9, 0xD2,   0xDE, 0x82, 0xC5, 0x58,
    0x6C, 0xF9, 0x57, 0xB0,   0x4F, 0x09, 0x69, 0x4B,
    0x7D, 0xE1, 0x0A, 0x8D,   0x9D, 0x88, 0x0B, 0xD0,
    0x04, 0x7D, 0xA6, 0x99,   0x99, 0x17, 0x8C, 0x8D
};

static const std::uint8_t testSecretData[52] = {
    0xB1, 0xD7, 0xAC, 0x38,   0x6C, 0xE4, 0xD3, 0x19,
    0x4F, 0xCC, 0x35, 0xE0,   0xA8, 0xFB, 0x65, 0x41,
    0x0F, 0xBE, 0x39, 0x41,   0x43, 0xF0, 0x47, 0x63,
    0x1D, 0xAC, 0x4C, 0xA1,   0x84, 0x30, 0x90, 0xC0,
    0xA2, 0x3E, 0x80, 0xA9,   0x07, 0x6F, 0x97, 0xA2,
    0x72, 0x52, 0x57, 0xC3,   0xC2, 0x03, 0xDC, 0x2F,
    0xAD, 0x2D, 0x6B, 0xE1
};

const QByteArray TestWebHook::timeStampSecret(reinterpret_cast<const char*>(tsSecretData), 64);
const QString    TestWebHook::timeStampWebHookUrl("https://autonoma2.inesonic.com/v2/ts");

const QByteArray TestWebHook::testSecret(reinterpret_cast<const char*>(testSecretData), 52);
const QString    TestWebHook::testWebHookUrl("https://autonoma2.inesonic.com/v2/test");

TestWebHook::TestWebHook() {
    eventLoop           = new QEventLoop(this);
    quitOnTimestampUpdate = false;
    operationFailed       = false;
    receivedJsonData      = false;
    receivedRawData       = false;
    timeDeltaWasUpdated   = false;

    networkAccessManager = new QNetworkAccessManager(this);
    webHook              = new Wh::WebHook(networkAccessManager, testSecret, this);

    Wh::WebHook::setTimestampSecret(timeStampSecret);
    Wh::WebHook::setTimestampUrl(QUrl(timeStampWebHookUrl));

    connect(webHook, &Wh::WebHook::timeDeltaUpdated, this, &TestWebHook::timeDeltaUpdated);
    connect(webHook, &Wh::WebHook::jsonResponseReceived, this, &TestWebHook::jsonResponseReceived);
    connect(webHook, &Wh::WebHook::responseReceived, this, &TestWebHook::responseReceived);
    connect(webHook, &Wh::WebHook::failedToSend, this, &TestWebHook::failedToSend);
}


TestWebHook::~TestWebHook() {}


void TestWebHook::timeDeltaUpdated() {
    timeDeltaWasUpdated = true;
    if (quitOnTimestampUpdate) {
        eventLoop->quit();
    }
}


void TestWebHook::jsonResponseReceived(const QJsonDocument& jsonResponse) {
    receivedJsonData = true;
    reportedJsonData = jsonResponse;
}


void TestWebHook::responseReceived(const QByteArray& rawData) {
    receivedRawData = true;
    reportedRawData = rawData;

    eventLoop->quit();
}


void TestWebHook::failedToSend(int networkError) {
    (void) networkError;
    operationFailed = true;
    eventLoop->quit();
}


void TestWebHook::initTestCase() {}


void TestWebHook::testTimeDelta() {
    quitOnTimestampUpdate = true;
    operationFailed       = false;
    receivedJsonData      = false;
    receivedRawData       = false;
    timeDeltaWasUpdated   = false;

    webHook->forceTimeDeltaAdjustment();
    eventLoop->exec();

    QCOMPARE(timeDeltaWasUpdated, true);
    QCOMPARE(operationFailed, false);
}


void TestWebHook::testMessage() {
    quitOnTimestampUpdate = false;
    operationFailed       = false;
    receivedJsonData      = false;
    receivedRawData       = false;
    timeDeltaWasUpdated   = false;

    QJsonObject json;
    json.insert(QString("test_data"), 1);

    webHook->send(testWebHookUrl, json);
    eventLoop->exec();

    QCOMPARE(receivedJsonData, true);
    QCOMPARE(receivedRawData, true);

    QJsonObject jsonObject = reportedJsonData.object();
    QCOMPARE(jsonObject.size(), 1);
}


void TestWebHook::testMessageWithDelta() {
    webHook->setTimeDelta(3600000); // An error of one hour
    testMessage();
}


void TestWebHook::cleanupTestCase() {}
