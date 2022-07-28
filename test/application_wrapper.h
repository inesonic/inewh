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
* This header provides a wrapper for tests pulling the QtTest framework more cleanly into the signal/slot mechanism.
***********************************************************************************************************************/

#ifndef APPLICATION_WRAPPER_H
#define APPLICATION_WRAPPER_H

#include <QtGlobal>
#include <QObject>
#include <QList>

class QCoreApplication;
class QTimer;

/**
 * Class that pulls the QtTest functions under an instance of \ref UniqueApplication.  The class allows tests to be
 * run under the application framework expected by the various classes.
 *
 * This approach was inspired by comments on the web page:
 *
 *     http://stackoverflow.com/questions/1524390/what-unit-testing-framework-should-i-use-for-qt
 */
class ApplicationWrapper:public QObject {
    Q_OBJECT

    public:
        /**
         * Constructor
         *
         * \param[in] argumentCount The number of command line arguments.
         *
         * \param[in] argumentValues The command line arguments.
         *
         * \param[in] parent Pointer to the parent object.
         */
        ApplicationWrapper(
            int&     argumentCount,
            char**   argumentValues,
            QObject* parent = Q_NULLPTR
        );

        ~ApplicationWrapper() override;

        /**
         * Adds a new test class to the test framework.
         *
         * \param[in] testInstance The test instance to be added.
         */
        void includeTest(QObject* testInstance);

        /**
         * Executes all registered tests for the application.
         *
         * \return Returns 0 on success, non-zero on error.
         */
        int exec();

    private slots:
        /**
         * Starts the tests, in succession.  Triggered by the signal/slot event loop.
         */
        void runNextTest();

    private:
        void startNextTest();

        QCoreApplication*         applicationInstance;
        QTimer*                   startTimer;
        QList<QObject*>           registeredTests;
        QList<QObject*>::iterator nextTest;
        int                       currentStatus;
};

#endif
