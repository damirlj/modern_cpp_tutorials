package com.example.practice_kotlin.coroutines

import kotlinx.coroutines.CoroutineDispatcher
import kotlinx.coroutines.CoroutineName
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Job
import kotlinx.coroutines.async
import kotlinx.coroutines.coroutineScope


class CoroutinesWrapper(dispatcher: CoroutineDispatcher) :
    CoroutineScope, AutoCloseable {

    // Job management - to ensure that all jobs launched within CoroutinesWrapper
    // will be properly closed - by calling job.cancel()
    private val job = Job()
    // Specifies the environment in which the coroutines will be executed (scheduler, name and other custom attributes)
    override val coroutineContext = dispatcher + job + CoroutineName("CoroutinesWrapper")

    /**
     * Turn the callable into (deferred) coroutine, that
     * will be executed within the specified CoroutineScope - within
     * the given thread/thread pool context specified by _dispatcher_ parameter of the c-tor, while
     * calling coroutine remains suspended
     */
    private suspend fun <R> func2coroutine(f: () -> R): R = coroutineScope {
        val coro = async { f() } // coroutine builder: factory method to start the coroutine within the given scope
        coro.await()
    }

    /**
     * Reference implementation {@link CoroutinesWrapper#func2coroutine}
     * @param f Regular function that is turned into deferred coroutine
     * @return Result of the regular function, of type R
     */
    suspend fun <R> await(f: () -> R): R {
        return func2coroutine(f)
    }

    /**
     * high-order function
     * Providing the thread/thread pool context (CoroutineScope) for the coroutine
     * that will be executed, while suspending the calling coroutine
     *
     * @param f Coroutine to be executed
     * @return The result of the coroutine, of type R
     */
    suspend fun <R> awaitCoroutine(f: suspend () -> R): R {
        return coroutineScope { f() }
    }

    override fun close() {
        job.cancel()
    }
}