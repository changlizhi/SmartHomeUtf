#include <stdio.h>
#include <string.h>

/* 本程序为其他程序调用，所有参数自己定义
 * 详细参数：
 * des d/e key msg
 *  d——解密   e——加密
 *  key——密钥
 *  msg——要加/解密的明/密文
 */
int main(int arg, char* arv[]) {
    if (arg < 3) {
        printf("Input Error");
        return 0;
    }
    init_trans();

    // 读取参数过程
    char mode = arv[1][0];
    strcpy(key, arv[2]);
    strcpy(msg, arv[3]);

    getKeys(); // 得到16轮要用到的密钥

    int i;
    if (mode == 'e') {
        for (i = 0; msg[i]; i += 8) {
            DES(msg + i, ENCODE); // 加密
            printf("%s", res);
        }
    } else if (mode == 'd') {
        for (i = 0; msg[i]; i += 16) {
            dropMsg(res, msg + i); // 将密文转换为真正的密文
            DES(res, DECODE); // 解密
            printf("%s", res);
        }
    } else {
        printf("Input Error!!!");
    }
    printf("\n");
    return 0;
}
