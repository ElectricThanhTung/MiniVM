package java.lang;

public final class StringBuilder {
    private byte[] value;
    private int count = 0;

    public StringBuilder() {
        value = new byte[16];
    }

    public StringBuilder(String str) {
        if(str == null)
            str = "null";
        byte[] strBytes = str.getBytes();
        int strSize = strBytes.length;
        int capacity = (strSize / 16) * 16;
        if((strSize % 16) != 0)
            capacity += 16;

        byte[] buffer = new byte[capacity];
        System.arraycopy(strBytes, 0, buffer, 0, strSize);
        value = buffer;
        count += strSize - 1;
    }

    private void checkCapacity(int size) {
        int local_count = count;
        int size_old = value.length;
        if((size_old - local_count) <= size) {
            int new_size = size + local_count;
            int capacity = (new_size / 16) * 16;
            if((new_size % 16) != 0)
                capacity += 16;

            byte[] new_value = new byte[capacity];
            System.arraycopy(value, 0, new_value, 0, local_count);
            value = new_value;
        }
    }

    public StringBuilder append(String str) {
        if(str == null)
            str = "null";
        byte[] strBytes = str.getBytes();
        int strSize = strBytes.length;
        int local_count = count;
        checkCapacity(strSize);

        System.arraycopy(strBytes, 0, value, local_count, strSize);

        count += strSize - 1;
        return this;
    }

    public StringBuilder append(StringBuilder str) {
        byte[] strBytes;
        if(str == null)
            strBytes = "null".getBytes();
        else
            strBytes = str.getBytes();
        int strSize = strBytes.length;
        int local_count = count;
        checkCapacity(strSize);

        System.arraycopy(strBytes, 0, value, local_count, strSize);

        count += strSize - 1;
        return this;
    }

    public StringBuilder append(Object obj) {
        return append(String.valueOf(obj));
    }

    public StringBuilder append(char[] str) {
        // TO DO
        return this;
    }

    public StringBuilder append(boolean b) {
        if(b == true)
            append("true");
        else
            append("false");
        return this;
    }

    public StringBuilder append(char c) {
        append(String.valueOf((int)c));
        return this;
    }

    public StringBuilder append(int i) {
        append(String.valueOf(i));
        return this;
    }

    public StringBuilder append(long lng) {
        append(String.valueOf(lng));
        return this;
    }

    public StringBuilder append(float f) {
        append(String.valueOf(f));
        return this;
    }

    public StringBuilder append(double d) {
        append(String.valueOf(d));
        return this;
    }

    public int length() {
        return count;
    }

    public int capacity() {
        return value.length;
    }

    public String toString() {
        return new String(value, 0, count + 1);
    }

    public byte[] getBytes() {
        return value;
    }
}
