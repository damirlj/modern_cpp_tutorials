package <your package>;

import static junit.framework.Assert.assertTrue;

import java.util.List;
import java.util.Locale;
import java.util.concurrent.TimeUnit;

import org.jetbrains.annotations.NotNull;
import org.junit.Test;

public class TestEvent {

  @FunctionalInterface
  private interface ISignal<TEvent extends Event> {
    void invoke(TEvent e);
  }

  @NotNull
  private Thread signalThread(
      ISignal<AutoResetEvent> callback, final @NotNull AutoResetEvent event, final long timeout) {
    return new Thread(
        () -> {
          try {
            Thread.sleep(timeout);
            callback.invoke(event);
          } catch (InterruptedException e) {
            e.printStackTrace();
          }
        });
  }

  private Runnable waitTaskSignaled(@NotNull Event event, final long signaledAfterMs) {
    return () -> {
      long start = System.nanoTime();
      event.waitEvent();
      long elapsed = (System.nanoTime() - start) / 1_000_000;
      System.out.printf(
          Locale.ENGLISH,
          "<Event> (%s) received after: %d[ms]\n",
          Thread.currentThread().getName(),
          elapsed);
      assertTrue(elapsed >= signaledAfterMs);
    };
  }

  @Test
  public void testWaitSignaled() throws InterruptedException {
    final AutoResetEvent event = new AutoResetEvent();
    final long timeout = 1000L;

    Thread waiter = new Thread(waitTaskSignaled(event, timeout));

    Thread signaler = signalThread(Event::signal, event, timeout);
    signaler.start();

    waiter.start();
    waiter.join();
  }

  @Test
  public void testWaitSignaled_multipleThreads() throws InterruptedException {
    final AutoResetEvent event = new AutoResetEvent();
    final long timeout = 1000L;

    Thread signaler = signalThread(Event::signalAll, event, timeout);
    signaler.start();

    List<Thread> threads =
        List.of(
            new Thread(waitTaskSignaled(event, timeout), "t_waiter#1"),
            new Thread(waitTaskSignaled(event, timeout), "t_waiter#2"),
            new Thread(waitTaskSignaled(event, timeout), "t_waiter#3"));
    for (Thread thread : threads) {
      thread.start();
    }
    for (Thread thread : threads) {
      thread.join();
    }
  }

  private Thread waitForTask(@NotNull Event event, final long timeout) {
    return new Thread(
        () -> {
          long start = System.nanoTime();
          boolean signaled = event.waitEventFor(timeout, TimeUnit.MILLISECONDS);
          long elapsed = (System.nanoTime() - start) / 1_000_000;
          if (signaled) {
            System.out.printf(Locale.ENGLISH, "<Event> received after: %d[ms]\n", elapsed);
            assertTrue(elapsed < timeout);
          } else {
            System.out.printf(Locale.ENGLISH, "<Event> timeout expired: %d[ms]\n", elapsed);
            assertTrue(elapsed >= timeout);
          }
        });
  }

  @Test
  public void testWaitForSignaled() throws InterruptedException {
    final AutoResetEvent event = new AutoResetEvent();
    final long timeout = 1000L;

    Thread waiter = waitForTask(event, timeout * 2);

    Thread signaler = signalThread(Event::signal, event, timeout);
    signaler.start();

    waiter.start();
    waiter.join();
  }

  @Test
  public void testWaitForSignaled_signal_first() throws InterruptedException {
    final AutoResetEvent event = new AutoResetEvent();
    final long timeout = 1000L;

    event.signal();
    Thread.sleep(100);

    Thread waiter = waitForTask(event, timeout);

    waiter.start();
    waiter.join();
  }

  @Test
  public void testWaitForSignaled_not_signaled() throws InterruptedException {
    final AutoResetEvent event = new AutoResetEvent();
    final long timeout = 1000L;

    Thread waiter = waitForTask(event, timeout);

    waiter.start();
    waiter.join();
  }

  @Test
  public void testWaitFor_missedSignal() throws InterruptedException {
    final AutoResetEvent event = new AutoResetEvent();
    final long timeout = 1000L;

    Thread waiter = waitForTask(event, timeout / 2);

    waiter.start();

    Thread signaler = signalThread(Event::signal, event, timeout);
    signaler.start();
    signaler.join();
  }
}
