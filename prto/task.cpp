#include "task.h"

Task::Task()
{
    clear();
}

Task::~Task()
{
}

void Task::clear()
{
    Enabled = true;
    Type = 0;
    Sort = 0;
    Params.clear();
    Path.clear();
    Id = -1;
}