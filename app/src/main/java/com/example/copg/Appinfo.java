package com.example.copg;

public class AppInfo {
    public String packageName;
    public String appName;
    public byte[] icon;

    public AppInfo(String packageName, String appName, byte[] icon) {
        this.packageName = packageName;
        this.appName = appName;
        this.icon = icon;
    }
}
