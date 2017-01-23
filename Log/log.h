#ifndef LOG_H
#define LOG_H

#include <QString>
#include <QFile>

class Log
{
public:
    static Log instance();
    void fileName(QString filename);
    void resetToDefault();

    static constexpr char BASE_DEF_FILE_NAME[] = "gwloralogs.txt";

    enum Severity
    {
        Critical,
        Major,
        Minor,
        Trivial
    };

    void message(Severity sv, const char * format, ...);

private:
    Log();
    QString _default_filename;
};

#endif // LOG_H
