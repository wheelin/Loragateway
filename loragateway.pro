TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG += qt c++11
LIBS += -lwiringPi -lm -lpthread

SOURCES += main.cpp \
    Data/sensordata.cpp \
    OTTMis/ottmisfile.cpp \
    Transmission/gwloramac.cpp \
    Transmission/lora.cpp \
    Transmission/packet.cpp \
    Transmission/packetfifo.cpp \
    OTTMis/ottmissensor.cpp \
    Data/conversion.cpp \
    Log/log.cpp

include(deployment.pri)
qtcAddDeployment()

HEADERS += \
    Data/sensordata.h \
    OTTMis/ottmisfile.h \
    Transmission/gwloramac.h \
    Transmission/lora.h \
    Transmission/packet.h \
    Transmission/packetfifo.h \
    OTTMis/ottmissensor.h \
    Data/conversion.h \
    Log/log.h \
    OTTMis/OTTCodes.h

