import java.util.LinkedList;

//-------------------------------------------------------------------------------
// Classe que representa um pool de threads com fila de tarefas
class FilaTarefas {
    private final int nThreads;
    private final MyPoolThreads[] threads;
    private final LinkedList<Runnable> queue;
    private boolean shutdown;

    public FilaTarefas(int nThreads) {
        this.shutdown = false;
        this.nThreads = nThreads;
        queue = new LinkedList<Runnable>();
        threads = new MyPoolThreads[nThreads];
        for (int i = 0; i < nThreads; i++) {
            threads[i] = new MyPoolThreads();
            threads[i].start();
        }
    }

    public void execute(Runnable r) {
        synchronized(queue) {
            if (this.shutdown) return;
            queue.addLast(r);
            queue.notify();
        }
    }

    public void shutdown() {
        synchronized(queue) {
            this.shutdown = true;
            queue.notifyAll();
        }
        for (int i = 0; i < nThreads; i++) {
            try {
                threads[i].join();
            } catch (InterruptedException e) {
                return;
            }
        }
    }

    private class MyPoolThreads extends Thread {
        public void run() {
            Runnable r;
            while (true) {
                synchronized(queue) {
                    while (queue.isEmpty() && (!shutdown)) {
                        try {
                            queue.wait();
                        } catch (InterruptedException ignored) {}
                    }
                    if (queue.isEmpty()) return;
                    r = queue.removeFirst();
                }
                try {
                    r.run();
                } catch (RuntimeException e) {}
            }
        }
    }
}

//-------------------------------------------------------------------------------

class Hello implements Runnable {
    String msg;
    public Hello(String m) {
        msg = m;
    }

    public void run() {
        System.out.println(msg);
    }
}

class Primo implements Runnable {
    int numero;

    public Primo(int n) {
        this.numero = n;
    }

    public void run() {
        if (ehPrimo(numero)) {
            System.out.println(numero + " é primo");
        } else {
            System.out.println(numero + " não é primo");
        }
    }

    private boolean ehPrimo(int n) {
        if (n <= 1) return false;
        if (n == 2) return true;
        if (n % 2 == 0) return false;
        for (int i = 3; i * i <= n; i += 2) {
            if (n % i == 0) return false;
        }
        return true;
    }
}

public class MyPool {
    private static final int NTHREADS = 5;

    public static void main (String[] args) {
        FilaTarefas pool = new FilaTarefas(NTHREADS);

        for (int i = 0; i < 10; i++) {
            Runnable hello = new Hello("Hello da tarefa " + i);
            pool.execute(hello);

            Runnable primo = new Primo(i + 1);
            pool.execute(primo);
        }

        pool.shutdown();
        System.out.println("Terminou");
    }
}