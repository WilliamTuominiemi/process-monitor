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
            ifstream cmdlineFile(cmdlinePath);

            if (cmdlineFile.is_open())
            {
                string cmdline;
                getline(cmdlineFile, cmdline);
                cmdlineFile.close();

                replace(cmdline.begin(), cmdline.end(), '\0', ' ');

                if (!cmdline.empty())
                {
                    processes.push_back({cmdline, pid});
                }
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
        cout << "PID: " << process.pid << ", Name: " << short_name << endl;
    }

    return 0;
}