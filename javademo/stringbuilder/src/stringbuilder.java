
public class stringbuilder {
    public static void main(String[] args) {
        String hello = "Hello! This is the ";
        String msg_end = "th message.";
        for(int i = 0; i < 10000; i++)
            System.out.println(hello + (i + 1) + msg_end);
    }
}
