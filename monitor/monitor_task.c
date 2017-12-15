/**
* monitor_task.c -- 监测模块任务
* 
* 作者: yangzhilong
* 创建时间: 2009-10-30
* 最后修改时间: 2009-10-30
*/
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>
#include <netinet/in.h>

#include <sys/types.h>  
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>  
#include <unistd.h> 
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/errno.h>
#include <poll.h>
#include <sys/stat.h>

#include "include/basetype.h"
#include "include/debug/shellcmd.h"
#include "include/debug.h"
#include "include/param/term.h"
#include "include/sys/schedule.h"
#include "include/sys/gpio.h"
#include "include/sys/reset.h"
#include "include/sys/timeal.h"
#include "include/sys/timer.h"
#include "include/sys/task.h"
#include "include/sys/cycsave.h"
#include "include/sys/syslock.h"
#include "include/lib/bcd.h"
#include "downlink/plmdb.h"
#include "include/uplink/svrnote.h"
#include "../uplink/svrcomm.h"
#include "include/monitor/alarm.h"
#include "downlink/plcomm.h"



#define maxn 0x8000     // 理论支持明文长度
#define ENCODE 0,16,1       // 加密用的宏
#define DECODE 15,-1,-1     // 解密用的宏

// 明文初始置换
char msg_ch[64] = {
    58, 50, 42, 34, 26, 18, 10, 2, 60, 52, 44, 36, 28, 20, 12, 4,
    62, 54, 46, 38, 30, 22, 14, 6, 64, 56, 48, 40, 32, 24, 16, 8,
    57, 49, 41, 33, 25, 17,  9, 1, 59, 51, 43, 35, 27, 19, 11, 3,
    61, 53, 45, 37, 29, 21, 13, 5, 63, 55, 47, 39, 31, 23, 15, 7
};

// 密钥初始置换
char key_ch[56] = {
    57, 49, 41, 33, 25, 17,  9,  1, 58, 50, 42, 34, 26, 18,
    10,  2, 59, 51, 43, 35, 27, 19, 11,  3, 60, 52, 44, 36,
    63, 55, 47, 39, 31, 23, 15,  7, 62, 54, 46, 38, 30, 22,
    14,  6, 61, 53, 45, 37, 29, 21, 13,  5, 28, 20, 12,  4
};

// 扩展置换
char msg_ex[48] = {
    32,  1,  2,  3,  4,  5,  4,  5,  6,  7,  8,  9,
    8,  9, 10, 11, 12, 13, 12, 13, 14, 15, 16, 17,
    16, 17, 18, 19, 20, 21, 20, 21, 22, 23, 24, 25,
    24, 25, 26, 27, 28, 29, 28, 29, 30, 31, 32,  1
};

// 每轮密钥的位移
char key_mov[16] = {
    1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1
};

// 压缩置换
char key_cmprs[48] = {
    14, 17, 11, 24,  1,  5,  3, 28, 15,  6, 21, 10,
    23, 19, 12,  4, 26,  8, 16,  7, 27, 20, 13,  2,
    41, 52, 31, 37, 47, 55, 30, 40, 51, 45, 33, 48,
    44, 49, 39, 56, 34, 53, 46, 42, 50, 36, 29, 32
};

// S 盒置换
char s_box[8][4][16] = {
    // S1
    14,  4, 13,  1,  2, 15, 11,  8,  3, 10,  6, 12,  5,  9,  0,  7,
    0, 15,  7,  4, 14,  2, 13,  1, 10,  6, 12, 11,  9,  5,  3,  8,
    4,  1, 14,  8, 13,  6,  2, 11, 15, 12,  9,  7,  3, 10,  5,  0,
    15, 12,  8,  2,  4,  9,  1,  7,  5, 11,  3, 14, 10,  0,  6, 13,
    // S2
    15,  1,  8, 14,  6, 11,  3,  4,  9,  7,  2, 13, 12,  0,  5, 10,
    3, 13,  4,  7, 15,  2,  8, 14, 12,  0,  1, 10,  6,  9, 11,  5,
    0, 14,  7, 11, 10,  4, 13,  1,  5,  8, 12,  6,  9,  3,  2, 15,
    13,  8, 10,  1,  3, 15,  4,  2, 11,  6,  7, 12,  0,  5, 14,  9,
    // S3
    10,  0,  9, 14,  6,  3, 15,  5,  1, 13, 12,  7, 11,  4,  2,  8,
    13,  7,  0,  9,  3,  4,  6, 10,  2,  8,  5, 14, 12, 11, 15,  1,
    13,  6,  4,  9,  8, 15,  3,  0, 11,  1,  2, 12,  5, 10, 14,  7,
    1, 10, 13,  0,  6,  9,  8,  7,  4, 15, 14,  3, 11,  5,  2, 12,
    // S4
    7, 13, 14,  3,  0,  6,  9, 10,  1,  2,  8,  5, 11, 12,  4, 15,
    13,  8, 11,  5,  6, 15,  0,  3,  4,  7,  2, 12,  1, 10, 14,  9,
    10,  6,  9,  0, 12, 11,  7, 13, 15,  1,  3, 14,  5,  2,  8,  4,
    3, 15,  0,  6, 10,  1, 13,  8,  9,  4,  5, 11, 12,  7,  2, 14,
    // S5
    2, 12,  4,  1,  7, 10, 11,  6,  8,  5,  3, 15, 13,  0, 14,  9,
    14, 11,  2, 12,  4,  7, 13,  1,  5,  0, 15, 10,  3,  9,  8,  6,
    4,  2,  1, 11, 10, 13,  7,  8, 15,  9, 12,  5,  6,  3,  0, 14,
    11,  8, 12,  7,  1, 14,  2, 13,  6, 15,  0,  9, 10,  4,  5,  3,
    // S6
    12,  1, 10, 15,  9,  2,  6,  8,  0, 13,  3,  4, 14,  7,  5, 11,
    10, 15,  4,  2,  7, 12,  9,  5,  6,  1, 13, 14,  0, 11,  3,  8,
    9, 14, 15,  5,  2,  8, 12,  3,  7,  0,  4, 10,  1, 13, 11,  6,
    4,  3,  2, 12,  9,  5, 15, 10, 11, 14,  1,  7,  6,  0,  8, 13,
    // S7
    4, 11,  2, 14, 15,  0,  8, 13,  3, 12,  9,  7,  5, 10,  6,  1,
    13,  0, 11,  7,  4,  9,  1, 10, 14,  3,  5, 12,  2, 15,  8,  6,
    1,  4, 11, 13, 12,  3,  7, 14, 10, 15,  6,  8,  0,  5,  9,  2,
    6, 11, 13,  8,  1,  4, 10,  7,  9,  5,  0, 15, 14,  2,  3, 12,
    // S8
    13,  2,  8,  4,  6, 15, 11,  1, 10,  9,  3, 14,  5,  0, 12,  7,
    1, 15, 13,  8, 10,  3,  7,  4, 12,  5,  6, 11,  0, 14,  9,  2,
    7, 11,  4,  1,  9, 12, 14,  2,  0,  6, 10, 13, 15,  3,  5,  8,
    2,  1, 14,  7,  4, 10,  8, 13, 15, 12,  9,  0,  3,  5,  6, 11
};

// P 盒置换
char p_box[32] = {
    16, 7, 20, 21, 29, 12, 28, 17, 1,  15, 23, 26, 5,  18, 31, 10,
    2,  8, 24, 14, 32, 27, 3,  9,  19, 13, 30, 6,  22, 11, 4,  25
};

// 末置换
char last_ch[64] = {
    40, 8, 48, 16, 56, 24, 64, 32, 39, 7, 47, 15, 55, 23, 63, 31,
    38, 6, 46, 14, 54, 22, 62, 30, 37, 5, 45, 13, 53, 21, 61, 29,
    36, 4, 44, 12, 52, 20, 60, 28, 35, 3, 43, 11, 51, 19, 59, 27,
    34, 2, 42, 10, 50, 18, 58, 26, 33, 1, 41,  9, 49, 17, 57, 25
};

// hash 置换，将加密后的密文置换为可读明文
char hs_ch[20] = "0123456789abcdef";
char sh_ch[128];

void init_trans() {
    char i;
    for (i = 0; i < 16; i++)
        sh_ch[hs_ch[i]] = i;    // 完成hash转换的对应
}

char msg[maxn], key[16], res[32];
char msgb[72], msgbt[72], keyb[18][72];

// 字符转成二进制
void ChToBit(char* dest, char* src, int length) {
    int i, j;
    char t;
    for (i = 0; i < length; i++) {
        for (j = 8, t = src[i]; j > 0; j--) {
            dest[(i << 3) + j] = t & 1;   // 取字符末位
            t >>= 1;
        }
    }
}

// 二进制转成字符
void BitToCh(char* dest, char* src, int length) {
    int i;
    for (i = 0; i < length << 3; i++) {
        dest[i >> 3] <<= 1;
        dest[i >> 3] |= src[i + 1];   // 添加到末位
    }
    dest[length] = 0;
}

// 批置换，以offset为偏移，以count为长度
void BatchSet(char* dest, char* src, char* offset, int count) {
    int i;
    for (i = 0; i < count; i++)
        dest[i + 1] = src[offset[i]];
}

// 得到16轮所需的密钥
void getKeys() {
    char tk[128], bk[72];
    char* ptk = tk;
    int i, j;
    for (i = 0; i < 8; i++)
        key[i] <<= 1; // 跳过奇偶校验位
    ChToBit(bk, key, 8);
    BatchSet(tk, bk, key_ch, 56);
    for (i = 0; i < 16; i++) {
        for (j = 0; j < key_mov[i]; j++, ptk++) {
            ptk[57] = ptk[28];
            ptk[28] = ptk[1];
            // ptk++ 为亮点所在，实质上每一位都没有左移，只是指针右移了
        }
        BatchSet(keyb[i], ptk, key_cmprs, 48);
    }
}

// 将加密后的密文转换为可读的明文
void msgPro(char* dest, char* src) {
    int i, j;
    for (i = 0; i < 16; i++) {
        dest[i] = 0;
        for (j = 1; j <= 4; j++) // 取4位按hash构造一个字符
            dest[i] = (dest[i] << 1) | src[i * 4 + j];
        dest[i] = hs_ch[dest[i]];
    }
    dest[i] = 0;
}

// 将密文转换为真正的密文
void dropMsg(char* dest, char* src) {
    int i;
    for (i = 0; i < 16; i++) { // 为hash的逆运算过程
        dest[i >> 1] = (dest[i >> 1] << 4) | sh_ch[src[i]];
    }
}

// 通用加/解密函数，后面三个参数由宏ENCODE和DECODE提供
void DES(char* pmsg, int st, int cl, int step) {
    int i, row, col;
    char r[64], rt[48], s[8];
    ChToBit(msgbt, pmsg, 8);
    BatchSet(msgb, msgbt, msg_ch, 64); // 初始置换
    for (; st != cl; st += step) {
        memcpy(rt, msgb + 33, 32);
        BatchSet(r, msgb + 32, msg_ex, 48); // 扩展置换
        for (i = 1; i <= 48; i++)
            r[i] ^= keyb[st][i]; // 异或操作
        // s_box 代替
        for (i = 0; i < 48; i += 6) {
            row = col = 0;
            row = r[i + 1] << 1 | r[i + 6];
            col = (r[i + 2] << 3) | (r[i + 3] << 2) | (r[i + 4] << 1) | r[i + 5];
            s[i / 12] = (s[i / 12] <<= 4) | s_box[i / 6][row][col];
        }
        ChToBit(r, s, 4);
        BatchSet(msgb + 32, r, p_box, 32); // p_box 置换
        for (i = 1; i <= 32; i++)
            msgb[i + 32] ^= msgb[i]; // 异或
        memcpy(msgb + 1, rt, 32);
    }
    memcpy(msgbt + 33, msgb + 1, 32);
    memcpy(msgbt + 1, msgb + 33, 32);
    BatchSet(msgb, msgbt, last_ch, 64); // 末置换
    if (step == 1)
        msgPro(res, msgb); // 使密文可读
    else
        BitToCh(res, msgb, 8); // 转为原明文
}



#define IPSTR "10.0.0.104"
#define PORT 8989
#define BUFSIZE 1024

//int main(int argc, char **argv)
//char cilent_main();
char cilent_main(char buf[BUFSIZE]);

char cilent_main(char buf[BUFSIZE])
{
        int sockfd, ret, i, h;
        struct sockaddr_in servaddr;
        //char str1[4096], str2[4096], buf[BUFSIZE], *str;
        char str1[4096], str2[4096], *str;
        socklen_t len;
        fd_set   t_set1;
        char *toke;
        char *ps[1024];
        int t = 0;
        int j = 0;
        struct timeval  tv;


        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
                printf("创建网络连接失败,本线程即将终止---socket error!\n");
                exit(0);
        }

        bzero(&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(PORT);
        if (inet_pton(AF_INET, IPSTR, &servaddr.sin_addr) <= 0 ){
                printf("创建网络连接失败,本线程即将终止--inet_pton error!\n");
                exit(0);
        };

        if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
                printf("连接到服务器失败,connect error!\n");
                exit(0);
        }
        printf("与远端建立了连接\n");
        memset(str2, 0, 4096);
        strcat(str2, "theDataToPost");
        str=(char *)malloc(128);
        len = strlen(str2);
        sprintf(str, "%d", len);

        memset(str1, 0, 4096);
        strcat(str1, "GET /mhsy_web/huoqulianjie?sn=1234 HTTP/1.1\n");
        strcat(str1, "Host:10.0.0.118\n");
 //       strcat(str1, "Host:IPSTR\n");
        strcat(str1, "Content-Type: text/html\n");
        strcat(str1, "Content-Length: ");
        strcat(str1, str);
        strcat(str1, "\n\n");
        printf("%s\n",str);

        strcat(str1, str2);
        strcat(str1, "\r\n\r\n");
        printf("%s\n",str1);

        printf("****************\n");
        ret = write(sockfd,str1,strlen(str1));
        if (ret < 0) {
                printf("发送失败！错误代码是%d，错误信息是'%s'\n",errno, strerror(errno));
                exit(0);
        }else{
                printf("消息发送成功，共发送了%d个字节！\n\n", ret);
        }
        printf("****************\n");
         printf("*****************%s\n",str1);
        FD_ZERO(&t_set1);
        FD_SET(sockfd, &t_set1);

        while(1){
                sleep(2);
                tv.tv_sec= 0;
                tv.tv_usec= 0;
                h= 0;
                printf("--------------->1\n");
                h= select(sockfd +1, &t_set1, NULL, NULL, &tv);
                printf("--------------->2\n");


                if (h < 0) {
                        close(sockfd);
                        printf("在读取数据报文时SELECT检测到异常，该异常导致线程终止！\n");
                        return -1;
                }

                if (h > 0){
                        memset(buf, 0, 1024);
                        i= read(sockfd, buf,1024 );
                        if (i<0){
                                close(sockfd);
                                printf("读取数据报文时发现远端关闭，该线程终止！\n");
                                return -1;
                        }
                }

                        printf("********1********\n");
                        printf("%s\n", buf);
                        return buf;
                        /*printf("*********1*******\n");

                        char *c = strtok(buf,"\n");
                        while(c)
                        {
                                ps[t++] = c;
                                c = strtok(NULL,"\n");

                        }
                        for(j =0 ;j<t;j++)
                        {
                            printf("%s\n",ps[j]);
                        }*/
        }
        close(sockfd);


//        return 0;
}


//static int TimerIdPowerOff = -1;
#define USB_DISK   "/dev/sda1"
#define TMP_UDISK    TEMP_PATH"udisk.tmp" //监时文件
#define UDISKPARAM     "fdisk -l /dev/sd?" //检测U盘挂载盘符

#define TMP_SERIAL    TEMP_PATH"serial.tmp" //监时文件

unsigned char termtousb=0;//0:可以用U盘更新程序到终端,1从终端拷贝数据到U盘
unsigned char udiskname[96];//盘符名
static int USBLockId = -1;//资源锁
static int USBUartLockId = -1;//资源锁
int udisk = -1;//文件名//盘符名文件ID

extern unsigned char hard_info[18];
extern unsigned char     audiourl[512];

extern unsigned char       UpdateAudiourlFlag;

unsigned char wifi_down_musice_state=0;
unsigned char wifi_update_system_state=0;
/**
* @brief 校准系统时钟
*/

extern void MakeAlarmG(alarm_buf_t *pbuf);
extern void SaveAlarm(alarm_buf_t *pbuf);
extern void UpdateAlarm(alarm_buf_t *pbuf);
extern alarm_buf_t * GetCurrentAlarm();
static int currentButtonState = -1; //0--为停止状态，1--为播放状态

static void SysClockCilibrate(void)
{
    sysclock_t clock;

    if(ReadExternSysClock(&clock)) return;

    SysClockSet(&clock);
}

#define CYCSAVE_TIMEOUT        (7*3500)    // 7 hour
#define CYCSAVE_FIRSTCNT        (3*3500)    // 3 hour
#define DBCLR_TIMEOUT            (16*3500)  // 16 hour
#define DBCLR_FIRSETCNT        (2*3500)   // 2 hour
#define CLKCHK_TIMEOUT            (5*3500)   // 5 hour

//播放音频文件 
//filename 文件名称
//type 文件类型1--音频治疗文件，0--语音提示文件
static int PlayVoice(const char * filename,int type)
{
    char cmd[128]={0};
    char dir[80] = {0};
    system("killall -9 madplay");
    system("killall -9 aplay");
    int filestat = 0;
    if(type == 1)
    {
        filestat = getFileDays();//判断音频文件是否有效
        PrintLog(0,"getFileDays %d",filestat);
        if(filestat !=0)
        {
            //音频文件失效，播放提示音
            sprintf(cmd,"aplay /tmp/mounts/SD-P1/voice/musicefileInvalid.wav  &");
            system(cmd);
            Sleep(1);
            //切换音频播放开关
            gpio_set_value(GPIO_39,1);
            gpio_set_value(GPIO_42,1);
            return -1;
        }
        else if(access("/tmp/mounts/SD-P1/play/shock.mp3",F_OK)==0)
        {
            //音频有效，则循环播放音频文件
            sprintf(cmd,"madplay /tmp/mounts/SD-P1/play/%s -r &",filename);
            system(cmd);
            Sleep(1);
            //切换音频播放开关
            gpio_set_value(GPIO_39,0);
            gpio_set_value(GPIO_42,0);
            return 0;
        }
        else
        {
            //无音频文件，播放提示音
            sprintf(cmd,"aplay /tmp/mounts/SD-P1/voice/musicefileInvalid.wav  &");
            system(cmd);
            Sleep(1);
            gpio_set_value(GPIO_39,1);
            gpio_set_value(GPIO_42,1);
            return -1;
        }



    }
    else if(type == 0)
    {
        //播放指定文件名的提示音
        sprintf(cmd,"aplay /tmp/mounts/SD-P1/voice/%s  &",filename);

        system(cmd);
        Sleep(1);
        gpio_set_value(GPIO_39,1);
        gpio_set_value(GPIO_42,1);
        return 0;

    }
}

/**
* @brief 监测任务
*/
extern void DbaseClear(void);
//更新音频播放记录和统计播放时长，存入文件中，在设备登录后用于上传工作参数
static void *UpdateAlarmTask_Monitor(void *arg)
{
    static int times = 0;
    while(1){//每100毫秒监测一次

        if(currentButtonState == 1)//如果是播放按键为按下
        {
            times++;
            if(times >= 30)
            {
                PrintLog(0,"UpdateAlarmTask_Monitor");
                UpdateAlarm(GetCurrentAlarm());//更新播放时间
                times = 0;
            }
        }
        else if(currentButtonState == 0 ||currentButtonState == -1)
        {
            times = 0;
        }
        Sleep(100);


        if(exitflag)
            break;
    }
    return 0;
}


static void *NetLedTask_Monitor(void *arg)
{
    static int lednet = 1;
    int  times = 0;
    while(1){
        if(UpdateAudiourlFlag == 1)
        {
            gpio_set_value(GPIO_LED_NET,1);
            Sleep(10);
            gpio_set_value(GPIO_LED_NET,0);
            Sleep(10);
        }
        else if(SvrCommLineState == LINESTAT_ON)
        {
            gpio_set_value(GPIO_LED_NET,0);
            Sleep(100);
        }
        else if(SvrCommLineState == LINESTAT_OFF)
        {
            times++;
            if(times>1000);
            {
                 gpio_set_value(GPIO_LED_NET,lednet);
                times = 0;
                if(lednet == 0)
                    lednet = 1;
                else
                    lednet = 0;
            }

        }
        Sleep(1000);
        if(exitflag)
            break;
    }
    return 0;
}

//下载音乐文件任务监测
static void *SysLedTask_Monitor(void *arg)
{
    while(1){
        gpio_set_value(GPIO_LED_SYS,1);
            Sleep(100);
        gpio_set_value(GPIO_LED_SYS,0);
        Sleep(100);
        if(exitflag)
            break;
    }
    return 0;
}

//下载音乐文件任务监测
static void *DownLoadMusicTask_Monitor(void *arg)
{
    char downloadcmd[512] = {0};
    struct    stat     buf;
    int     reseult;
    int     filestat = 0;
    int     firstDownTaskCheck = 1;
    while(1){//每10毫秒监测一次
        //如果音频更新为1或者第一次更新为1或者登录状态为1
        if(UpdateAudiourlFlag ||(1 == firstDownTaskCheck)||(1==ParaTermG.login_update_system)) //进入下载状态
        {
            reseult = stat("/tmp/mounts/SD-P1/music.zip",&buf);//检测压缩文件的状态

            PrintLog(0,"music.zip size:%d,create time:%s",buf.st_size,ctime(&buf.st_ctime));
            PrintLog(0," in downloadMusic Task stop play...\n");


             if(1 == firstDownTaskCheck)//如果是第一次下载，则说明还没有下载，此值有文件配置才对，否则一直会进入此条件
             {
                 wifi_down_musice_state = 1;
                PrintLog(0," in firstDownTaskCheck Task stop play...\n");
                system("wifi up");
                 memset(downloadcmd,0,512);
                sprintf(downloadcmd,"sh /opt/work/musicdownload.sh");
                system(downloadcmd);
                firstDownTaskCheck = 0;
                wifi_down_musice_state = 0;
             }
             if(1==ParaTermG.login_update_system)
             {
                 wifi_down_musice_state = 1;
                PrintLog(0," in firstDownTaskCheck Task stop play...\n");
                system("wifi up");
                 memset(downloadcmd,0,512);
                sprintf(downloadcmd,"sh /opt/work/musicdownload.sh");
                system(downloadcmd);
                ParaTermG.login_update_system = 0;
                wifi_down_musice_state = 0;
             }
             else
             {
                wifi_down_musice_state = 1;
                system("wifi up");
                PlayVoice("startdownload.wav",0);
                currentButtonState = 0;

                memset(downloadcmd,0,512);
                sprintf(downloadcmd,"sh /opt/work/musicdownload.sh %s",audiourl);
                system(downloadcmd);
                PlayVoice("enddownload.wav",0);
                system("wifi up");
                PrintLog(0,"downloadMusic Task Finshed...\n");

                wifi_down_musice_state = 0;

                UpdateAudiourlFlag = 0;
            }



        }
        Sleep(10);
        if(exitflag)
            break;
    }
    return 0;
}
//系统远程升级监测任务
static void *UpdateSystemTask_Monitor(void *arg)
{
    char systemcheckcmd[512] = {0};
    int  checktime = 600;
        while(1){
             if(checktime>600)//每10分钟必须更新一次
             {
                wifi_update_system_state = 1;
                 PrintLog(0," in UpdateSystemTask_Monitor Task stop play...\n");
                memset(systemcheckcmd,0,512);
                sprintf(systemcheckcmd,"sh /opt/work/ftup.sh");
                system(systemcheckcmd);
                wifi_update_system_state = 0;
                checktime=0;
             }
             if(1==ParaTermG.login_update_system)//扫描到系统更新标记时也要更新
             {
                wifi_update_system_state = 1;
                 PrintLog(0," in UpdateSystemTask_Monitor Task stop play...\n");
                memset(systemcheckcmd,0,512);
                sprintf(systemcheckcmd,"sh /opt/work/ftup.sh");
                system(systemcheckcmd);
                ParaTermG.login_update_system = 0;
                wifi_update_system_state = 0;//阻止wifi关闭
                checktime=0;//重新计时
             }
            Sleep(1000);
            checktime++;
            if(exitflag)//此值永远为0，是做什么用的呢？
                break;
        }
        return 0;

}


//监测播放按键按下事件
static void *PlayTask_Pressdown(void *arg)
{

    static int presstimes = 0;
    unsigned int value = 0;
    int  playstate = 0;
    gpio_export(GPIO_PLAY);
    gpio_set_dir(GPIO_PLAY, 0);
    gpio_set_edge(GPIO_PLAY, "rising");

    currentButtonState  = 0;
    PrintLog(0,"ParaTermG.first_start %d...\n",ParaTermG.first_start);
    if(ParaTermG.first_start)
    {
        PlayVoice("welcome.wav",0);

    }
    else
    {
        PlayVoice("welcome1.wav",0);
    }
    system("sh /opt/work/unzipmusic.sh");

    while (1) {

            gpio_get_value(GPIO_PLAY,&value);
            if(value == 0 )
            {
                presstimes++;


            }
            else if(value == 1)
            {
                if(presstimes>5)
                {

                    if(currentButtonState == 0)
                    {
                        if(UpdateAudiourlFlag == 0)
                        {

                            PlayVoice("startplay.wav",0);

                            Sleep(600);
                            playstate = PlayVoice("shock.mp3",1);
                            if(playstate == 0)
                            {
                                MakeAlarmG(GetCurrentAlarm());
                                currentButtonState = 1;
                                PrintLog(0,"play button press to start play...\n");
                                system("wifi up");
                                SvrCommLineState = LINESTAT_OFF;
                            }

                        }
                        else if(UpdateAudiourlFlag == 1)
                        {
                            PlayVoice("downloading.wav",0);
                        }

                    }
                    else if(currentButtonState == 1)
                    {
                        if(UpdateAudiourlFlag == 1)
                        {
                            PlayVoice("downloading.wav",0);
                        }
                        else if(UpdateAudiourlFlag == 0)
                        {
                            PrintLog(0,"play button press to stop play...\n");
                            system("killall -9 madplay");
                            PrintLog(0,"play button press to stop play1...\n");

                            SaveAlarm(GetCurrentAlarm());
                            gpio_set_value(GPIO_42,1);
                            gpio_set_value(GPIO_39,0);
                            //system("wifi up");
                            PrintLog(0,"play button press to stop play2...\n");

                            PlayVoice("stopplay.wav",0);
                            PrintLog(0,"play button press to stop play3...\n");

                            currentButtonState = 0;
                        }
                    }
                    presstimes = 0;
                }
            }
        Sleep(1);
    if(exitflag)
    {

        break;
    }
    }
    return 0;
}
static void wifitodnamy(){//仅供公司测试时使用，一旦wifi设置失败了，可以长按+号5秒进行公司的wifi链接
    sleep(600);
    char systemcheckcmd[512] = {0};
    memset(systemcheckcmd,0,512);
    sprintf(systemcheckcmd,"sh /www/cgi-bin/setwifi dnamy 2017dnamy");//测试控制器
    system(systemcheckcmd);
}

//监测音量按键按下事件
static void *VolumeBtn_Pressdown(void *arg)
{


    static int pressaddtimes = 0;
    static int presssubtimes = 0;
    static int currentVolume = 9;
    unsigned char VolumeLevel[10]={0};
    VolumeLevel[0] = 0;
    VolumeLevel[1] = 40;
    VolumeLevel[2] = 70;
    VolumeLevel[3] = 80;
    VolumeLevel[4] = 90;
    VolumeLevel[5] = 100;
    VolumeLevel[6] = 110;
    VolumeLevel[7] = 120;
    VolumeLevel[8] = 124;
    VolumeLevel[9] = 127;
    char   cmd[100];
    int gpio_fdadd;
    int gpio_fdsub;

    unsigned int value = 0;

    gpio_export(GPIO_KEY_ADD);
    gpio_set_dir(GPIO_KEY_ADD, 0);
    gpio_set_edge(GPIO_KEY_ADD, "rising");
    gpio_fdadd = gpio_fd_open(GPIO_KEY_ADD);

    gpio_export(GPIO_KEY_SUB);
    gpio_set_dir(GPIO_KEY_SUB, 0);
    gpio_set_edge(GPIO_KEY_SUB, "rising");
    gpio_fdsub = gpio_fd_open(GPIO_KEY_SUB);

    PrintLog(0,"play current volume is %d...\n",VolumeLevel[currentVolume]);

    while (1) {

            gpio_get_value(GPIO_KEY_ADD,&value);
            if(value == 0 )
            {
                pressaddtimes++;
                if(pressaddtimes >400 && presssubtimes<100)
                {
                    //if(currentButtonState == 0)
                    {
                        PlayVoice("enablewifi.wav",0);
                        sprintf(cmd,"wifi up");
                        system(cmd);
                        wifitodnamy();//仅供测试使用
                        pressaddtimes = 0;
                    }
                }

            }
            else if(value == 1)
            {
                if(presssubtimes>800 && pressaddtimes >800)
                {
                    PrintLog(0,"1 presssubtimes is %d...pressaddtimes is %d\n",presssubtimes,pressaddtimes);
                    sprintf(cmd,"uci set wireless.ap.encryption=\'none\'");
                    system(cmd);
                    sprintf(cmd,"uci delete wireless.ap.key");
                    system(cmd);
                    sprintf(cmd,"uci commit");
                    system(cmd);
                    sprintf(cmd,"uci -c/opt/ft set ftconfig.@ftconfig[0].firststart=1");
                    system(cmd);
                    sprintf(cmd,"uci -c/opt/ft commit");
                    sprintf(cmd,"wifi up");
                    system(cmd);
                    sprintf(cmd,"wifi up");
                    system(cmd);
                    PlayVoice("resetwifi.wav",0);
                    presssubtimes = 0;
                    pressaddtimes = 0;
                }
                if(pressaddtimes>5)
                {
                    if(currentVolume<9)
                        currentVolume++;

                    PrintLog(0,"play current volume is %d...\n",VolumeLevel[currentVolume]);
                    memset(cmd,0,100);
                    sprintf(cmd,"amixer cset numid=9,iface=MIXER,name=\'Headphone Playback Volume\' %d",VolumeLevel[currentVolume]);
                    system(cmd);
                    pressaddtimes = 0;
                }
            }

            gpio_get_value(GPIO_KEY_SUB,&value);
            if(value == 0 )
            {
                presssubtimes++;
                if(presssubtimes >400 &pressaddtimes <100)
                {
                    //if(currentButtonState == 0)
                    {

                    PlayVoice("disablewifi.wav",0);
                    sprintf(cmd,"wifi up");
                        system(cmd);
                        presssubtimes = 0;
                    }
                }


            }
            else if(value == 1)
            {
                if(presssubtimes>800 && pressaddtimes >800)
                {
                    PrintLog(0,"1 presssubtimes is %d...pressaddtimes is %d\n",presssubtimes,pressaddtimes);
                    sprintf(cmd,"uci set wireless.ap.encryption=\'none\'");
                    system(cmd);
                    sprintf(cmd,"uci delete wireless.ap.key");
                    system(cmd);
                    sprintf(cmd,"uci commit");
                    system(cmd);
                    sprintf(cmd,"uci -c/opt/ft set ftconfig.@ftconfig[0].firststart=1");
                    system(cmd);
                    sprintf(cmd,"uci -c/opt/ft commit");

                    sprintf(cmd,"wifi up");
                    system(cmd);
                    sprintf(cmd,"wifi up");
                    system(cmd);
                    PlayVoice("resetwifi.wav",0);
                    presssubtimes = 0;
                    pressaddtimes = 0;
                }
                if(presssubtimes>5)
                {

                    if(currentVolume>0)
                        currentVolume--;
                    PrintLog(0,"play current volume is %d...\n",VolumeLevel[currentVolume]);
                    memset(cmd,0,100);
                    sprintf(cmd,"amixer cset numid=9,iface=MIXER,name=\'Headphone Playback Volume\' %d",VolumeLevel[currentVolume]);
                    system(cmd);
                    presssubtimes = 0;
                }
            }

        Sleep(1);
    if(exitflag)
    {
      gpio_fd_close(gpio_fdadd);
      gpio_fd_close(gpio_fdsub);
        break;
    }
    }
    gpio_fd_close(gpio_fdadd);
    gpio_fd_close(gpio_fdsub);
    return 0;
}


DECLARE_INIT_FUNC(MonitorTaskInit);
int MonitorTaskInit(void)
{

    RunStateInit();
    SysCreateTask(PlayTask_Pressdown, NULL);//音频播放键按下时任务
    AlarmInit();
    SysCreateTask(UpdateSystemTask_Monitor, NULL);//系统更新任务
    SysCreateTask(UpdateAlarmTask_Monitor, NULL);//更新播放时间
    SysCreateTask(DownLoadMusicTask_Monitor, NULL);//音乐下载，内部有协议通信方法
    SysCreateTask(NetLedTask_Monitor, NULL);
    SysCreateTask(SysLedTask_Monitor, NULL);
    SysCreateTask(VolumeBtn_Pressdown, NULL);
    SET_INIT_FLAG(MonitorTaskInit);
    return 0;
}


