/**
 * @author: Damir Ljubic
 * @email: damirlj@yahoo.com
 *     <p>All rights reserved!
 */
package <your package>;

import androidx.annotation.NonNull;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

public class AOThreadBase {

  // Single-thread execution context
  private final ExecutorService executionContext = Executors.newSingleThreadExecutor();
  private final Looper looper = new Looper();

  /** Start the AOT: Attach the Looper for the execution context */
  public void start() {
    executionContext.submit(looper.getRunnable());
  }

  @FunctionalInterface
  private interface IStopThread {
    void stop(@NonNull ExecutorService executor);
  }

  /** Stop thread which drain the message queue */
  private void stop(@NonNull IStopThread callback) {
    looper.stop(); // set the exit flag
    callback.stop(executionContext);
  }

  /**
   * Try to stop the background AOT.
   *
   * @param timeout The timeout to wait for
   * @param unit The time unit for the timeout to be expressed with
   */
  public void stop(long timeout, TimeUnit unit) {
    stop(
        executor -> {
          executor.shutdown();
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

  public void submit(Runnable task) throws InterruptedException {
    looper.submit(task);
  }
}
