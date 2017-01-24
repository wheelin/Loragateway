#include "log.h"
#include <QStandardPaths>
#include <stdio.h>

Log Log::instance()
{
    static Log INSTANCE;
    return INSTANCE;
}

void Log::message(Log::Severity sv, bool file, const char *format, ...)
{
    char buffer[256];
    va_list args;
    va_start (args, format);
    vsnprintf (buffer,256,format, args);
    va_end (args);

    if(file)
    {
        QString msg;
        switch (sv) {
        case Severity::Error:
            msg = "ERROR    >> ";
            break;
        case Severity::Note:
            msg = "NOTE     >> ";
            break;
        case Severity::Success:
            msg = "SUCCESS  >> ";
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
    else
    {
        switch (sv) {
        case Severity::Error:
            printf(RED "ERROR    >> " RESET);
            break;
        case Severity::Note:
            printf(CYAN "NOTE     >> " RESET);
            break;
        case Severity::Success:
            printf(GREEN "SUCCESS  >> " RESET);
            break;
        default:
            break;
        }
        printf("%s\n", buffer);
    }
}

Log::Log()
{
    _default_filename = QString("%1/%2.txt")
                            .arg(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation))
                            .arg("gwLog");
}
