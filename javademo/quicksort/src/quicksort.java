
public class quicksort {
    static void exchange(int[] a, int m, int n) {
        int t = a[m];
        a[m] = a[n];
        a[n] = t;
    }

    static int partition(int[] a, int m, int n) {
        int x = a[m];
        int j = n + 1;
        int i = m - 1;

        while(true) {
            j--;
            while(a[j] > x)
                j--;
            i++;
            while (a[i] < x)
                i++;
            if(i < j)
                exchange(a, i, j);
            else
                return j;
        }
    }

    static void qsort(int[] a, int l, int r) {
        if (l < r) {
            int r2 = partition(a, l, r);
            qsort(a, l, r2);
            qsort(a, r2 + 1, r);
        }
    }

    static void printArray(int[] a) {
        for(int i = 0; i < a.length - 1; i++)
            System.out.print(a[i] + ", ");
        System.out.println(a[a.length - 1]);
        System.out.println("");
    }

    public static void main(String[] args) {
        int test[] = {32, 123, 12, -12, 65, -2, 66, -64, -2, 2, 122, 19, 1234};
        System.out.println("Before sorting:");
        printArray(test);
        qsort(test, 0, test.length - 1);
        System.out.println("After sorting:");
        printArray(test);
    }
}
