#ifndef HCARCS_UTILS_H
#define HCARCS_UTILS_H

#define EXTRA_DEBUG_ON 1

#define DEBUG_W_QT 1

#if DEBUG_W_QT
#include <QDebug>
#define DEBUG_OUT qDebug().nospace()
#define DEBUG_OUT_END << ""
#else
#include <iostream>
#include <list>
#define DEBUG_OUT std::cout
#define DEBUG_OUT_END << std::endl
#endif

enum direction
{
    C, //clockwise
    CC //counterclockwise
};

// log levels
enum llevel
{
    OUTPUT,
    ERROR,
    INFO,
    DEBUG,
    EXTRA_DEBUG
};

class Interval;
class Arc;

void log_data(Interval *intervalArray, int arraySize, llevel level);
void log_data(Interval **intervalPtrArray, int arraySize, llevel level);
void log_data(Arc *arcArray, int arraySize, llevel level);
void log_data(Arc **arcPtrArray, int arraySize, llevel level);
void log_data(const std::list<int> &list, llevel level);
void log_data(const std::list<std::list<int>> &list_of_lists, llevel level);
void log_data(int *indexesArray, int arraySize, llevel level);

void log_msg(const std::string &msg, llevel level);
void log_dashedLine();
void log_asteriskLine();

// run checks in O(path.size). arraySize is for printing
bool check_path_intersection(Interval *intervalArray, int arraySize, std::list<int> path);
bool check_path_intersection(Arc *arcArray, int arraySize, std::list<int> path);

class EasyLogger : public QObject
{
    Q_OBJECT

public:
    static EasyLogger* getInstance();

    EasyLogger(EasyLogger &other) = delete;
    void operator=(const EasyLogger &other) = delete;

signals:
    void logMessage(const QString &msg);

protected:
    EasyLogger() {}
    static EasyLogger *mp_singleton;
};



#endif // HCARCS_UTILS_H
