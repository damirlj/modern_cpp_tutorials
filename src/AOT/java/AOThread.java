/**
 * @author: Damir Ljubic
 * @email: damirlj@yahoo.com
 *     <p>All rights reserved!
 */
package <your package>;

import org.jetbrains.annotations.NotNull;

import java.util.Optional;
import java.util.concurrent.Callable;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

public final class AOThread {

  // Single-thread execution context
  private final ExecutorService executionContext = Executors.newSingleThreadExecutor();
  private final Looper looper = new Looper();

  /** Start the AOT: Attach the Looper with the execution context */
  public void start() {
    executionContext.submit(looper.getRunnable());
  }

  @FunctionalInterface
  private interface IStopThread {
    void stop(@NotNull ExecutorService executor);
  }

  /** Stop thread which drain the message queue */
  private void stop(@NotNull IStopThread callback) {
    looper.stop(); // set the exit flag
    callback.stop(executionContext);
  }

  /**
   * Try to stop the background AOT gracefully, by waiting on the last task completion
   *
   * @param timeout The timeout to wait for
   * @param unit The time unit for the timeout to be expressed with
   */
  public void stop(long timeout, TimeUnit unit) {
    stop(
        executor -> {
          executor.shutdown(); // initiate shutdown
          try {
            if (!executor.awaitTermination(timeout, unit)) {
              executor.shutdownNow();
            }
          } catch (InterruptedException e) {
            executor.shutdownNow();
            Thread.currentThread().interrupt();
          }
        });
  }

  /**
   * Stop the AOT background thread immediately, without waiting on the task completion
   *
   * @note There is no guarantees that the last running task will be successfully interrupted
   */
  public void stopNow() {
    stop(ExecutorService::shutdownNow);
  }

  public void submit(@NotNull Runnable task) throws InterruptedException {
    looper.submit(task);
  }

  /**
   * Submit the job to execution context. The client will be synchronized with the result of the
   * execution.
   *
   * @param job The task to be enqueued
   * @return The future object - for synchronizing on the execution outcome
   * @param <R> The result type
   */
  @NotNull
  public <R> Optional<CompletableFuture<R>> enqueueWithResult(@NotNull Callable<R> job) {
    CompletableFuture<R> f = new CompletableFuture<>();

    // Wrap the callable into parameterless runnable task
    Runnable task =
        () -> {
          try {
            R result = job.call();
            f.complete(result); // signal the result to client
          } catch (Exception e) {
            f.completeExceptionally(e); // or signal the exception to client
          }
        };

    try {
      submit(task);
    } catch (InterruptedException e) {
      Thread.currentThread().interrupt();
      return Optional.empty();
    }

    return Optional.of(f);
  }

  @FunctionalInterface
  public interface IJob {
    void execute() throws Exception;
  }

  /**
   * Submit the job to the execution context. <br>
   * Fire-and-forget: client will not wait on the outcome of execution, if any.
   *
   * @param job The task to be executed
   */
  public void enqueue(@NotNull IJob job) {
    Runnable task = ()->{
      try{
        job.execute();
      }catch(Exception e) {
        e.printStackTrace(); // your own logging
      }
    };

    try {
      submit(task);
    } catch (InterruptedException e) {
      Thread.currentThread().interrupt();
    }
  }
}
