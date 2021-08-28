package java.lang;

public final class Object {
    public Object() { }

    public boolean equals(Object obj) {
        return (this == obj);
    }

    public native String getName();

    public String toString() {
        return getName();
    }
}
