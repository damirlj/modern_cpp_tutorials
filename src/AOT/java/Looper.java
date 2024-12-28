/**
 * @author: Damir Ljubic
 * @email: damirlj@yahoo.com
 *     <p>All rights reserved!
 */
package de.esolutions.airplay.helper.aot;

import org.jetbrains.annotations.NotNull;

import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;

public final class Looper {
  // Thread-safe tasks queue
  private final BlockingQueue<Runnable> queue = new LinkedBlockingQueue<>();

  private static final Runnable STOPPING_TASK = () -> {};

  /** For stopping the Looper */
  public void stop() throws InterruptedException {
    queue.put(STOPPING_TASK); // to unblock the queue and terminate gracefully
  }

  /** Looper drains the queue, and executes the tasks in order of reception (FIFO) */
  private final Runnable looper =
      () -> {
        for (; ;) {
          try {
            Runnable task = queue.take(); // blocking call
            if (task == STOPPING_TASK) break;
            task.run();
          } catch (InterruptedException e) {
            Thread.currentThread().interrupt(); // set the interrupt flag to "true"
          }
        }
        System.out.println("<Exit> Looper"); // your own logging
      };

  @NotNull
  public Runnable getLooper() {
    return looper;
  }

  public void submit(@NotNull Runnable task) throws InterruptedException {
    queue.put(task);
  }
}
