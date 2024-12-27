/**
 * @author: Damir Ljubic
 * @email: damirlj@yahoo.com
 *     <p>All rights reserved!
 */

package <your package>;

import androidx.annotation.NonNull;

import org.jetbrains.annotations.NotNull;
import org.junit.Test;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.Callable;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;
import java.util.function.Consumer;

public class TestAOT {

  @NonNull
  private <R> List<Future<R>> submitTasks_withResults(
      @NotNull List<Callable<R>> callables, @NotNull AOThread aot) {
    List<Future<R>> results = new ArrayList<>(callables.size());
    for (Callable<R> callable : callables) {
      aot.enqueueWithResult(callable).map(results::add);
    }
    return results;
  }

  private <R> void syncOnResults_blocking(
      @NotNull List<Future<R>> results, @NotNull Consumer<R> callback) {
    for (Future<R> result : results) {
      try {
        callback.accept(result.get()); // blocking call
      } catch (Exception e) {
        e.printStackTrace();
      }
    }
  }

  private static final List<Callable<String>> callables =
      List.of(
          () -> {
            final long timeout = 400L;
            Thread.sleep(timeout);
            return String.format( // it's always hosted by the same thread - execution context
                "(thread: %s): Sleeping for %d[ms]", Thread.currentThread().getName(), timeout);
          },
          () -> {
            final long timeout = 250L;
            Thread.sleep(timeout);
            return String.format(
                "(thread: %s): Sleeping for %d[ms]", Thread.currentThread().getName(), timeout);
          },
          () -> {
            final long timeout = 500L;
            Thread.sleep(timeout);
            return String.format(
                "(thread: %s): Sleeping for %d[ms]", Thread.currentThread().getName(), timeout);
          });

  @Test
  public void test_AOTWaitingOnResults() throws InterruptedException {
    final AOThread aot = new AOThread();
    aot.start();

    Thread client =
        new Thread(
            () -> {
              final List<Future<String>> results = submitTasks_withResults(callables, aot);
              // Client: synchronized on the result
              syncOnResults_blocking(results, System.out::println);
            },
            "t_client");

    client.start();
    client.join();

    aot.stopNow();
  }

  @Test
  public void test_AOTWithoutWaitingOnResults() throws InterruptedException {

    final AOThread aot = new AOThread();
    aot.start();

    Thread client =
        new Thread(
            () -> {
              final List<AOThread.IJob> callables =
                  List.of(
                      () -> {
                        final long timeout = 300L;
                        Thread.sleep(timeout);
                        System.out.printf(
                            "Calling from thread: %s, after %d[ms]\n",
                            Thread.currentThread().getName(), timeout);
                      },
                      () -> {
                        final long timeout = 100L;
                        Thread.sleep(timeout);
                        System.out.printf(
                            "Calling from thread: %s, after %d[ms]\n",
                            Thread.currentThread().getName(), timeout);
                      },
                      () -> {
                        final long timeout = 200L;
                        Thread.sleep(timeout);
                        System.out.printf(
                            "Calling from thread: %s, after %d[ms]\n",
                            Thread.currentThread().getName(), timeout);
                      });
              
              for (AOThread.IJob callable : callables) {
                aot.enqueue(callable);
                try {
                  Thread.sleep(100);
                } catch (InterruptedException e) {
                  e.printStackTrace();
                }
              }
            },
            "t_client");

    client.start();
    client.join();

    aot.stop(1, TimeUnit.SECONDS);
  }
}
