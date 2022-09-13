#include <stdlib.h>
#include <stdio.h>
#include <jni.h>
#include <string.h>

#include <piclib.h>
#include <ptp.h>

struct PtpRuntime ptp_runtime;

JNIEXPORT void JNICALL Java_dev_petabyt_camcontrol_MainActivity_init
        (JNIEnv *env, jobject thiz) {
    ptp_runtime.transaction = 1;
    ptp_runtime.data = malloc(1024);
}

JNIEXPORT jbyteArray JNICALL Java_dev_petabyt_camcontrol_MainActivity_recievePacket
        (JNIEnv *env, jobject thiz, jint code, jintArray params0, jint read_size) {

    jsize length = (*env)->GetArrayLength(env, params0);
    jint *params1 = (*env)->GetIntArrayElements(env, params0, 0);

    uint32_t params[5];

    for (int i = 0; i < (int)length; i++) {
        params[i] = params1[i];
    }

    (*env)->ReleaseIntArrayElements(env, params0, params1, 0);

    int size = ptp_recv_packet(&ptp_runtime, code, params, (int)length, (int)read_size);

    jbyteArray ret = (*env)->NewByteArray(env, size);
    (*env)->SetByteArrayRegion(env, ret, 0, size, (const jbyte *)(ptp_runtime.data));
    return ret;
}

JNIEXPORT jbyteArray JNICALL Java_dev_petabyt_camcontrol_MainActivity_recievePacketPre
        (JNIEnv *env, jobject thiz, jint code) {

    int size = ptp_recv_packet_pre(&ptp_runtime, code);

    jbyteArray ret = (*env)->NewByteArray(env, size);
    (*env)->SetByteArrayRegion(env, ret, 0, size, (const jbyte *)(ptp_runtime.data));
    return ret;
}
