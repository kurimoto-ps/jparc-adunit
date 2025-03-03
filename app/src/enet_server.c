#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include "adc_stm32_custom.h"
#include "adc_averaging_ctx.h"
#include "adc_operation.h"

#define BIND_PORT 10000

uint8_t cmd_buf[3];
uint16_t data;
uint8_t tmp_buf[3000];

static const uint8_t kCmdReadCh1[3]   = {0x70, 0x00, 0x21};
static const uint8_t kCmdReadCh2[3]   = {0x70, 0x00, 0x22};
static const uint8_t kCmdNumAvg[3]    = {0x70, 0x00, 0x71};
static const uint8_t kCmdRemaining[3] = {0x70, 0x00, 0x72};
static const uint8_t kCmdAverage[3]   = {0x70, 0x00, 0x73};
static const uint8_t kCmdOffset[3]    = {0x70, 0x00, 0x74};
static const uint8_t kCmdReset[3]     = {0x70, 0x00, 0x75};

static const uint8_t kRspReadCh1[3]   = {0x80, 0x00, 0x21};
static const uint8_t kRspReadCh2[3]   = {0x80, 0x00, 0x22};
static const uint8_t kRspNumAvg[3]    = {0x80, 0x00, 0x71};
static const uint8_t kRspRemaining[3] = {0x80, 0x00, 0x72};
static const uint8_t kRspAverage[3]   = {0x80, 0x00, 0x73};
static const uint8_t kRspOffset[3]    = {0x80, 0x00, 0x74};
static const uint8_t kRspReset[3]     = {0x80, 0x00, 0x75};

static const uint8_t kRspBusy[3]    = {0xf1, 0xff, 0x80};

static const float default_offset = 524288.0f;

static float offset = 524288.0f;

void send_data(int client, void *buf, size_t size) {
    size_t total_sent = 0;
	while (total_sent < size) {
		ssize_t sent = send(client, (char *)buf + total_sent, size - total_sent, 0);
		if (sent < 0) {
			printk("Error sending data\n");
			break;
		}
		total_sent += sent;
	}
}

void send_waveform(int client) {
    size_t buf_index = 0;
    size_t buf_size = avctx.buf_size / sizeof(avctx.buf_sum[0]);
    float inv_num_averages = 1.0f / (float)avctx.num_averages;

    for (int i = 0; i < buf_size; i++) {
        float val_start = avctx.buf_sum[i] * inv_num_averages;
        float val_end = (i == buf_size - 1) ? val_start : avctx.buf_sum[i + 1] * inv_num_averages;
        float delta = (val_end - val_start) * 0.1f;

        for (int j = 0; j < 10; j++) {
            int val = -(int)((val_start + delta * j) * 16.0f - offset);  // "-" due to inveted amplifier
            tmp_buf[buf_index] = val & 0x0000ff;
            tmp_buf[buf_index + 1] = (val & 0x00ff00) >> 8;
            tmp_buf[buf_index + 2] = (val & 0xff0000) >> 16;
            buf_index += 3;

            if (buf_index >= sizeof(tmp_buf)) {
                send_data(client, (void*)tmp_buf, buf_index);
                buf_index = 0;
            }
        }
    }
    if (buf_index > 0) {
        send_data(client, (void*)tmp_buf, buf_index);
    }
    tmp_buf[0] = 0xff;
    tmp_buf[1] = 0xff;
    tmp_buf[2] = 0xff;
    send_data(client, (void*)tmp_buf, 3);
}
    

bool check_cmd(uint8_t set[3]) {
    for (int i = 0; i < 3; i++) {
        if (set[i] != cmd_buf[i]) {
            return false;
        }
    }
    return true;
}

float cal_offset(unsigned int num_averages) {
    if(avctx.buf_sum[0]==0.0f) return default_offset;
    unsigned int cnt = 0;
    float sum = 0;
    unsigned int prev;
    unsigned int len = avctx.buf_size/sizeof(avctx.buf_sum[0]);
    
    for(unsigned int i=0;i<len;i++){
        if(avctx.buf_sum[i]==0.0f){ 
            sum -= (float)prev; 
            cnt--;
            break;
        }
        sum += (float)avctx.buf_sum[i];
        prev = avctx.buf_sum[i];
        cnt++;
    }
    return (sum/((float)cnt)/((float)num_averages))*16;

}

void handle_client_commands(int client) {
    static unsigned int num_averages_offset = 1;
    static bool offset_done = false;
    while (1) {
        ssize_t len = recv(client, cmd_buf, 3, 0);
        if (len == 0) {
            close(client);
            break;
        }

        if (check_cmd(kCmdRemaining)) {
            send(client, kRspRemaining, 3, 0);
            send(client, (uint8_t*)&avctx.remaining_count, 2, 0);
            continue;
        }

        if (k_event_test(&adc_events, BIT(0))) {
            send(client, kRspBusy, 3, 0);
            continue;
        }

        if (check_cmd(kCmdReadCh1)) {
            printk("test\n");
            recv(client, tmp_buf, 3, 0);
            printk("required num data = %d\n", (tmp_buf[2]<<16)|(tmp_buf[1]<<8)|tmp_buf[0]);
            send(client, kRspReadCh1, 3, 0);
            send_waveform(client);
            continue;
        }

        if (check_cmd(kCmdReadCh2)) {
            recv(client, tmp_buf, 3, 0);
            printk("required num data = %d\n", (tmp_buf[2]<<16)|(tmp_buf[1]<<8)|tmp_buf[0]);
            send(client, kRspReadCh2, 3, 0);
            send_waveform(client);
            continue;
        }

        if (check_cmd(kCmdNumAvg)) {
            recv(client, (uint8_t*)&data, 2, 0);
            set_num_averages(data);
            printk("num averages = %d\n", avctx.num_averages);
            send(client, kRspNumAvg, 3, 0);
            send(client, (uint8_t*)(&(avctx.num_averages)), sizeof(avctx.num_averages), 0);
            continue;
        }

        if (check_cmd(kCmdAverage)) {
            send(client, kRspAverage, 3, 0);
            if(!offset_done){
                offset = cal_offset(num_averages_offset);
                offset_done = true;
                printk("offset = %d\n",(int)offset);
            }
            memset(avctx.buf_sum, 0, avctx.buf_size);
            avctx.remaining_count = avctx.num_averages;
            k_event_set(&adc_events, BIT(0)); // Trigger ADC start event
            continue;
        }

        if (check_cmd(kCmdOffset)) {
            send(client, kRspOffset, 3, 0);
            memset(avctx.buf_sum, 0, avctx.buf_size);
            avctx.remaining_count = avctx.num_averages;
            num_averages_offset = avctx.num_averages;
            k_event_set(&adc_events, BIT(0)); // Trigger ADC start event
            continue;
        }

        if (check_cmd(kCmdReset)) {
            avctx.remaining_count = avctx.num_averages;
            send(client, kRspReset, 3, 0);
            continue;
        }
    }
}

void enet_server(void) {
    int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in addr = {.sin_family = AF_INET, .sin_port = htons(BIND_PORT), .sin_addr.s_addr = htonl(INADDR_ANY)};
    bind(sockfd, (struct sockaddr *)&addr, sizeof(addr));
    listen(sockfd, 1);
    while (1) {
        int client = accept(sockfd, NULL, NULL);
        handle_client_commands(client);
    }
}
