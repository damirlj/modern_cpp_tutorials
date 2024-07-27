package com.example.practice_kotlin.coroutines

import kotlinx.coroutines.*
import kotlinx.coroutines.test.runTest
import org.junit.Assert.assertEquals
import org.junit.Test


class TestCoroutines {

    private suspend fun simulateWork(wrapper: CoroutinesWrapper, timeoutInMs: Long): Long {
        return wrapper.awaitCoroutine {
            delay(timeoutInMs)
            timeoutInMs
        }
    }

    @OptIn(ExperimentalCoroutinesApi::class)
    @Test
    fun test_coroutineAwait() = runTest {
        CoroutinesWrapper(Dispatchers.Default).use { wrapper ->
            val timeoutInMs = 1000L
            val result = simulateWork(wrapper, timeoutInMs)
            println("Timeout: $result[ms]")
            assertEquals(result, timeoutInMs)
        }
    }

    @OptIn(ExperimentalCoroutinesApi::class)
    @Test
    fun test_coroutineAwaitNoResult() = runTest {
        CoroutinesWrapper(Dispatchers.Default).use { wrapper ->
            wrapper.awaitCoroutine {
                delay(1000L)
                println("Inside coroutine")
            }
        }
    }

    @OptIn(ExperimentalCoroutinesApi::class)
    @Test
    fun test_regularFunctionAwait() = runTest {
        CoroutinesWrapper(Dispatchers.Default).use { wrapper ->
            val timeoutInMs = 1000L
            val result = wrapper.await{
                Thread.sleep(timeoutInMs)
                timeoutInMs
            }
            assertEquals(result, timeoutInMs)
        }
    }
}

