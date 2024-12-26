/**
 * @author: Damir Ljubic
 * @email: damirlj@yahoo.com
 *     <p>All rights reserved!
 */
package <your package>;

import androidx.annotation.NonNull;

import java.util.Optional;
import java.util.concurrent.Callable;
import java.util.concurrent.CompletableFuture;

public final class AOThreadWithResult extends AOThreadBase {

  /**
   * Submit the job to the queue.
   *
   * @param job The job(callable) to be enqueued
   * @return The future object - for client to wait on the result.
   * @note It can return the empty optional, in case that the submission of the task is failed.
   */
  @NonNull
  public <R> Optional<CompletableFuture<R>> enqueue(@NonNull Callable<R> job) {
    CompletableFuture<R> f = new CompletableFuture<>();

    // Wrap the callable into parameterless runnable task
    Runnable task =
        () -> {
          try {
            R result = job.call();
            f.complete(result);
          } catch (Exception e) {
            f.completeExceptionally(e);
            e.printStackTrace(); // your specific logging instead
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
}
