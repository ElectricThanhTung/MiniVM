package java.lang;

public final class String {
    private final byte[] value;

    public String() {
        value = "".value;
    }

    public String(String original) {
        value = original.value;
    }

    public String(byte[] bytes) {
        value = bytes;
    }

    public String(byte bytes[], int offset, int length) {
        value = new byte[length];
        System.arraycopy(bytes, offset, value, 0, length);
    }

    public static String valueOf(boolean b) {
        return b ? "true" : "false";
    }

    public static String valueOf(byte b) {
        return String.valueOf((int)b);
    }

    public static String valueOf(char c) {
        return String.valueOf((int)c);
    }

    public static String valueOf(short s) {
        return String.valueOf((int)s);
    }

    public static String valueOf(int i) {
        byte[] buffer = new byte[12];
        int index = 11;
        boolean sign;
        if(i < 0) {
            sign = true;
            i = -i;
        }
        else
            sign = false;

        do {
            buffer[--index] = (byte)((i % 10) + 48);
            i /= 10;
        } while(i > 0);

        if(sign)
            buffer[--index] = '-';

        return new String(buffer, index, 12 - index);
    }

    public static String valueOf(long lng) {
        byte[] buffer = new byte[22];
        int index = 21;
        boolean sign;
        if(lng < 0) {
            sign = true;
            lng = -lng;
        }
        else
            sign = false;

        do {
            buffer[--index] = (byte)((lng % 10) + 48);
            lng /= 10;
        } while(lng > 0);

        if(sign)
            buffer[--index] = '-';

        return new String(buffer, index, 22 - index);
    }

    public static String valueOf(float f) {
        if(f == (1.0f / 0.0f))
            return "inf";
        else if(f == (-1.0f / 0.0f))
            return "-inf";
        else if(f != f)
            return "nan";

        if((-2147483648 <= f) && (f <= 2147483647)) {
            int length = (f < 0) ? 15 : 16;
            byte[] buffer = new byte[length];
            int integer = (int)f;
            int count = 0;
            do {
                integer /= 10;
                count++;
            } while(integer > 0);

            if(f < 0) {
                count++;
                buffer[0] = '-';
            }

            integer = (int)f;
            int index = count;
            do {
                buffer[--index] = (byte)((integer % 10) + 48);
                integer /= 10;
            } while(integer > 0);

            float decimal = f - (int)f;
            buffer[count++] = '.';
            index = count;
            while(index < (length - 1)) {
                decimal *= 10;
                integer = (int)decimal;
                buffer[index++] = (byte)(integer + 48);
                decimal -= integer;
            }
            return new String(buffer);
        }
        else {
//          int bits = Float.floatToRawIntBits(f);
//          byte exponent = (byte)((bits >> 23) & 0xFF - 127);
//          int significand = bits & 0x7FFFFF;
            return "float";
        }
    }

    public static String valueOf(double d) {
        // TO DO
        return "double";
    }

    public static String valueOf(Object obj) {
        return (obj == null) ? "null" : obj.toString();
    }

    public String toLowerCase() {
        byte[] local_value = value;
        int length = local_value.length;
        byte[] new_value = new byte[length];
        length--;
        for(int i = 0; i < length; i++) {
            if((65 <= local_value[i]) && (local_value[i] <= 90))
                new_value[i] = (byte)(local_value[i] + 32);
            else
                new_value[i] = local_value[i];
        }
        return new String(new_value);
    }

    public String toUpperCase() {
        byte[] local_value = value;
        int length = local_value.length;
        byte[] new_value = new byte[length];
        length--;
        for(int i = 0; i < length; i++) {
            if((97 <= local_value[i]) && (local_value[i] <= 122))
                new_value[i] = (byte)(local_value[i] - 32);
            else
                new_value[i] = local_value[i];
        }
        return new String(new_value);
    }

    public String substring(int beginIndex) {
        if(beginIndex == 0)
            return this;
        return new String(value, beginIndex, value.length - beginIndex);
    }

    public String substring(int beginIndex, int endIndex) {
        if((beginIndex == 0) && (endIndex == (value.length - 1)))
            return this;
        byte[] new_value = new byte[endIndex - beginIndex + 1];
        System.arraycopy(value, beginIndex, new_value, 0, endIndex - beginIndex);
        return new String(new_value);
    }

    public int indexOf(int ch) {
        byte[] local_value = value;
        int length = local_value.length - 1;
        for(int i = 0; i < length; i++) {
            if(ch == local_value[i])
                return i;
        }
        return -1;
    }

    public int indexOf(String str) {
        if(this == str)
            return 0;
        byte[] local_value = value;
        byte[] str_value = str.getBytes();
        int length = local_value.length - 1;
        int str_len = str_value.length - 1;

        if(length < str_len)
            return -1;

        for(int i = 0; i < length; i++) {
            if(local_value[i] == str_value[0]) {
                boolean found = true;
                for(int j = 1; j < str_len; j++) {
                    if(local_value[i + j] != str_value[j]) {
                        found = false;
                        break;
                    }
                }
                if(found)
                    return i;
            }
        }
        return -1;
    }

    public int lastIndexOf(int ch) {
        byte[] local_value = value;
        int i = local_value.length - 2;
        for(; i >= 0; i--) {
            if(ch == local_value[i])
                return i;
        }
        return -1;
    }

    public int lastIndexOf(String str) {
        if(this == str)
            return 0;
        byte[] local_value = value;
        byte[] str_value = str.getBytes();
        int length = local_value.length - 1;
        int str_len = str_value.length - 1;

        if(length < str_len)
            return -1;

        for(int i = length - str_len; i >= 0; i--) {
            if(local_value[i] == str_value[0]) {
                boolean found = true;
                for(int j = 1; j < str_len; j++) {
                    if(local_value[i + j] != str_value[j]) {
                        found = false;
                        break;
                    }
                }
                if(found)
                    return i;
            }
        }
        return -1;
    }

    public String replace(String target, String replacement) {
        byte[] local_value = value;
        byte[] str_value = target.getBytes();
        byte[] replacement_value = replacement.getBytes();
        int length = local_value.length - 1;
        int str_len = str_value.length - 1;
        int replace_len = replacement_value.length - 1;

        if(length < str_len)
            return this;

        int new_len = length;
        int buff_size = ((new_len + 1) / 16) * 16;
        if(((new_len + 1) % 16) > 0)
            buff_size += 16;
        byte[] buffer = new byte[buff_size];
        int count = 0;
        for(int i = 0; i < length;) {
            if(local_value[i] == str_value[0]) {
                boolean found = true;
                for(int j = 1; j < str_len; j++) {
                    if(local_value[i + j] != str_value[j]) {
                        found = false;
                        break;
                    }
                }
                if(found) {
                    int len = replace_len - str_len;
                    new_len += len;
                    if((new_len + 1) > buff_size) {
                        buff_size = ((new_len + 1) / 16) * 16;
                        if(((new_len + 1) % 16) > 0)
                            buff_size += 16;

                        byte[] new_buff = new byte[buff_size];
                        System.arraycopy(buffer, 0, new_buff, 0, count);
                        buffer = new_buff;
                    }
                    System.arraycopy(replacement_value, 0, buffer, count, replace_len);
                    count += replace_len;
                    i += str_len;
                    continue;
                }
            }
            buffer[count++] = local_value[i];
            i++;
        }
        return new String(buffer, 0, count + 1);
    }

    public boolean contains(String str) {
        return (indexOf(str) >= 0);
    }

    public int compareTo(String str) {
        if(this == str)
            return 0;
        byte[] local_value = value;
        int size = local_value.length - 1;
        byte[] sValue = str.value;

        for(int i = 0; i < size; i++) {
            byte value1 = local_value[i];
            byte value2 = sValue[i];
            if(value1 != value2) {
                if(value1 > value2)
                    return 1;
                return -1;
            }
        }
        return 0;
    }

    public int compareToIgnoreCase(String str) {
        if(this == str)
            return 0;
        byte[] local_value = value;
        int size = local_value.length - 1;
        byte[] sValue = str.value;

        for(int i = 0; i < size; i++) {
            byte value1 = local_value[i];
            byte value2 = sValue[i];
            if((65 <= value1) && (value1 <= 90))
                value1 += 32;
            if((65 <= value2) && (value2 <= 90))
                value2 += 32;
            if(value1 != value2) {
                if(value1 > value2)
                    return 1;
                return -1;
            }
        }
        return 0;
    }

    public String trim() {
        byte[] local_value = value;
        int length = local_value.length;
        int start = 0;
        int end = length - 2;
        while(start < length) {
            if(local_value[start] != 32)
                break;
            start++;
        }
        while(end >= 0) {
            if(local_value[end] != 32)
                break;
            end--;
        }
        end++;
        if((start == 0) && (end == (length - 1)))
            return this;
        else if(start > end)
            return "";
        else {
            byte[] new_value = new byte[end - start + 1];
            System.arraycopy(value, start, new_value, 0, end - start);
            return new String(new_value);
        }
    }

    public String toString() {
        return this;
    }

    public byte[] getBytes() {
        return value;
    }

    public int length() {
        return value.length;
    }

    public boolean equals(Object anObject) {
        if(!(anObject instanceof String))
            return false;
        String str = (String)anObject;
        if(value.length != str.getBytes().length)
            return false;
        return (compareTo(str) == 0);
    }

    public boolean equalsIgnoreCase(String str) {
        if(value.length != str.getBytes().length)
            return false;
        return (compareToIgnoreCase(str) == 0);
    }
}
