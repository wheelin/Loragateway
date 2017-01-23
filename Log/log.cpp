#include "log.h"
#include <QStandardPaths>
#include <stdio.h>

Log Log::instance()
{
    static Log INSTANCE;
    return INSTANCE;
}

void Log::message(Log::Severity sv, const char *format, ...)
{
    char buffer[256];
    va_list args;
    va_start (args, format);
    vsnprintf (buffer,256,format, args);
    va_end (args);
    QString msg;
    switch (sv) {
    case Severity::Critical:
        msg = "CRITICAL >> ";
        break;
    case Severity::Major:
        msg = "MAJOR    >> ";
        break;
    case Severity::Minor:
        msg = "MINOR    >> ";
        break;
    case Severity::Trivial:
        msg = "TRIVIAL  >> ";
        break;
    default:
        break;
    }
    msg.append(buffer);
    QFile f(_default_filename);
    if(!f.open(QIODevice::Append))
    {
        printf("ERROR >> cannot open log file\n");
        return;
    }
    f.write(msg.toLocal8Bit());
    f.close();
}

Log::Log()
{
    _default_filename = QString("%1/%2.txt")
                            .arg(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation))
                            .arg("gwLog");
}
