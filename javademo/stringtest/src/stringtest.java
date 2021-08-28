
public class stringtest {
    public static void main(String[] args) {
        String str = "   Hello! This is input string for the string class testing process     ";
        System.out.println("Input:");
        System.out.println(str);
        System.out.println("");

        System.out.println("trim space:");
        System.out.println(str.trim());
        System.out.println("");

        System.out.println("Convert to lower case:");
        System.out.println(str.toLowerCase());
        System.out.println("");

        System.out.println("Convert to upper case:");
        System.out.println(str.toUpperCase());
        System.out.println("");

        System.out.println("Index of 'f' (102):");
        System.out.println(str.indexOf(102));
        System.out.println("");

        System.out.println("Index of \"string\":");
        System.out.println(str.indexOf("string"));
        System.out.println("");

        System.out.println("Last index of 't' (116):");
        System.out.println(str.lastIndexOf(116));
        System.out.println("");

        System.out.println("Last index of \"string\":");
        System.out.println(str.lastIndexOf("string"));
        System.out.println("");

        System.out.println("Substring 52->End:");
        System.out.println(str.substring(52));
        System.out.println("");

        System.out.println("Substring 10->30:");
        System.out.println(str.substring(10, 30));
        System.out.println("");

        System.out.println("trim space:");
        System.out.println(str.trim());
        System.out.println("");

        System.out.println("Replace \"string\" to \"str\":");
        System.out.println(str.replace("string", "str"));
        System.out.println("");

        System.out.println("Replace \"input string\" to \"input STRING VALUE\":");
        System.out.println(str.replace("input string", "input STRING VALUE"));
        System.out.println("");
    }
}
