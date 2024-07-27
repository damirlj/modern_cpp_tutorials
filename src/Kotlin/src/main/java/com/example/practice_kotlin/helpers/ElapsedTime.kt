package com.example.practice_kotlin.helpers

import kotlinx.coroutines.runBlocking
import kotlin.reflect.KFunction


private suspend fun <R> elapsed(f: suspend () -> R): Pair<R, Long> {
    val start = System.currentTimeMillis()
    val result = f()
    return Pair(result, System.currentTimeMillis() - start)
}

suspend fun <R> elapsedAny(f: KFunction<R>, vararg args : Any?): Pair<R, Long> {
    return elapsed { f.call(*args) }
}

/**
 * For measuring the elapsed time of a synchronous function.
 * It will be turned into blocking call - waiting on the function to be
 * executed within the coroutine context
 */
fun <R> func_elapsed(f: () -> R): Pair<R, Long> {
    val result: Pair<R, Long>
    runBlocking {
        result = elapsed(f) // suspension point
    }
    return result
}

/**
 * For measuring elapsed time of a suspend call
 */
suspend fun <R> coro_elapsed(f: suspend () -> R): Pair<R, Long> = elapsed(f)

