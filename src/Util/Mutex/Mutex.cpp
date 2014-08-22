// Emil Hedemalm
// 2013-03-29

#include "Mutex.h"
#include "../Timer/Timer.h"

#ifdef WINDOWS
	#include <Windows.h>
#endif
#if defined LINUX | defined OSX
    #include <pthread.h>
    #define PRINT_THREAD_NAME std::cout<<"\nThread "<<name<<": "
#endif

// A Multi-platform class for handling mutexes (for locking/unlocking threads)
//class Mutex {
//public:
int Mutex::activeMutexes = 0;
bool Mutex::logClaimsAndReleases = false;

/// Doesn't do anything
Mutex::Mutex(){
    isCreated = false;
	isOpened = false;
	isClaimed = false;
	initialized = false;
	id = activeMutexes++;
    #ifdef WINDOWS
        win32MutexHandle = NULL;
	#endif
}

/// Calls Destroy.
Mutex::~Mutex()
{
	if (isCreated)
		Destroy();
}


#if defined LINUX | defined OSX
/// Creates, asserts if fails, run once
bool Mutex::Create(String i_name){
    assert(!isCreated);
    name = i_name;
    PRINT_THREAD_NAME << " attempting to create mutex...";
    int res = pthread_mutex_init(&mutex, NULL);
    assert(res == 0);
    isCreated = true;
}
/// Asserts if fails, waits forever
bool Mutex::Claim(int milliseconds){
    assert(isCreated);
    if (logClaimsAndReleases)
        PRINT_THREAD_NAME << " attempting to claim mutex...";
    int res = pthread_mutex_lock(&mutex);
    assert(res == 0);
    isClaimed = true;
    assert(isClaimed);
}
/// Asserts if fails, releases
bool Mutex::Release(){
    assert(isCreated);
  //  assert(isClaimed);
    if (logClaimsAndReleases)
        PRINT_THREAD_NAME << " attempting to release mutex...";
    int res = pthread_mutex_unlock(&mutex);
    assert(res == 0);
    isClaimed = false;
}
/// Deallocates, assets if fails srun once
bool Mutex::Destroy(){
    assert(isCreated);
    PRINT_THREAD_NAME << " attempting to destroy mutex...";
    int res = pthread_mutex_destroy(&mutex);
    assert(res == 0);
    isCreated = false;
}

#elif defined WINDOWS

/// Create a mutex
bool Mutex::Create(String i_name){
	i_name.ConvertToWideChar();
	win32MutexHandle = CreateMutex(NULL, false, i_name);
	if (!win32MutexHandle){
		int error = GetLastError();
		if (error != NO_ERROR){
			assert(false && "\nError when creating mutex:" && error);
			return false;
        }
	}
	this->name = i_name;
	isCreated = true;
	Open();
	return true;
}

/// Retrieves the mutex name for handling. This is required for any further operations
bool Mutex::Open(){
    assert(isCreated);
    assert(!this->isOpened && "Trying to open mutex that is already opened!");
        win32MutexHandle = OpenMutex(MUTEX_ALL_ACCESS, FALSE, name);
        if (!win32MutexHandle){
            int error = GetLastError();
            assert(error && "\nUnable to open physics mutex ");
            return false;
        }
    this->isOpened = true;
    return true;
}
/// Releases the mutex name for handling
bool Mutex::Close(){
    assert(isCreated);
	assert(this->isOpened && "Trying to close mutex that wasn't opened properly!");
	int result;
	result = CloseHandle(win32MutexHandle);
	if (result){
        this->isOpened = false;
		win32MutexHandle = NULL;
	}
    else
        assert(false && "Unable to Close mutex!");
    return result != 0;
}
/** Attempts to claim the mutex. If milliseconds is -1 it will wait indefinitely
	until it manages to retrieve it.
*/
bool Mutex::Claim(int milliseconds)
{
	assert(this->isOpened);
	int result;
	int i = INFINITE;
	result = WAIT_TIMEOUT;
	while(result == WAIT_TIMEOUT)
	{
		result = WaitForSingleObject(this->win32MutexHandle, milliseconds);
		if (result == WAIT_FAILED || result == WAIT_TIMEOUT){
			result = GetLastError();
			assert("Waiting for PhysicsMessageQueueMutex failed!" && result && false);
			return false;
		}
	}
	isClaimed = true;
	return true;
}
/** Releases the claimed mutex.
*/
bool Mutex::Release()
{
	assert(this->isOpened);
//	assert(isClaimed == true);
//	assert(this->isClaimed && "Trying to release unclaimed mutex!");
	int result;
	result = ReleaseMutex(win32MutexHandle);
    if (result)
        this->isClaimed = false;
    else {
        assert(false && "Unable to release mutex!");
    }
	isClaimed = false;
	assert(isClaimed == false);
    return result != 0;
}

/// Deallocate/free resources for this mutex.
bool Mutex::Destroy(){
	assert(isCreated);
	Close();
	isCreated = false;
	return true;
}


#endif // WINDOWS
