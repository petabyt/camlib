#include <stdlib.h>
#include <stdio.h>
#include <jni.h>
#include <string.h>

#include <camlib.h>
#include <ptp.h>

struct PtpRuntime ptp_runtime;

#ifndef JNI_FUNC
	#define JNI_FUNC(ret, name) JNIEXPORT ret JNICALL Java_dev_petabyt_camcontrol_MainActivity_##name
#endif

struct AndroidBackend {
    jobject in;
    jobject out;
    jclass pac;
    JNIEnv *env;
}backend;

JNI_FUNC(void, Cinit)(JNIEnv *env, jobject thiz, jobject pac) {
	// TODO: init piclib
	ptp_runtime.transaction = 1;
	ptp_runtime.session = 0;
	ptp_runtime.data = malloc(4096);
	ptp_runtime.data_length = 4096;

    backend.pac = (*env)->GetObjectClass(env, pac);
    backend.env = env;
}

#if 0
JNI_FUNC(jbyteArray, CrecievePacket)(JNIEnv *env, jobject thiz, jint code, jintArray params0, jint read_size) {
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
#endif

JNI_FUNC(jbyteArray, bulkPacketCmd)(JNIEnv *env, jobject thiz, jint code, jintArray params0, jint read_size) {
    jsize length = (*env)->GetArrayLength(env, params0);
    jint *params1 = (*env)->GetIntArrayElements(env, params0, 0);

    struct PtpCommand cmd;
    cmd.code = code;
    cmd.data_length = 0;

    uint32_t params[5];

    for (int i = 0; i < (int)length; i++) {
        cmd.params[i] = params1[i];
    }

    cmd.param_length = 0;

    (*env)->ReleaseIntArrayElements(env, params0, params1, 0);

    int size = ptp_bulk_packet_cmd(&ptp_runtime, &cmd);

    jbyteArray ret = (*env)->NewByteArray(env, size);
    (*env)->SetByteArrayRegion(env, ret, 0, size, (const jbyte *)(ptp_runtime.data));
    return ret;
}

int ptp_send_bulk_packets(struct PtpRuntime *r, int length) {
    jbyteArray data = (*backend.env)->NewByteArray(backend.env, length);
    (*backend.env)->SetByteArrayRegion(backend.env, data, 0, length, (const jbyte *)(ptp_runtime.data));

    jmethodID sendData = (*backend.env)->GetMethodID(backend.env, backend.pac, "sendData", "[BII)I");
    return (*backend.env)->CallIntMethod(backend.env, backend.pac, data, length, 1000);
}
// https://github.com/mikma/androidsc-usb/blob/master/jni/usbjni.cpp