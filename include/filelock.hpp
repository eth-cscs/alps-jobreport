#ifndef JOBREPORT_FILELOCK_HPP
#define JOBREPORT_FILELOCK_HPP
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>
#include <errno.h>
#include <cstring>

#include "utils.hpp"

// This is a simple filelock class that uses flock to lock a file.
// This will only work on POSIX systems.
class FileLock
{
public:
    explicit FileLock(const std::string &filename) : filename(filename), fd(-1) {}

    void lock()
    {
        if (filename.empty())
        {
            raise_error("Filename cannot be empty");
        }
        
        fd = open(filename.c_str(), O_RDWR | O_CREAT, 0666);
        if (fd == -1)
        {
            raise_error("Failed to open file: " + filename);
        }

        if (flock(fd, LOCK_EX) == -1)
        {
            close(fd);
            raise_error("Failed to lock file: " + filename);
        }
    }

    void unlock()
    {
        if (fd != -1)
        {
            flock(fd, LOCK_UN);
            close(fd);
            fd = -1;
        }
    }

    ~FileLock()
    {
        unlock();
    }

private:
    std::string filename;
    int fd;
};
#endif
