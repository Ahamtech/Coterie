APP_NAME = CoterieService

CONFIG += qt warn_on

include(config.pri)

LIBS += -lbb -lbbsystem -lbbdata -lbbnetwork -lunifieddatasourcec -lbbpim -lbbplatform

QT += network sql