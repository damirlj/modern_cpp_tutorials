package <your package>;

import static junit.framework.Assert.assertTrue;

import java.util.List;
import java.util.Locale;
import java.util.concurrent.TimeUnit;


import org.jetbrains.annotations.NotNull;
import org.junit.Test;

public class TestEvent {

  @FunctionalInterface
  private interface ISignal <TEvent extends Event> {
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

  @Test
  public void testWaitSignaled() throws InterruptedException {
    final AutoResetEvent event = new AutoResetEvent();
    final long timeout = 1000L;

    Thread waiter =
        new Thread(
            () -> {
              long start = System.nanoTime();
              event.waitEvent();
              long elapsed = (System.nanoTime() - start) / 1_000_000;
              System.out.printf(Locale.ENGLISH, "<Event> signaled after: %d[ms]%n", elapsed);
              assertTrue(elapsed >= timeout);
            });

    Thread signaler = signalThread(Event::signal, event, timeout);
    signaler.start();

    waiter.start();
    waiter.join();
  }

  @Test
  public void testWaitSignaled_multipleThreads() throws InterruptedException {
    final AutoResetEvent event = new AutoResetEvent();
    final long timeout = 1000L;

    Runnable waiter =
        () -> {
          long start = System.nanoTime();
          event.waitEvent();
          long elapsed = (System.nanoTime() - start) / 1_000_000;
          System.out.printf(Locale.ENGLISH, "<Event> (%s) signaled after: %d[ms]%n", Thread.currentThread().getName(), elapsed);
          assertTrue(elapsed >= timeout);
        };

    Thread signaler = signalThread(Event::signalAll, event, timeout);
    signaler.start();

    List<Thread> threads =
        List.of(
            new Thread(waiter, "t_waiter#1"),
            new Thread(waiter, "t_waiter#2"),
            new Thread(waiter, "t_waiter#3"));
    for (Thread thread : threads) {
      thread.start();
    }
    for (Thread thread : threads) {
      thread.join();
    }
  }

  @Test
  public void testWaitForSignaled() throws InterruptedException {
    final AutoResetEvent event = new AutoResetEvent();
    final long timeout = 1000L;

    Thread waiter =
        new Thread(
            () -> {
              long start = System.nanoTime();
              boolean signaled = event.waitEventFor(timeout * 2, TimeUnit.MILLISECONDS);
              if (signaled) {
                long elapsed = (System.nanoTime() - start) / 1_000_000;
                System.out.printf(Locale.ENGLISH, "<Event> signaled after: %d[ms]%n", elapsed);
                assertTrue(elapsed >= timeout);
              }
            });

    Thread signaler = signalThread(Event::signal, event, timeout);
    signaler.start();

    waiter.start();
    waiter.join();
  }

  @Test
  public void testWaitFor_missedSignal() throws InterruptedException {
    final AutoResetEvent event = new AutoResetEvent();
    final long timeout = 1000L;

    Thread waiter =
        new Thread(
            () -> {
              long start = System.nanoTime();
              boolean signaled = event.waitEventFor(timeout / 2, TimeUnit.MILLISECONDS);
              if (!signaled) { // timeout expired
                long elapsed = (System.nanoTime() - start) / 1_000_000;
                System.out.printf(Locale.ENGLISH, "<Event> timeout expired: %d[ms]%n", elapsed);
                assertTrue(elapsed <= timeout);
              }
            });
    waiter.start();

    Thread signaler = signalThread(Event::signal, event, timeout);
    signaler.start();
    signaler.join();
  }
}
