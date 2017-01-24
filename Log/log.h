#ifndef LOG_H
#define LOG_H

#include <QString>
#include <QFile>

#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define RESET   "\x1b[0m"

class Log
{
public:
    static Log instance();
    void fileName(QString filename);
    void resetToDefault();

    static constexpr char BASE_DEF_FILE_NAME[] = "gwloralogs.txt";

    enum Severity
    {
        Error,
        Note,
        Success,
    };

    void message(Severity sv, bool file, const char * format, ...);

private:
    Log();
    QString _default_filename;
};

#endif // LOG_H
