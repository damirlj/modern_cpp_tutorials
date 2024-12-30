/**
 * @author: Damir Ljubic
 * @email: damirlj@yahoo.com
 *     <p>All rights reserved!
 */
package <your own package>;

import androidx.annotation.NonNull;

import org.jetbrains.annotations.NotNull;
import org.junit.Test;

import java.util.ArrayList;
import java.util.List;
import java.util.Optional;
import java.util.concurrent.Callable;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ExecutionException;
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

  private <R> void fetchResultOrException(
      @NonNull Future<R> result, @NotNull Consumer<R> callback) {
    try {
      callback.accept(result.get()); // blocking call: wait on result being set
    } catch (ExecutionException | InterruptedException e) { // or exception being thrown
      e.printStackTrace();
    }
  }

  private <R> void syncOnResults(@NotNull List<Future<R>> results, @NotNull Consumer<R> callback) {
    for (Future<R> result : results) {
      fetchResultOrException(result, callback);
    }
  }

  @NotNull
  private Callable<String> createCallable(long timeout) {
    return () -> {
      Thread.sleep(timeout); // simulate some work
      return String.format( // it's always hosted by the same thread - execution context
          "(thread: %s): Sleeping for %d[ms]", Thread.currentThread().getName(), timeout);
    };
  }

  @Test
  public void test_AOTWaitingOnResults() throws InterruptedException {
    final AOThread aot = new AOThread();
    aot.start();

    Thread client =
        new Thread(
            () -> {
              final List<Callable<String>> callables =
                  List.of(createCallable(200L), createCallable(100L), createCallable(300L));
              final List<Future<String>> results = submitTasks_withResults(callables, aot);
              // Client: synchronized on the result
              syncOnResults(results, System.out::println);
            },
            "t_client");

    client.start();
    client.join();

    aot.stopNow();
  }

  @NotNull
  private AOThread.IJob createJob(long timeout) {
    return () -> {
      Thread.sleep(timeout); // simulate some work
      System.out.printf(
          "Executing within thread: \"%s\", after %d[ms]\n",
          Thread.currentThread().getName(), timeout);
    };
  }

  @Test
  public void test_AOTWithoutWaitingOnResults() throws InterruptedException {

    final AOThread aot = new AOThread();
    aot.start();

    Thread client =
        new Thread(
            () -> {
              final List<AOThread.IJob> callables = List.of(createJob(200L), createJob(100L));
              for (AOThread.IJob callable : callables) {
                aot.enqueue(callable);
                try {
                  Thread.sleep(1000);
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

  @Test
  public void test_bothOfThem() throws InterruptedException {
    final AOThread aot = new AOThread();
    aot.start();

    Thread client =
        new Thread(
            () -> {
              // Submit, without waiting
              aot.enqueue(createJob(100L));
              aot.enqueue(() -> {}); // this will not brake the looper

              // Wait on result, or exception being set
              Optional<CompletableFuture<String>> opFut =
                  aot.enqueueWithResult(createCallable(200L));
              opFut.ifPresent(
                  fut ->
                      fetchResultOrException(
                          fut,
                          res ->
                              System.out.printf(
                                  "(Thread: \"%s\") Receiving result: %s\n",
                                  Thread.currentThread().getName(), res)));
            },
            "t_client");

    client.start();
    client.join();

    aot.stopNow();
  }
}
