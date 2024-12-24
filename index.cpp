#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <dirent.h>
#include <unistd.h>
#include <algorithm>

using namespace std;

struct ProcessInfo
{
    string name;
    int pid;
    string state;
    long memoryUsage;
};

vector<ProcessInfo> getRunningProcesses()
{
    vector<ProcessInfo> processes;

    DIR *dir = opendir("/proc");
    if (dir == nullptr)
    {
        cerr << "Error opening /proc directory" << endl;
        return processes;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr)
    {
        if (isdigit(entry->d_name[0]))
        {
            int pid = stoi(entry->d_name);

            string cmdlinePath = "/proc/" + to_string(pid) + "/cmdline";
            string statusPath = "/proc/" + to_string(pid) + "/status";

            ifstream cmdlineFile(cmdlinePath);
            ifstream statusFile(statusPath);

            string cmdline, state;
            long memoryUsage = 0;

            if (cmdlineFile.is_open())
            {
                getline(cmdlineFile, cmdline);
                cmdlineFile.close();
                replace(cmdline.begin(), cmdline.end(), '\0', ' ');
            }

            if (statusFile.is_open())
            {
                string line;
                while (getline(statusFile, line))
                {
                    if (line.find("State:") == 0)
                    {
                        state = line.substr(line.find(":") + 2);
                    }
                    else if (line.find("VmRSS:") == 0)
                    {
                        memoryUsage = stol(line.substr(line.find(":") + 2));
                    }
                }
                statusFile.close();
            }

            if (!cmdline.empty())
            {
                processes.push_back({cmdline, pid, state, memoryUsage});
            }
        }
    }

    closedir(dir);
    return processes;
}

int main()
{
    vector<ProcessInfo> processes = getRunningProcesses();

    cout << "Running Processes:" << endl;
    for (const auto &process : processes)
    {
        string short_name = "";
        auto pos = process.name.find_last_of('/');
        if (pos != string::npos)
        {
            short_name = process.name.substr(pos);
        }

        cout << "PID: " << process.pid
             << ", Name: " << short_name
             << ", State: " << process.state
             << ", Memory Usage: " << process.memoryUsage << " kB" << endl;
    }

    return 0;
}
