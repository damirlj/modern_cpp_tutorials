package <your package>;

import androidx.annotation.NonNull;

import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import java.util.concurrent.Callable;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/** Helper class for event synchronization */
public class Event {
  private final Lock mLock; // mutex
  private final Condition mCondition; // condition variable
  private volatile boolean mEventSignaled = false; // predicate to prevent spurious wake ups

  private static final class ThreadList {
    private final List<Long> threads = new ArrayList<>();

    public void add() {
      threads.add(Thread.currentThread().getId());
    }

    public void remove() {
      threads.remove(Thread.currentThread().getId());
    }

    public boolean empty() {
      return threads.isEmpty();
    }
  }

  private final ThreadList waitingThreads = new ThreadList();
  private final boolean autoReset;

  public Event(boolean autoReset) {
    mLock = new ReentrantLock();
    mCondition = mLock.newCondition();

    this.autoReset = autoReset;
  }

  @FunctionalInterface
  private interface ILockCallback {
    void apply() throws Exception; // signature relevant: methods of condition variable may throw
  }

  private void lockAndThen(@NonNull ILockCallback callback) {
    mLock.lock();
    try {
      callback.apply(); // Call condition variable related methods, without handling exceptions
    } catch (Exception e) {
      e.printStackTrace();
    } finally {
      mLock.unlock();
    }
  }

  /**
   * Signal the event to wake up a single thread, among the all threads which are waiting on the
   * same event
   */
  public void signal() {
    ILockCallback callback =
        () -> {
          mEventSignaled = true;
          if (!waitingThreads.empty()) mCondition.signal();
        };

    lockAndThen(callback);
  }

  /** Signal the event to the all threads that are waiting on the same event notification */
  public void signalAll() {
    ILockCallback callback =
        () -> {
          mEventSignaled = true;
          if (!waitingThreads.empty()) mCondition.signalAll();
        };

    lockAndThen(callback);
  }

  private boolean waitOnCondition(Callable<Boolean> waitStrategy) throws Exception {
    waitingThreads.add();
    try {
      while (!mEventSignaled) {
        if (!waitStrategy.call()) {
          return false; // Timeout or deadline reached
        }
      }
      return true;
    } finally {
      waitingThreads.remove();
      if (autoReset && waitingThreads.empty()) {
        mEventSignaled = false;
      }
    }
  }

  /**
   * Wait on event signalization, or until thread which waits on event notification is interrupted
   */
  public void waitEvent() {
    lockAndThen(
        () ->
            waitOnCondition(
                () -> {
                  mCondition.await();
                  return true;
                }));
  }

  /**
   * Wait on event being signaled, waiting thread interrupted, or timeout is expired
   *
   * @param time The relative time interval to be waited for
   * @param unit Unit of time to wait for (nanoseconds, microseconds, milliseconds, etc.)
   * @return Indication of the wait outcome: false - timeout expired before event is notified.
   *     Otherwise - true
   */
  public boolean waitEventFor(final long time, final TimeUnit unit) {
    final boolean[] signaled = {false};
    lockAndThen(() -> signaled[0] = waitOnCondition(() -> mCondition.await(time, unit)));
    return signaled[0];
  }

  /**
   * Wait on the event being signaled, waiting thread interrupted, or the deadline reached
   *
   * @param deadline The absolute time to be waited on
   * @return Indication of the wait outcome: false - timeout expired before event is notified.
   *     Otherwise - true
   */
  public boolean waitEventUntil(final Date deadline) {
    final boolean[] signaled = {false};
    lockAndThen(() -> signaled[0] = waitOnCondition(() -> mCondition.awaitUntil(deadline)));
    return signaled[0];
  }
}
