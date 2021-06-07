# Introduction

These are some collection of working concepts that served me as 
teaching platform - for exploring the new features introduced by the latest C++ standards.
Some of them were starting point in implementation of various utility 
classes and libraries in real projects, across different platforms (like socket library, logging mechanism, benchmarking, etc.)
I’ve also used them to introduce the colleagues, as part of the internal discussions and knowledge transfer, the new approaches 
in realization of the well know topics, like concurrency and new memory model, the power of callable objects and 
functional programming, generics and algorithms – template metaprogramming, chrono library, etc.

Please have in mind that using the code in commercial purposes is not allowed.  
Exposing them to the community is the way for getting the valuable feedback.  
Thanks in advance.  


## Lesson1

<b>AOT – Active Object Thread</b> design pattern is introduced, as way to have asynchronous interthread communication, 
delegating the tasks to the background thread, enqueuing them into the tasks queue.
The thread drains the queue and provides the execution context, separate from the thread(s) from which 
the tasks are sent, in which they will be executed sequentially, in order of arrival (FIFO).
The caller thread can be also synchronized on result of task being executed, waiting on signaling the execution completion 
through the communication channel (future).

This concept is heavily used for interthread communication, especially for time consuming – blocking tasks that 
are delegated to the background thread in asynchronous way, where order of execution is preserved (first came, first served).
Typically, you would use this approach for <i>producer-consumer</i> scenarios

