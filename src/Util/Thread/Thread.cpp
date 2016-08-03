/// Emil Hedemalm
/// 2016-07-30
/// Thread handling class. For arbitrary thread creation - parallelizing tasks - this may be useful. Used in pathfinding, loading files, etc.

#include "Thread.h"
#include "Random/Random.h"
#include "OS/Sleep.h"
#include "String/AEString.h"

int Thread::enumerator = 0;

Thread::Thread(void (*funcPtr)(void*)) : functionPointer(funcPtr)
{	
	threadID = ++enumerator;
	started = ended = false;
	stopRequested = false;
}

Thread::~Thread()
{
	/// Delete arguments dynamically created, if necessary.
	args.ClearAndDelete();
}

/// Adds an argument.
void Thread::AddArgument(Argument * arg)
{
	args.AddItem(arg);
}

/// List of dynamically created additional arguments, which may be tampered with and deleted upon thread completion.
void Thread::SetArguments(List<Argument*> additionalArgs)
{
	args = additionalArgs;
}

/// List of dynamically created additional arguments, which may be tampered with and deleted upon thread completion.
void Thread::Run(List<Argument*> additionalArgs)
{
	args = additionalArgs;
	Run();
}

/// Runs the thread.
void Thread::Run()
{
	if (started)
		return;
#ifdef WINDOWS 
	// https://msdn.microsoft.com/en-us/library/kdzttdcb.aspx?f=255&MSPPError=-2147217396
	Argument ** argsArray = new Argument * [args.Size() + 1];
	argsArray[0] = (Argument *) this;
	for (int i = 0; i < args.Size(); ++i)
	{
		argsArray[i+1] = args[i];
	}
	// Each argument will be a pointer to something arbitrary, to be deleted by
	threadHandle = _beginthread(functionPointer, 0, argsArray); // 2nd arg is stacksize, 3rd is argument list.
#elif defined LINUX | defined OSX
	pthread_create(&threadHandle, NULL, classAndFunctionName, NULL);
#endif
	started = true;
}

/// Stops the thread prematurely (if possible).
void Thread::Stop()
{
	if (!started)
		return;
	stopRequested = true;
}

/// Checks if the thread has ended.
bool Thread::HasEnded()
{
	return ended;
}

/// From thread function, call this if you delete the provided arguments yourself.
void Thread::ArgsDeleted()
{
	args.Clear();
}

THREAD_DECLARATION(ThreadTestSample)

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
