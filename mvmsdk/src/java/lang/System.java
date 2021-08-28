package java.lang;

import java.io.PrintStream;

public final class System {
    public static final PrintStream out = new PrintStream();
    public static native final void arraycopy(Object src,  int  srcPos, Object dest, int destPos, int length);
}
