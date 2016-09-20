package com.github.yongzhizhan.spring.model;

import com.sun.jna.Native;
import com.sun.jna.Pointer;
import com.sun.jna.win32.StdCallLibrary;

public interface Capture extends StdCallLibrary {
    static Capture INSTANCE = (Capture) Native.loadLibrary("capture", Capture.class);

    boolean getCaptureImage(String vSrcPath, Pointer vDestPath, int vSize);
}
