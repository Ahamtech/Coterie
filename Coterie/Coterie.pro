APP_NAME = Coterie

CONFIG += qt warn_on cascades10

include(config.pri)

LIBS += -lbb -lbbnetwork -lbbdevice -lbbsystem -lbbdata -lbbcascadespickers
QT += xml