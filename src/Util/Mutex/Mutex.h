// Emil Hedemalm
// 2013-03-29

#ifndef AEONIC_MUTEX_H
#define AEONIC_MUTEX_H

#include "../Util.h"
#include "../../OS/OS.h"

typedef void* HANDLE;

// A Multi-platform class for handling mutexes (for locking/unlocking threads)
class Mutex {
private:
	Mutex(const Mutex & copy);
public:
	/// Doesn't do anything
	Mutex();
	/// Calls Destroy if needed.
	~Mutex();

/// Hide them if incorrect OS..!
#ifdef WINDOWS
	/// Creates a mutex with given name in the OS and opens it for usage.
	bool Create(String name);
	/** Attempts to claim the mutex. If milliseconds is -1 it will wait indefinitely
		until it manages to retrieve it. */
	bool Claim(int milliseconds);
	/** Releases the claimed mutex. */
	bool Release();
	/// Deallocate/free resources for this mutex and closes it.
	bool Destroy();


    /// Linux functions, same names!
#elif defined LINUX | defined OSX
    /// Creates, asserts if fails, run once
    bool Create(String name);
    /// Asserts if fails, waits forever. The milliseconds argument is not implemented at this time, but when it is, a -1 will refer to infinite.
    bool Claim(int milliseconds);
    /// Asserts if fails, releases
    bool Release();
    /// Deallocates, assets if fails srun once
    bool Destroy();
#endif

    const String GetName() const { return name; };

private:

#ifdef WINDOWS
	/// Unneccessary to have these public..
	/// Opens the mutex for handling. This is required for any further operations
	bool Open();
	/// Releases the mutex name for handling
	bool Close();
#endif

    bool isCreated;
	bool isOpened;
	bool isClaimed;
	bool initialized;
	String name;
	/// Enumerated id
	int id;
	static int activeMutexes;
    static bool logClaimsAndReleases;

#ifdef WINDOWS
	HANDLE                  win32MutexHandle;
#elif defined LINUX | defined OSX
    pthread_mutex_t         mutex;
#endif
};

#endif
