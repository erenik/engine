// Emil Hedemalm
// 2013-03-29

#ifndef AEONIC_MUTEX_H
#define AEONIC_MUTEX_H

#include "../Util.h"
#include "../../OS/OS.h"

typedef void* HANDLE;
class Mutex;

/** A RAII-based Mutex wrapper class. 
	Intended use is to create a MutexHandle with given mutex at the start of the intended closed execution code,
	and then when the function exists, the MutexHandle is de-allocated automatically (assuming you create it on the Stack)
	making the Mutex release no matter how you return, without having to explicitly call Mutex.Release();
*/
class MutexHandle 
{
public:
	MutexHandle(Mutex & mutex);
	~MutexHandle();
private:
	Mutex * mutex;
};

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
	bool Claim(int milliseconds = -1);
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

	bool IsCreated() const { return isCreated; };

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
