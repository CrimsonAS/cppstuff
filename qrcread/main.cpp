/*
 * qrcread
 *  A simple tool to get QResources out of a binary
 *
 * Copyright (c) 2013 Jolla Ltd. <robin.burchell@jolla.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <dlfcn.h>
#include <stdio.h>
#include <sys/errno.h>
#include <QDebug>

#define protected public // HACK, QResource::children is protected, I'm too lazy to subclass
#include <QResource>

void recursivelyPrintResource(const QResource &r)
{
    qDebug() << r.absoluteFilePath();
    qDebug() << r.children();

    foreach (const QString &child, r.children()) {
        QResource aChild(r.absoluteFilePath() + "/" + child);
        recursivelyPrintResource(aChild);
    }
}

// NB: we don't have a valid qApp, be careful what parts of Qt you use.
// It may be possible to create one, I didn't to try avoid interfering with
// the application loading.
int main(int argc, char **argv) {
    // Assuming the binary is build as a dlopenable shared object, let's try
    // open it and read what's inside. This won't work for all cases, of
    // course, but it'll catch the majority.
    //
    // Another alternative would be to somehow run it in a sandbox after
    // intercepting qRegisterResourceData using LD_LIBRARY_PATH and kill it off
    // immediately: qRegisterResourceData is called on binary/library load using
    // Q_CONSTRUCTOR_FUNCTION
    qDebug() << "opening " << argv[1];
    void *hnd = dlopen(argv[1], RTLD_NOW);
    qDebug() << "opened " << hnd;

    {
        QResource r("/"); // the root of all evil
        recursivelyPrintResource(r);
    }
}

