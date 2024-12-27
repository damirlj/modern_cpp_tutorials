/**
 * @author: Damir Ljubic
 * @email: damirlj@yahoo.com
 *     <p>All rights reserved!
 */
package <your package>;

import org.jetbrains.annotations.NotNull;

import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.atomic.AtomicBoolean;

public final class Looper {
  // Thread-safe tasks queue
  private final BlockingQueue<Runnable> queue = new LinkedBlockingQueue<>();
  private final AtomicBoolean keepRunning = new AtomicBoolean(true);

  private boolean running() {
    return keepRunning.get();
  }

  /** For stopping the Looper */
  public void stop() {
    keepRunning.set(false);
  }

  /** Looper drains the queue, and executes the tasks in order of reception (FIFO) */
  private final Runnable looper =
      () -> {
        while (running()) {
          try {
            Runnable task = queue.take();
            task.run();
          } catch (InterruptedException e) {
            Thread.currentThread().interrupt(); // set the interrupt flag to "true"
          }
        }
      };

  @NotNull
  public Runnable getLooper() {
    return looper;
  }

  public void submit(@NotNull Runnable task) throws InterruptedException {
    queue.put(task);
  }
}
