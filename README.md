# C++ Concurrent Programming
Using the deferred option of std::sync allows us to delay the execution of the controller task until we receive input indicating a key has been hit. 
This eliminates the need for condition variables, mutexes, and other synchronization mechanisms in the controller task.
