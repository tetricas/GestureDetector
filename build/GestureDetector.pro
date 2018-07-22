#-------------------------------------------------
#
# Project created by QtCreator 2017-12-13T15:48:49
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GestureDetector
TEMPLATE = app

CONFIG(debug, debug|release) {
    DESTDIR = $$PWD/../bin/GestureDetector/debug
} else {
    DESTDIR = $$PWD/../bin/GestureDetector/release
}

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

HEADERS += \
        ../include/mainwindow.h \
        ../include/opencv_processor.h \
        ../include/gesture_manager.h

SOURCES += \
        ../src/main.cpp \
        ../src/mainwindow.cpp \
        ../src/opencv_processor.cpp \
        ../src/gesture_manager.cpp

FORMS += \
    ../rc/mainwindow.ui

OTHER_FILES += \
    ../xml/gestures.xml

INCLUDEPATH += C:\opencv\build\install/include \
            ../include/ \
            ../thirdparty/rapidxml-1.13

LIBS += C:\opencv\build\install/x86/mingw/bin\libopencv_core340.dll
LIBS += C:\opencv\build\install/x86/mingw/bin\libopencv_highgui340.dll
LIBS += C:\opencv\build\install/x86/mingw/bin\libopencv_imgcodecs340.dll
LIBS += C:\opencv\build\install/x86/mingw/bin\libopencv_imgproc340.dll
LIBS += C:\opencv\build\install/x86/mingw/bin\libopencv_features2d340.dll
LIBS += C:\opencv\build\install/x86/mingw/bin\libopencv_calib3d340.dll
LIBS += C:\opencv\build\install/x86/mingw/bin\libopencv_videoio340.dll
LIBS += C:\opencv\build\install/x86/mingw/bin\libopencv_video340.dll

win32 {
    PWD_WIN = $${PWD}
    DESTDIR_WIN = $${DESTDIR}
    PWD_WIN ~= s,/,\\,g
    DESTDIR_WIN ~= s,/,\\,g

    copyfiles.commands = $$quote(cmd /c xcopy /S /I /Y /D $${PWD_WIN}\\..\\xml $${DESTDIR_WIN}\\$${TYPE})

    QMAKE_EXTRA_TARGETS += copyfiles
    POST_TARGETDEPS += copyfiles
}
