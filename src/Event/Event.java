//
// <author> damirlj@yahoo.com
// Copyright (c) 2024. All rights reserved!
//
package <your package>;

import androidx.annotation.NonNull;
import java.util.Date;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/** Helper class for event synchronization */
public final class Event {
  private final Lock mLock; // mutex
  private final Condition mCondition; // condition variable
  private boolean mEventSignaled = false; // predicate to prevent spurious wake-ups

  public Event() {
    mLock = new ReentrantLock();
    mCondition = mLock.newCondition();
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
        e.printStackTrace(); // You can add your own logging mechanism
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
          mCondition.signal();
        };

    lockAndThen(callback);
  }

  /** Signal the event to the all threads that are waiting on the same event notification */
  public void signalAll() {
    ILockCallback callback =
        () -> {
          mEventSignaled = true;
          mCondition.signalAll();
        };

    lockAndThen(callback);
  }

  /**
   * Wait on event signalization, or until thread which waits on event notification is interrupted
   *
   * @param autoReset Reset event after being signaled - for the subsequent wait call
   */
  public void waitEvent(boolean autoReset) {

    ILockCallback callback =
        () -> {
          while (!mEventSignaled) {
            mCondition.await();
          }
          if (autoReset) mEventSignaled = false;
        };

    lockAndThen(callback);
  }

  /**
   * Wait on event being signaled, waiting thread interrupted, or timeout is expired
   *
   * @param time The relative time interval to be waited for
   * @param timeUnit Unit of time to wait for (nanoseconds, microseconds, milliseconds, etc.)
   * @param autoReset Reset event after being signaled - for the subsequent wait call
   * @return Indication of the wait outcome: false - timeout expired before event is notified.
   *     Otherwise - true
   */
  public boolean waitEventFor(final long time, final TimeUnit timeUnit, boolean autoReset) {
    final boolean[] signaled = new boolean[1];
    ILockCallback callback =
        () -> {
          while (!mEventSignaled) {
            signaled[0] = mCondition.await(time, timeUnit);
          }
          if (autoReset) mEventSignaled = false; // don't care if it's already false:timeout expired
        };

    lockAndThen(callback);

    return signaled[0];
  }

  /**
   * Wait on the event being signaled, waiting thread interrupted, or the deadline reached
   *
   * @param deadline The absolute time to be waited on
   * @param autoReset Reset event after being signaled - for the subsequent wait call
   * @return Indication of the wait outcome: false - timeout expired before event is notified.
   *     Otherwise - true
   */
  public boolean waitEventUntil(final Date deadline, boolean autoReset) {
    final boolean[] signaled = new boolean[1];
    ILockCallback callback =
        () -> {
          while (!mEventSignaled) {
            signaled[0] = mCondition.awaitUntil(deadline);
          }
          if (autoReset) mEventSignaled = false; // don't care if it's already false
        };

    lockAndThen(callback);

    return signaled[0];
  }
}
