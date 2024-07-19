#pragma once

#include <string>
#include <optional>
#include <sys/types.h>

class Process
{
public:
    Process(pid_t pid) : pid(pid) {}

    static std::optional<Process> FindByName(std::string name);
private:
    pid_t pid;
};