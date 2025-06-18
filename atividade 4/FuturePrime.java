import java.util.concurrent.*;
import java.util.*;

class VerificaPrimo implements Callable<Boolean> {
    private final int numero;

    public VerificaPrimo(int numero) {
        this.numero = numero;
    }

    public Boolean call() {
        return ehPrimo(numero);
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

public class FuturePrime {
    private static final int NTHREADS = 10;
    private static final int MAX_NUM = 10000;

    public static void main(String[] args) {
        ExecutorService executor = Executors.newFixedThreadPool(NTHREADS);
        List<Future<Boolean>> resultados = new ArrayList<Future<Boolean>>();

        for (int i = 1; i <= MAX_NUM; i++) {
            Callable<Boolean> tarefa = new VerificaPrimo(i);
            Future<Boolean> futuro = executor.submit(tarefa);
            resultados.add(futuro);
        }

        int totalPrimos = 0;
        for (int i = 0; i < resultados.size(); i++) {
            try {
                if (resultados.get(i).get()) {
                    totalPrimos++;
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }

        executor.shutdown();
        System.out.println("Total de primos entre 1 e " + MAX_NUM + ": " + totalPrimos);
    }
}