/*
 * 本例程适用于支持POSIX线程的Linux设备, 通过配置C Link SDK（以下简称SDK）中MQTT连接相关参数，建立连接，然后创建如下2个线程：
 *
 * + 一个线程用于保活长连接。
 * + 一个线程用于接收消息, 并在有消息到达时进入默认的数据回调，在连接状态变化时进入事件回调。
 *
 * 您需关注或修改的部分, 已使用TODO在注释中标明。
 *
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "aiot_state_api.h"
#include "aiot_sysdep_api.h"
#include "aiot_mqtt_api.h"

/* 位于portfiles/aiot_port文件夹下的系统适配函数集合。 */
extern aiot_sysdep_portfile_t g_aiot_sysdep_portfile;

/* 位于external/ali_ca_cert.c中的服务器证书。 */
extern const char *ali_ca_cert;

static pthread_t g_mqtt_process_thread;
static pthread_t g_mqtt_recv_thread;
static uint8_t g_mqtt_process_thread_running = 0;
static uint8_t g_mqtt_recv_thread_running = 0;

/* TODO: 如果需关闭日志, 则该函数的实现为空, 如果需减少日志, 可根据code选择不打印。
 *
 * 例如: [1577589489.033][LK-0317] LightSwitch&a18wP******
 *
 * 上述日志的code就是0317(十六进制), code值的定义见core/aiot_state_api.h。
 *
 */

/* 日志回调函数, SDK的日志从此处输出。 */
int32_t demo_state_logcb(int32_t code, char *message)
{
    printf("%s", message);
    return 0;
}

/* MQTT事件回调函数, 当网络连接、重连或断开时，触发该函数, 事件定义见core/aiot_mqtt_api.h。 */
void demo_mqtt_event_handler(void *handle, const aiot_mqtt_event_t *event, void *userdata)
{
    switch (event->type) {
        /* 调用了aiot_mqtt_connect()接口, 与mqtt服务器建立连接。 */
        case AIOT_MQTTEVT_CONNECT: {
            printf("AIOT_MQTTEVT_CONNECT\n");
            /* TODO: 处理SDK建立连接成功, 不可在此调用耗时较长的阻塞函数。 */
        }
        break;

        /* SDK因网络状况被动断开连接后, 成功自动发起重连。 */
        case AIOT_MQTTEVT_RECONNECT: {
            printf("AIOT_MQTTEVT_RECONNECT\n");
            /* TODO: 处理SDK重连成功, 不可在此调用耗时较长的阻塞函数。 */
        }
        break;

        /* SDK因网络状况被动断开了连接, network底层读写失败, heartbeat没有按预期得到服务端心跳应答。 */
        case AIOT_MQTTEVT_DISCONNECT: {
            char *cause = (event->data.disconnect == AIOT_MQTTDISCONNEVT_NETWORK_DISCONNECT) ? ("network disconnect") :
                          ("heartbeat disconnect");
            printf("AIOT_MQTTEVT_DISCONNECT: %s\n", cause);
            /* TODO: 处理SDK被动断开连接, 不可在此调用耗时较长的阻塞函数。 */
        }
        break;

        default: {

        }
    }
}

/* MQTT默认消息处理回调。当SDK从服务器收到MQTT消息时, 且您未设置对应回调的处理时，以下接口被调用。 */
void demo_mqtt_default_recv_handler(void *handle, const aiot_mqtt_recv_t *packet, void *userdata)
{
    switch (packet->type) {
        case AIOT_MQTTRECV_HEARTBEAT_RESPONSE: {
            printf("heartbeat response\n");
            /* TODO: 处理服务器对心跳的回应, 通常不处理。 */
        }
        break;

        case AIOT_MQTTRECV_SUB_ACK: {
            printf("suback, res: -0x%04X, packet id: %d, max qos: %d\n",
                   -packet->data.sub_ack.res, packet->data.sub_ack.packet_id, packet->data.sub_ack.max_qos);
            /* TODO: 处理服务器对订阅请求的回应, 通常不处理。 */
        }
        break;

        case AIOT_MQTTRECV_PUB: {
            printf("pub, qos: %d, topic: %.*s\n", packet->data.pub.qos, packet->data.pub.topic_len, packet->data.pub.topic);
            printf("pub, payload: %.*s\n", packet->data.pub.payload_len, packet->data.pub.payload);
            /* TODO: 处理服务器下发的业务报文。 */
        }
        break;

        case AIOT_MQTTRECV_PUB_ACK: {
            printf("puback, packet id: %d\n", packet->data.pub_ack.packet_id);
            /* TODO: 处理服务器对QoS=1上报消息的回应, 通常不处理。 */
        }
        break;

        default: {

        }
    }
}

/* 执行aiot_mqtt_process的线程, 包含心跳发送和QoS=1消息重发。 */
void *demo_mqtt_process_thread(void *args)
{
    int32_t res = STATE_SUCCESS;

    while (g_mqtt_process_thread_running) {
        res = aiot_mqtt_process(args);
        if (res == STATE_USER_INPUT_EXEC_DISABLED) {
            break;
        }
        sleep(1);
    }
    return NULL;
}

/* 执行aiot_mqtt_recv的线程, 包含网络自动重连和从服务器收取MQTT消息。 */
void *demo_mqtt_recv_thread(void *args)
{
    int32_t res = STATE_SUCCESS;

    while (g_mqtt_recv_thread_running) {
        res = aiot_mqtt_recv(args);
        if (res < STATE_SUCCESS) {
            if (res == STATE_USER_INPUT_EXEC_DISABLED) {
                break;
            }
            sleep(1);
        }
    }
    return NULL;
}

/**/
int main(int argc, char *argv[])
{
    int32_t     res = STATE_SUCCESS;
    void       *mqtt_handle = NULL;
    int8_t      public_instance = 1;  /* 是否公共实例。如果接入公共实例, 该参数设置为1；如果接入企业版实例，则设置为0。 */
    char       *url = "iot-as-mqtt.cn-shanghai.aliyuncs.com"; /* 阿里云物联网平台上海地域的接入域名的后缀。 TODO: 如果是非上海地域，需更改为您的地域。如果是企业版实例, 则更改为对应的接入域名。*/
    char        host[100] = {0}; /* 使用该数组拼接设备连接的物联网平台的完整地址。 其规则为 ${productKey}.iot-as-mqtt.cn-shanghai.aliyuncs.com。 */
    uint16_t    port = 443;      /* 端口号。无论设备是否使用TLS连接, 目的端口均为443。 */
    aiot_sysdep_network_cred_t cred; /* 安全凭据结构体, 如果要用TLS, 该结构体中配置CA证书等参数。 */

    /* TODO: 替换为您设备的认证信息 */
    char *product_key       = "h0njatWd7OS";
    char *device_name       = "Transfor_test";
    char *device_secret     = "4e87383055d5ecce649a1e517e24a643";

    /* 配置SDK的底层依赖。 */
    aiot_sysdep_set_portfile(&g_aiot_sysdep_portfile);
    /* 配置SDK的日志输出。 */
    aiot_state_set_logcb(demo_state_logcb);

    /* 创建SDK的安全凭据, 用于建立TLS连接。 */
    memset(&cred, 0, sizeof(aiot_sysdep_network_cred_t));
    cred.option = AIOT_SYSDEP_NETWORK_CRED_SVRCERT_CA;  /* 使用RSA证书校验MQTT服务端。 */
    cred.max_tls_fragment = 16384; /* 最大的分片长度为16K, 其它可选值：4K、2K、1K、0.5K。 */
    cred.sni_enabled = 1;                               /* TLS建连时, 支持Server Name Indicator。 */
    cred.x509_server_cert = ali_ca_cert;                 /* 用于验证MQTT服务端的RSA根证书。 */
    cred.x509_server_cert_len = strlen(ali_ca_cert);     /* 用于验证MQTT服务端的RSA根证书长度。 */

    /* 创建1个MQTT客户端实例，并内部初始化默认参数。 */
    mqtt_handle = aiot_mqtt_init();
    if (mqtt_handle == NULL) {
        printf("aiot_mqtt_init failed\n");
        return -1;
    }

    /* TODO: 如果以下代码不被注释, 则例程会用TCP（而不是TLS）连接物联网平台。 */
    /*
    {
        memset(&cred, 0, sizeof(aiot_sysdep_network_cred_t));
        cred.option = AIOT_SYSDEP_NETWORK_CRED_NONE;
    }
    */
    if (1 == public_instance) {
        snprintf(host, 100, "%s.%s", product_key, url);
    } else {
        snprintf(host, 100, "%s", url);
    }

    /* 配置MQTT服务器地址。 */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_HOST, (void *)host);
    /* 配置MQTT服务器端口。 */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_PORT, (void *)&port);
    /* 配置设备ProductKey。 */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_PRODUCT_KEY, (void *)product_key);
    /* 配置设备DeviceName。 */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_DEVICE_NAME, (void *)device_name);
    /* 配置设备DeviceSecret。 */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_DEVICE_SECRET, (void *)device_secret);
    /* 配置网络连接的安全凭据。 */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_NETWORK_CRED, (void *)&cred);
    /* 配置MQTT默认消息接收回调函数。 */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_RECV_HANDLER, (void *)demo_mqtt_default_recv_handler);
    /* 配置MQTT事件回调函数。 */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_EVENT_HANDLER, (void *)demo_mqtt_event_handler);

    /* 与服务器建立MQTT连接。 */
    res = aiot_mqtt_connect(mqtt_handle);
    if (res < STATE_SUCCESS) {
        /* 尝试建立连接失败, 销毁MQTT实例, 回收资源。 */
        aiot_mqtt_deinit(&mqtt_handle);
        printf("aiot_mqtt_connect failed: -0x%04X\n", -res);
        return -1;
    }

    /* 创建一个单独的线程用于执行aiot_mqtt_process, 它会自动发送心跳保活, 以及重发QoS=1的未应答报文。 */
    g_mqtt_process_thread_running = 1;
    res = pthread_create(&g_mqtt_process_thread, NULL, demo_mqtt_process_thread, mqtt_handle);
    if (res < 0) {
        printf("pthread_create demo_mqtt_process_thread failed: %d\n", res);
        return -1;
    }

    /* 创建一个单独的线程用于执行aiot_mqtt_recv, 它会循环收取服务器下发的MQTT消息, 并在断线时自动重连。 */
    g_mqtt_recv_thread_running = 1;
    res = pthread_create(&g_mqtt_recv_thread, NULL, demo_mqtt_recv_thread, mqtt_handle);
    if (res < 0) {
        printf("pthread_create demo_mqtt_recv_thread failed: %d\n", res);
        return -1;
    }

    // MQTT订阅Topic功能示例, 请根据业务需求进行使用。
    {
        char *sub_topic = "/h0njatWd7OS/Transfor_test/user/get";

        res = aiot_mqtt_sub(mqtt_handle, sub_topic, NULL, 1, NULL);
        if (res < 0) {
            printf("aiot_mqtt_sub failed, res: -0x%04X\n", -res);
            return -1;
        }
    }


    /* MQTT发布消息功能示例, 请根据自己的业务需求进行使用。 */
    while (1)
    {
        int status;
        scanf("%d", &status);
        fflush(stdin);
        char *pub_topic = "/sys/h0njatWd7OS/Transfor_test/thing/event/property/post";
        //char *pub_payload = "{\"method\":\"thing.event.property.post\",\"params\":{\"StatusLightSwitch\":1},\"version\":\"1.0\"}";
        char glx_pub_payload[256];
        sprintf(glx_pub_payload, "{\"method\":\"thing.event.property.post\",\"params\":{\"TargetTemperature\":35.12,\"StatusLightSwitch\":%d},\"version\":\"1.0\"}",status);
        // printf("pub:%s\r\n", glx_pub_payload);
        res = aiot_mqtt_pub(mqtt_handle, pub_topic, (uint8_t *)glx_pub_payload, (uint32_t)strlen(glx_pub_payload), 0);
        if (res < 0) {
            printf("aiot_mqtt_sub failed, res: -0x%04X\n", -res);
            return -1;
        }
        sleep(1);
    }

    // /* 主循环进入休眠。 */
    // while (1) {
    //     sleep(1);
    // }

    /* 断开MQTT连接, 通常不会运行到此处。 */
    g_mqtt_process_thread_running = 0;
    g_mqtt_recv_thread_running = 0;
    sleep(1);
    res = aiot_mqtt_disconnect(mqtt_handle);
    if (res < STATE_SUCCESS) {
        aiot_mqtt_deinit(&mqtt_handle);
        printf("aiot_mqtt_disconnect failed: -0x%04X\n", -res);
        return -1;
    }

    /* 销毁MQTT实例, 通常不会运行到此处。 */
    res = aiot_mqtt_deinit(&mqtt_handle);
    if (res < STATE_SUCCESS) {
        printf("aiot_mqtt_deinit failed: -0x%04X\n", -res);
        return -1;
    }

    pthread_join(g_mqtt_process_thread, NULL);
    pthread_join(g_mqtt_recv_thread, NULL);

    return 0;
}

