QT       += core gui xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mydelegate.cpp \
    tinystr.cpp \
    tinyxml.cpp \
    tinyxmlerror.cpp \
    tinyxmlparser.cpp \
    widget.cpp \
    updateThread.cpp \
    transferThread.cpp \
    dataProto.pb.cc

HEADERS += \
    mydelegate.h \
    tinystr.h \
    tinyxml.h \
    widget.h \
    dataDeclaration.h \
    transferThread.h \
    updateThread.h \
    dataProto.pb.h \
    ZipPack.h

#INCLUDEPATH += \


FORMS += \
    widget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    image.qrc

DEFINES += PROTOBUF_USE_DLLS

QMAKE_CFLAGS_DEBUG += -MDd
QMAKE_CXXFLAGS_DEBUG += -MDd

QMAKE_CFLAGS_RELEASE += -MD
QMAKE_CXXFLAGS_RELEASE += -MD

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/./ -llibprotobuf
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/./ -llibprotobufd

win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/./ -lZipPack

INCLUDEPATH += $$PWD/.
DEPENDPATH += $$PWD/.
