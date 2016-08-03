/// Emil Hedemalm
/// 2016-07-30
/// Thread handling class. For arbitrary thread creation - parallelizing tasks - this may be useful. Used in pathfinding, loading files, etc.

#ifndef THREAD_H
#define THREAD_H

#include "OS/OSThread.h"
#include "List/List.h"
#include "String/AEString.h"

/// Default argument. Subclass it to provide any data.
class Argument 
{
public:
	Argument() {};
	Argument(String argText) : argStr(argText) {};
	virtual ~Argument() { };
	String argStr;
};

/** For defining your own functions, there are some things to keep in mind.
	First, declare your function with 
		AE_THREAD_DECLARATION(functionName)
	Then, where you want to start its implementation, write
		AE_THREAD_START(functionName)	
	When you want to end the thread function, write
		AE_THREAD_END
	Curcly braces are NOT necessary. They are built into the AE_THREAD_START and AE_THREAD_END macros.
	This ensures that the Thread object knows that the thread has exited properly, and can save any returned data within the Thread object.
	
	Arguments are provided in the form of Argument ** args (array of Argument pointers), where the first arg is reserved (i.e. args[0]) 
	The first argument of any Thread-based functionis always this Thread class. Any other arguments - which MUST be allocated on the heap as Argument-based classes - will end up at indices 1 and onward.
	Arguments may be of other types, assuming that you deallocate all arguments within the thread, and mark it appropriately in the Thread class via caller->ArgsDeleted()
	
	AE_THREAD_START will make the Thread object available as 'caller'. You can then query if a request has been sent to Stop the thread prematurely via caller->stopRequested
*/

class Thread 
{
	static int enumerator; // 0 at first.
public:
	/// Thread, function to process, etc.
	Thread(void (*functionPointer)(void*));
	virtual ~Thread();

	/// Runs the thread.
	void Run();
	/// List of dynamically created additional arguments, which may be tampered with and deleted upon thread completion.
	void Run(List<Argument*> additionalArgs);
	/// Adds an argument.
	void AddArgument(Argument * arg);
	/// List of dynamically created additional arguments, which may be tampered with and deleted upon thread completion.
	void SetArguments(List<Argument*> additionalArgs);

	/// Stops the thread prematurely (if possible).
	void Stop();
	/// Checks if the thread has ended.
	bool HasEnded();
	int ThreadID() { return threadID;};
	/// From thread function, call this if you delete the provided arguments yourself.
	void ArgsDeleted();

	bool started;
	bool ended;
	bool stopRequested;
private:
	List<Argument*> args;
	THREAD_HANDLE threadHandle;
	void (*functionPointer)(void *);
	int threadID;

};


#define AE_THREAD_DECLARATION(functionName) THREAD_DECLARATION(functionName)
#define AE_THREAD_START(functionName) THREAD_START(functionName){ \
	Argument ** args = (Argument **)vArgs;\
	Thread * caller = (Thread*) args[0];
#define AE_THREAD_END 	delete[] args; caller->ended = true; _endthread(); }

//	std::cout<<"\nCalling thread: "<<caller->ThreadID();
//		std::cout<<"\nArgs: "<<vArgs;

/// Unit test for the class. Return true to quick the program (error in unit test).
bool ThreadTest();

/** Sample implementation below

/// Unit test for the class.
bool ThreadTest()
{
	std::cout<<"\nThreadTest";
	List<Thread*> threads;
	for (int i = 0; i < 50; ++i)
	{
		Thread * t = new Thread(ThreadTestSample);
		t->AddArgument(new Argument("Lallilall "+String(i)));
		t->Run();
		t->ArgsDeleted(); // Flag that we will delete all our added arguments within the thread itself.
		if (i % 5 == 0)
			t->Stop();
		threads.AddItem(t);
	}
	for (int i = 0; i < threads.Size(); ++i)
	{
		Thread * t = threads[i];
		while(!t->HasEnded());
	}
	/// Deallocate.
	threads.ClearAndDelete();
	return false;
}

AE_THREAD_START(ThreadTestSample)
	std::cout<<"\nOwn argument: "<<args[1]->argStr;
	delete args[1];
	Random rand;
	int amount = rand.Randi() % 1000;
	std::cout<<" amount: "<<amount;
	float sum = 0;
	for (int i = 0; i < amount; ++i)
	{
		float p = 1 / (float) amount;
		SleepThread(5);
		sum += p;
		if (caller->stopRequested)
		{
			std::cout<<"\nStop requested in thread "<<caller->ThreadID()<<" ending prematurely.";
			goto end;
		}
	}
end:
	std::cout<<"\nThread "<<caller->ThreadID()<<"has finished. Sum: "<<sum;
AE_THREAD_END


*/

#endif
