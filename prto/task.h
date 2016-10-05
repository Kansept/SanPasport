#ifndef TASK_H
#define TASK_H

#include <QString>

namespace TaskType
{
    enum TaskType {
        Zoz = 1,
        Vs = 2,
        Point = 3
    };
}

class Task
{
public:
    bool Enabled;
    int Type;
    float Sort;
    QString Params;
    QString Path;
    int Id;

    Task();
    ~Task();
    void clear();
};

#endif // TASK_H
