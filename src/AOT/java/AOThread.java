/**
 * @author: Damir Ljubic
 * @email: damirlj@yahoo.com
 *     <p>All rights reserved!
 */
package <your package>;

import androidx.annotation.NonNull;

import java.util.concurrent.Callable;

public final class AOThread extends AOThreadBase {

  /**
   * This is for AOT for handling the jobs that don't return the value: fire-and-forget
  */
  public void enqueue(@NonNull Callable<Void> job) {
    try {
      Runnable task =
          () -> {
            try {
              job.call();
            } catch (Exception e) {
              e.printStackTrace();
            }
          };

      submit(task);

    } catch (InterruptedException e) {
      Thread.currentThread().interrupt();
    }
  }
}
