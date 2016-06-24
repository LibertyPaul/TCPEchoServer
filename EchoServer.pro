TEMPLATE = app
CONFIG += console
CONFIG += c++14
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp

include(deployment.pri)
qtcAddDeployment()

HEADERS += \
    TCPEchoServer.hpp \
    SocketDescriptorHandler.hpp \
    TestClient.hpp

LIBS += -pthread
