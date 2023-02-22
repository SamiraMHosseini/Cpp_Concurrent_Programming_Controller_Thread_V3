#include <conio.h>
/*
we don't need this anymore!
struct ControllerResource
{
	ControllerResource() :
		mtx(), cv(), controllerFlag(false)
	{

	}

	std::mutex mtx;
	std::condition_variable cv;
	bool controllerFlag;
};

*/
struct SharedResource
{
	SharedResource() :
		cv(), mtx(), killflag(false)
	{

	}
	//These are resources that are shared between threads

	std::condition_variable cv;
	std::mutex mtx;
	bool killflag;
};
class A : public BannerBase
{

public:
	A() = delete;
	A(const A&) = default;


	A& operator=(const A&) = default;
	~A() = default;
	A(const char* const name) :
		BannerBase(name), counter(0)
	{

	}
	A(A&&) = default;

	void operator ()(SharedResource& sharedResource) //Functor 
	{
		START_BANNER;

		while (true)
		{

			std::unique_lock<std::mutex> lock(sharedResource.mtx);
			//Wait for 100ms unless there is an interrupt or a signal
			if (sharedResource.cv.wait_for(lock, 100ms, [&]() -> bool { return sharedResource.killflag; }))
			{
				break;
			}

			Debug::out("%d \n", this->counter);
			++(this->counter);


		}

	}

private:
	int counter;

};
class B : public BannerBase
{
public:
	B() = delete;
	B& operator=(const B&) = default;
	B(const B&) = default;
	~B() = default;
	B(const char* const name) :
		BannerBase(name), counter(0x10000)
	{

	}
	void operator()(SharedResource& sharedResource)
	{
		START_BANNER;

		while (true)
		{

			std::unique_lock<std::mutex> lock(sharedResource.mtx);
			//Wait for 10ms unless there is an interrupt or a signal
			bool flag = sharedResource.cv.wait_for(lock, 10ms, [&]() -> bool {return sharedResource.killflag; });
			if (flag)
			{
				break;
			}
			Debug::out("0x%x \n", this->counter);
			this->counter = this->counter - 1;

		}
	}

private:
	unsigned int counter;


};

class C : public BannerBase
{

public:

	C() = delete;
	C(const C&) = default;
	C& operator=(const C&) = default;
	~C() = default;
	C(const char* const name) :
		BannerBase(name), strings{ "apple", "orange" ,"banana", "lemon" }
	{

	}

	void operator() (SharedResource& sharedResource)
	{

		START_BANNER;
		int index{ 0 };
		while (true)
		{

			std::unique_lock<std::mutex> lock(sharedResource.mtx);
			//Wait for 1ms unless there is an interrupt or a signal
			if (sharedResource.cv.wait_for(lock, 1ms, [&]()->bool { return sharedResource.killflag; }))
			{
				break;
			}
			Debug::out("%s \n", this->strings[index]);
			++index;
			if (index > 3)
			{
				index = 0;
			}


		}
		//std::this_thread::sleep_for(1ms);
		std::this_thread::sleep_for(2s);
	}

private:


	const char* const strings[4];
};

class D : public BannerBase
{
public:
	D() = delete;
	D(const D&) = default;
	D& operator=(const D&) = default;
	virtual ~D() = default;

	D(const char* const name) :
		BannerBase(name)
	{



	}
	void operator() (SharedResource& sharedReference)
	{
		START_BANNER;
		const char* const sourceStr = "<0><1><2><3><4><5><6><7><8>";
		char* destStr = new char[strlen(sourceStr) + 1];
		memcpy(destStr, sourceStr, strlen(sourceStr) + 1);
		size_t len{ strlen(sourceStr) };
		size_t  tempLen = len;
		while (true)
		{

			std::unique_lock<std::mutex> lock(sharedReference.mtx);
			bool flag = sharedReference.cv.wait_for(lock, 0.1ms, [&]()->bool {return sharedReference.killflag; });
			if (flag)
			{
				break;
			}
			if (strlen(destStr) <= 0)
			{

				memcpy(destStr, sourceStr, len);
				tempLen = len;
			}
			Debug::out("%s \n", destStr);
			destStr[--tempLen] = '\0';
			destStr[--tempLen] = '\0';
			destStr[--tempLen] = '\0';

		}

		delete destStr;
	}




};

class Controller : public BannerBase
{
public:
	Controller() = delete;
	Controller(const Controller&) = default;
	Controller& operator=(const Controller&) = default;
	~Controller() = default;
	Controller(const char* const name) :
		BannerBase(name) {}
	void operator()(SharedResource& sharedResource)
	{

		START_BANNER;

		//std::unique_lock<std::mutex> controllerLock(controllerResource.mtx);
		//I used a predicate in the wait method of the condition variable to prevent spurious calls
		//controllerResource.cv.wait(controllerLock, [&]() -> bool { return controllerResource.controllerFlag; });
		std::unique_lock<std::mutex> lock(sharedResource.mtx);
		Debug::out("I have been notified...\n");
		sharedResource.killflag = true;
		lock.unlock();
		sharedResource.cv.notify_all();

	}
};

void f()
{
	while (true)
	{
		Debug::out("F\n");
	}

}
void g()
{
	Debug::out("G\n");
}




int main()
{
	START_BANNER_MAIN("Main");





	A a("A");
	B b("B");
	C c("C");
	D d("D");
	Controller controller("Controller");


	//Resources shared between threads 
	SharedResource sharedResource;
	//ControllerResource controllerResource;


	//Launch tasks
	std::future<void> taskA = std::async(std::launch::async, std::move(a), std::ref(sharedResource));
	std::future<void> taskB = std::async(std::launch::async, std::move(b), std::ref(sharedResource));
	std::future<void> taskC = std::async(std::launch::async, std::move(c), std::ref(sharedResource));
	std::future<void> taskD = std::async(std::launch::async, std::move(d), std::ref(sharedResource));

	std::future<void> taskController = std::async(std::launch::deferred, controller,
		std::ref(sharedResource));


	//Key Press
	_getch();

	//Signal to the tasks

	Debug::out("a key is pressed\n");
	taskController.get();

	/*
	When a thread is waiting using a condition variable,
	it is not checking the flag,
	but it is also not consuming any CPU time.
	Instead, the thread is in a sleep state, waiting for a notification from another thread that the condition has changed.

	*/

	//std::this_thread::sleep_for(1s);





}



