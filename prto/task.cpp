#include "task.h"


Task::Task()
{
    clear();
}

void Task::clear()
{
    Enabled = true;
    Type = 0;
    Number = 0;
    Params.clear();
    Path.clear();
    Id = -1;
}
// ----------------------------------- Деструктор ----------------------------------- //
Task::~Task()
{
}
