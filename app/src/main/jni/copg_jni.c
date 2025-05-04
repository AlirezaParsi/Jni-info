#include <jni.h>
#include <string.h>
#include <android/log.h>
#include <android/bitmap.h>
#include <vector>

#define LOG_TAG "COPG_JNI"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

jbyteArray drawableToByteArray(JNIEnv *env, jobject drawable) {
    jclass bitmapDrawableClass = env->FindClass("android/graphics/drawable/BitmapDrawable");
    if (!env->IsInstanceOf(drawable, bitmapDrawableClass)) {
        return NULL;
    }

    jmethodID getBitmap = env->GetMethodID(bitmapDrawableClass, "getBitmap", "()Landroid/graphics/Bitmap;");
    jobject bitmap = env->CallObjectMethod(drawable, getBitmap);

    jclass bitmapClass = env->FindClass("android/graphics/Bitmap");
    jmethodID compress = env->GetMethodID(bitmapClass, "compress", "(Landroid/graphics/Bitmap$CompressFormat;ILjava/io/OutputStream;)Z");
    jclass byteArrayOutputStreamClass = env->FindClass("java/io/ByteArrayOutputStream");
    jmethodID baosConstructor = env->GetMethodID(byteArrayOutputStreamClass, "<init>", "()V");
    jobject baos = env->NewObject(byteArrayOutputStreamClass, baosConstructor);

    jclass compressFormatClass = env->FindClass("android/graphics/Bitmap$CompressFormat");
    jfieldID pngFormat = env->GetStaticFieldID(compressFormatClass, "PNG", "Landroid/graphics/Bitmap$CompressFormat;");
    jobject png = env->GetStaticObjectField(compressFormatClass, pngFormat);

    env->CallBooleanMethod(bitmap, compress, png, 100, baos);
    jmethodID toByteArray = env->GetMethodID(byteArrayOutputStreamClass, "toByteArray", "()[B");
    jbyteArray iconBytes = (jbyteArray) env->CallObjectMethod(baos, toByteArray);

    env->DeleteLocalRef(bitmap);
    env->DeleteLocalRef(baos);
    env->DeleteLocalRef(bitmapDrawableClass);
    env->DeleteLocalRef(compressFormatClass);
    env->DeleteLocalRef(png);
    return iconBytes;
}

JNIEXPORT jobjectArray JNICALL
Java_com_example_copg_COPGNative_getInstalledApps(JNIEnv *env, jclass clazz, jboolean includeIcons) {
    // Get PackageManager
    jclass contextClass = env->FindClass("android/content/Context");
    jclass activityThreadClass = env->FindClass("android/app/ActivityThread");
    jmethodID getSystemContext = env->GetStaticMethodID(activityThreadClass, "currentApplication", "()Landroid/app/Application;");
    jobject application = env->CallStaticObjectMethod(activityThreadClass, getSystemContext);
    jmethodID getPackageManager = env->GetMethodID(contextClass, "getPackageManager", "()Landroid/content/pm/PackageManager;");
    jobject packageManager = env->CallObjectMethod(application, getPackageManager);

    // Get installed packages
    jclass packageManagerClass = env->FindClass("android/content/pm/PackageManager");
    jmethodID getInstalledPackages = env->GetMethodID(packageManagerClass, "getInstalledPackages", "(I)Ljava/util/List;");
    jint GET_ACTIVITIES = 0;
    jobject packageInfoList = env->CallObjectMethod(packageManager, getInstalledPackages, GET_ACTIVITIES);

    // Convert List to ArrayList
    jclass listClass = env->FindClass("java/util/List");
    jmethodID listSize = env->GetMethodID(listClass, "size", "()I");
    jmethodID listGet = env->GetMethodID(listClass, "get", "(I)Ljava/lang/Object;");
    jint size = env->CallIntMethod(packageInfoList, listSize);

    // Define AppInfo class
    jclass appInfoClass = env->FindClass("com/example/copg/AppInfo");
    jmethodID appInfoConstructor = env->GetMethodID(appInfoClass, "<init>", "(Ljava/lang/String;Ljava/lang/String;[B)V");

    // Create result array
    jobjectArray result = env->NewObjectArray(size, appInfoClass, NULL);

    for (jint i = 0; i < size; i++) {
        jobject packageInfo = env->CallObjectMethod(packageInfoList, listGet, i);
        jclass packageInfoClass = env->FindClass("android/content/pm/PackageInfo");

        // Get package name
        jfieldID packageNameField = env->GetFieldID(packageInfoClass, "packageName", "Ljava/lang/String;");
        jstring packageName = (jstring) env->GetObjectField(packageInfo, packageNameField);

        // Get app name
        jmethodID getApplicationInfo = env->GetMethodID(packageInfoClass, "applicationInfo", "Landroid/content/pm/ApplicationInfo;");
        jobject applicationInfo = env->CallObjectMethod(packageInfo, getApplicationInfo);
        jclass applicationInfoClass = env->FindClass("android/content/pm/ApplicationInfo");
        jmethodID loadLabel = env->GetMethodID(packageManagerClass, "getApplicationLabel", "(Landroid/content/pm/ApplicationInfo;)Ljava/lang/CharSequence;");
        jstring appName = (jstring) env->CallObjectMethod(packageManager, loadLabel, applicationInfo);

        // Get icon
        jbyteArray iconBytes = NULL;
        if (includeIcons) {
            jmethodID loadIcon = env->GetMethodID(packageManagerClass, "getApplicationIcon", "(Landroid/content/pm/ApplicationInfo;)Landroid/graphics/drawable/Drawable;");
            jobject drawable = env->CallObjectMethod(packageManager, loadIcon, applicationInfo);
            if (drawable) {
                iconBytes = drawableToByteArray(env, drawable);
                env->DeleteLocalRef(drawable);
            }
        }

        // Create AppInfo object
        jobject appInfo = env->NewObject(appInfoClass, appInfoConstructor, packageName, appName, iconBytes);
        env->SetObjectArrayElement(result, i, appInfo);

        // Cleanup
        if (packageName) env->DeleteLocalRef(packageName);
        if (appName) env->DeleteLocalRef(appName);
        if (iconBytes) env->DeleteLocalRef(iconBytes);
        if (packageInfo) env->DeleteLocalRef(packageInfo);
        if (applicationInfo) env->DeleteLocalRef(applicationInfo);
    }

    // Cleanup
    env->DeleteLocalRef(packageInfoList);
    env->DeleteLocalRef(packageManager);
    env->DeleteLocalRef(application);
    env->DeleteLocalRef(contextClass);
    env->DeleteLocalRef(activityThreadClass);
    env->DeleteLocalRef(packageManagerClass);
    env->DeleteLocalRef(listClass);
    env->DeleteLocalRef(appInfoClass);

    return result;
}
