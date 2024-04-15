#include <jni.h>
#include <string>
#include <android/log.h>
#include "ogl.h"


#define TAG "NATIVE-LIB.CPP"
FILE *gpFile = NULL;
extern "C"
{


JNIEXPORT jstring

JNICALL
Java_io_csrohit_ndkgl_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "String exported from C++";
    return env->NewStringUTF(hello.c_str());
}


JNIEXPORT int JNICALL
Java_io_csrohit_ndkgl_OpenGLRenderer_initialize(JNIEnv *env, jobject thiz) {

    return initialize();
}

JNIEXPORT void JNICALL
Java_io_csrohit_ndkgl_OpenGLRenderer_resize(JNIEnv *env, jobject thiz, jint width, jint height) {
    resize(width, height);
}


JNIEXPORT int JNICALL
Java_io_csrohit_ndkgl_OpenGLView_openLogFile(JNIEnv *env, jobject thiz,
                                             jstring log_file_directory) {
    jboolean isCopy = false;
    int res = -1;
    const char *tempString = env->GetStringUTFChars(log_file_directory, &isCopy);
    if (NULL != tempString) {
        gpFile = fopen(tempString, "w");
        if (NULL == gpFile) {
            __android_log_print(ANDROID_LOG_INFO, TAG,
                                "Failed to open log file %s errno %d\n",
                                tempString, errno);
            return -1;
        }
        __android_log_print(ANDROID_LOG_INFO, TAG,
                            "Log file opened successfully %s\n",
                            tempString);
        if (JNI_TRUE == isCopy) {
            env->ReleaseStringUTFChars(log_file_directory, tempString);
        }
    }
    return res;
}
JNIEXPORT void JNICALL
Java_io_csrohit_ndkgl_OpenGLView_setFilesDirectory(JNIEnv *env, jobject thiz,
                                                   jstring files_directory) {
    jboolean isCopy = false;
    const char *tempString = env->GetStringUTFChars(files_directory, &isCopy);
    if (NULL != tempString) {
        setFilesDirectory(tempString);
        if (JNI_TRUE == isCopy) {
            env->ReleaseStringUTFChars(files_directory, tempString);
        }
    }
}

JNIEXPORT void JNICALL
Java_io_csrohit_ndkgl_OpenGLView_uninitialize(JNIEnv *env, jobject thiz) {

    uninitialize();

    if (NULL != gpFile) {
        fprintf(gpFile, "Program terminated succesfully\n");
        fclose(gpFile);
        gpFile = NULL;
    }
}

JNIEXPORT void JNICALL
Java_io_csrohit_ndkgl_OpenGLRenderer_display(JNIEnv *env, jobject thiz) {
    display();
    update();

}
}
