package com.example.copg;

public class COPGNative {
    static {
        System.loadLibrary("copg");
    }

    public native AppInfo[] getInstalledApps(boolean includeIcons);
}
