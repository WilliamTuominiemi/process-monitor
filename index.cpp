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
    long cpuUsage;
};

long getSystemUptime()
{
    ifstream uptimeFile("/proc/uptime");
    if (!uptimeFile.is_open())
    {
        cerr << "Error opening /proc/uptime" << endl;
        return -1;
    }
    double systemUptime = 0.0;
    uptimeFile >> systemUptime;
    uptimeFile.close();
    return systemUptime;
}

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
            string statPath = "/proc/" + to_string(pid) + "/stat";
            string uptimePath = "/proc/uptime";

            ifstream cmdlineFile(cmdlinePath);
            ifstream statusFile(statusPath);
            ifstream statFile(statPath);
            ifstream uptimeFile(uptimePath);

            string cmdline, state;
            long memoryUsage = 0, userTime = 0, kernelTime = 0, startTime = 0, systemUptime = 0, cpuUsage = 0;

            systemUptime = getSystemUptime();

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

            if (statFile.is_open())
            {
                string line;
                getline(statFile, line);
                statFile.close();

                vector<string> fields;
                size_t pos = 0;
                while ((pos = line.find(" ")) != string::npos)
                {
                    fields.push_back(line.substr(0, pos));
                    line.erase(0, pos + 1);
                }

                if (fields.size() > 21)
                {
                    userTime = stol(fields[13]);
                    kernelTime = stol(fields[14]);
                    startTime = stol(fields[21]);
                }

                long hertz = sysconf(_SC_CLK_TCK);
                double processCPUTime = (userTime + kernelTime) / (double)hertz;
                double elapsedTime = systemUptime - (startTime / (double)hertz);

                if (elapsedTime > 0)
                {
                    int numCPUs = sysconf(_SC_NPROCESSORS_ONLN);
                    cpuUsage = (processCPUTime / elapsedTime) * 100.0 / numCPUs;
                }
            }

            if (!cmdline.empty() && cpuUsage > 0)
            {
                processes.push_back({cmdline, pid, state, memoryUsage, cpuUsage});
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
             << ", Memory Usage: " << process.memoryUsage << " kB"
             << ", CPU Usage: " << process.cpuUsage << " %"
             << endl;
    }

    return 0;
}
