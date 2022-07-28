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
* This file implements the \ref ApplicationWrapper class that pulls the QtTest framework more cleanly into the Qt
* signal/slot mechanisms of Qt.
***********************************************************************************************************************/

#include <QCoreApplication>
#include <QTimer>
#include <QDebug>
#include <QObject>
#include <QList>
#include <QtTest/QtTest>
#include <QTimer>

#include "application_wrapper.h"

ApplicationWrapper::ApplicationWrapper(
        int&     argumentCount,
        char**   argumentValues,
        QObject* parent
    ):QObject(
         parent
    ) {
    applicationInstance = new QCoreApplication(argumentCount, argumentValues);

    startTimer = new QTimer(this);
    startTimer->setSingleShot(true);

    connect(startTimer, &QTimer::timeout, this, &ApplicationWrapper::runNextTest);
}

ApplicationWrapper::~ApplicationWrapper() {
    delete applicationInstance;
}


void ApplicationWrapper::includeTest(QObject* testInstance) {
    testInstance->setParent(this);
    registeredTests.append(testInstance);
}


int ApplicationWrapper::exec() {
    nextTest = registeredTests.begin();
    startTimer->start(1);

    int exitStatus = applicationInstance->exec();
    return exitStatus | currentStatus;
}


void ApplicationWrapper::runNextTest() {
    currentStatus |= QTest::qExec(*nextTest);

    ++nextTest;
    startNextTest();
}


void ApplicationWrapper::startNextTest() {
    if (nextTest == registeredTests.end()) {
        applicationInstance->quit();
    } else {
        startTimer->start(1);
    }
}
