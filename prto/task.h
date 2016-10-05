#ifndef TASK_H
#define TASK_H

#include <QString>

class Task
{
public:
    enum TaskType {Zoz = 1, Vs = 2};

    bool Enabled;
    int Type;
    float Number;
    QString Params;
    QString Path;
    int Id;

    Task();
    ~Task();
    void clear();
};

#endif // TASK_H
