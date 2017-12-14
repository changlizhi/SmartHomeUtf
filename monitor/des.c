#include<stdio.h>
#include<string.h>
#include<stdlib.h>
int SubKey[16][48];
int r1[48];
const static int IP_Table[64] = {
58, 50, 42, 34, 26, 18, 10, 2, 60, 52, 44, 36, 28, 20, 12, 4,
62, 54, 46, 38, 30, 22, 14, 6, 64, 56, 48, 40, 32, 24, 16, 8,
57, 49, 41, 33, 25, 17,  9, 1, 59, 51, 43, 35, 27, 19, 11, 3,
61, 53, 45, 37, 29, 21, 13, 5, 63, 55, 47, 39, 31, 23, 15, 7
};
const static int IPR_Table[64] = {
40, 8, 48, 16, 56, 24, 64, 32, 39, 7, 47, 15, 55, 23, 63, 31,
38, 6, 46, 14, 54, 22, 62, 30, 37, 5, 45, 13, 53, 21, 61, 29,
36, 4, 44, 12, 52, 20, 60, 28, 35, 3, 43, 11, 51, 19, 59, 27,
34, 2, 42, 10, 50, 18, 58, 26, 33, 1, 41,  9, 49, 17, 57, 25
};
static const int E_Table[48] = {
32,  1,  2,  3,  4,  5,  4,  5,  6,  7,  8,  9,
8,  9, 10, 11, 12, 13, 12, 13, 14, 15, 16, 17,
16, 17, 18, 19, 20, 21, 20, 21, 22, 23, 24, 25,
24, 25, 26, 27, 28, 29, 28, 29, 30, 31, 32,  1
};
const static int P_Table[32] = {
16, 7, 20, 21, 29, 12, 28, 17, 1,  15, 23, 26, 5,  18, 31, 10,
2,  8, 24, 14, 32, 27, 3,  9,  19, 13, 30, 6,  22, 11, 4,  25
};
const static int PC1_Table[56] = {
57, 49, 41, 33, 25, 17,  9,  1, 58, 50, 42, 34, 26, 18,
10,  2, 59, 51, 43, 35, 27, 19, 11,  3, 60, 52, 44, 36,
63, 55, 47, 39, 31, 23, 15,  7, 62, 54, 46, 38, 30, 22,
14,  6, 61, 53, 45, 37, 29, 21, 13,  5, 28, 20, 12,  4
};
const static int PC2_Table[48] = {
14, 17, 11, 24,  1,  5,  3, 28,
15,  6, 21, 10,23, 19, 12,  4,
26,  8, 16,  7, 27, 20, 13,  2,
41, 52, 31, 37, 47, 55, 30, 40,
51, 45, 33, 48,44, 49, 39, 56,
34, 53, 46, 42, 50, 36, 29, 32
};
const static int LOOP_Table[16] = {
1,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1
};


const static int S_Box[8][4][16] = {
14,  4, 13,  1,  2, 15, 11,  8,  3, 10,  6, 12,  5,  9,  0,  7,
0, 15,  7,  4, 14,  2, 13,  1, 10,  6, 12, 11,  9,  5,  3,  8,
4,  1, 14,  8, 13,  6,  2, 11, 15, 12,  9,  7,  3, 10,  5,  0,
15, 12,  8,  2,  4,  9,  1,  7,  5, 11,  3, 14, 10,  0,  6, 13,
//S2
15,  1,  8, 14,  6, 11,  3,  4,  9,  7,  2, 13, 12,  0,  5, 10,
3, 13,  4,  7, 15,  2,  8, 14, 12,  0,  1, 10,  6,  9, 11,  5,
0, 14,  7, 11, 10,  4, 13,  1,  5,  8, 12,  6,  9,  3,  2, 15,
13,  8, 10,  1,  3, 15,  4,  2, 11,  6,  7, 12,  0,  5, 14,  9,
//S3
10,  0,  9, 14,  6,  3, 15,  5,  1, 13, 12,  7, 11,  4,  2,  8,
13,  7,  0,  9,  3,  4,  6, 10,  2,  8,  5, 14, 12, 11, 15,  1,
13,  6,  4,  9,  8, 15,  3,  0, 11,  1,  2, 12,  5, 10, 14,  7,
1, 10, 13,  0,  6,  9,  8,  7,  4, 15, 14,  3, 11,  5,  2, 12,
//S4
7, 13, 14,  3,  0,  6,  9, 10,  1,  2,  8,  5, 11, 12,  4, 15,
13,  8, 11,  5,  6, 15,  0,  3,  4,  7,  2, 12,  1, 10, 14,  9,
10,  6,  9,  0, 12, 11,  7, 13, 15,  1,  3, 14,  5,  2,  8,  4,
3, 15,  0,  6, 10,  1, 13,  8,  9,  4,  5, 11, 12,  7,  2, 14,
//S5
2, 12,  4,  1,  7, 10, 11,  6,  8,  5,  3, 15, 13,  0, 14,  9,
14, 11,  2, 12,  4,  7, 13,  1,  5,  0, 15, 10,  3,  9,  8,  6,
4,  2,  1, 11, 10, 13,  7,  8, 15,  9, 12,  5,  6,  3,  0, 14,
11,  8, 12,  7,  1, 14,  2, 13,  6, 15,  0,  9, 10,  4,  5,  3,
//S6
12,  1, 10, 15,  9,  2,  6,  8,  0, 13,  3,  4, 14,  7,  5, 11,
10, 15,  4,  2,  7, 12,  9,  5,  6,  1, 13, 14,  0, 11,  3,  8,
9, 14, 15,  5,  2,  8, 12,  3,  7,  0,  4, 10,  1, 13, 11,  6,
4,  3,  2, 12,  9,  5, 15, 10, 11, 14,  1,  7,  6,  0,  8, 13,
//S8
4, 11,  2, 14, 15,  0,  8, 13,  3, 12,  9,  7,  5, 10,  6,  1,
13,  0, 11,  7,  4,  9,  1, 10, 14,  3,  5, 12,  2, 15,  8,  6,
1,  4, 11, 13, 12,  3,  7, 14, 10, 15,  6,  8,  0,  5,  9,  2,
6, 11, 13,  8,  1,  4, 10,  7,  9,  5,  0, 15, 14,  2,  3, 12,
//s8
13,  2,  8,  4,  6, 15, 11,  1, 10,  9,  3, 14,  5,  0, 12,  7,
1, 15, 13,  8, 10,  3,  7,  4, 12,  5,  6, 11,  0, 14,  9,  2,
7, 11,  4,  1,  9, 12, 14,  2,  0,  6, 10, 13, 15,  3,  5,  8,
2,  1, 14,  7,  4, 10,  8, 13, 15, 12,  9,  0,  3,  5,  6, 11
};
void cpy(int *output, int *tmp, int length)
{
    int j0 = 0;
    for (j0 = 0; j0<length; j0++)
    {
    output[j0] = tmp[j0];
    }
}
void Permute(int *output, int *input, int length, const int* table)
{
    int i1;
    int *tmp =(int *)malloc(sizeof(length));
    for (i1= 0; i1<length; ++i1)
    tmp[i1] = input[table[i1] - 1];
    cpy(output, tmp, length);
}
void Xor(int* output, int *input_a, const int *input_b, int length) 
{
    int i3,j;
    for (i3 = 0; i3<length; i3++)
    output[i3] = input_a[i3] ^ input_b[i3];
}


void F(int right[32], int subkey[48])
{
    int r[48];
    Permute(r, right, 48, E_Table);


    Xor(r1, r, subkey, 48);

    int i2,j,k,h;
    int p;
    for (i2 = 0, j, k, h = 0; i2<8; i2++, h += 6)
    {
    j = (r1[h] * 2) + r1[h + 5];
    k = (r1[h + 1] * 8) + (r1[h + 2] * 4) + (r1[h + 3] * 2) + r1[h + 4];
    int x = S_Box[i2][j][k];
    for (p = 3; p>-1; p--)
    {
        right[i2 * 4 + p] = x % 2;
        x = x / 2;
    }
    }
    Permute(right, right, 32, P_Table);
}
void producekey(int *permute_key)
{
    int i,j,m;
    for (i = 0; i<16; i++)
    {
    int t = LOOP_Table[i];
    int g[56];
    for (j = 0; j<56; j++)
    {
    g[j] = permute_key[j];
    }
    for (j = 0; j<28; j++)
    {
    permute_key[j] = g[(t + j) % 28];
    }
    for (j = 28; j<56; j++)
    {
    permute_key[j] = g[(t + j) % 28 + 28];
    }
    for (m = 0; m<48; m++)
    {
    SubKey[i][m] = permute_key[PC2_Table[m] - 1];
    }
    }
}
void ByteToBit(int *output, char *input, int bits)
{
    int i;
    for (i = 0; i<bits; i++)
    {
    output[i] = (input[i / 8] >> (7 - (i % 8))) & 1;
    }
}


void BitToByte(char * output, int * input, int bits)
{
    int i;
    for (i = 0; i<8; i++)
    {
    output[i] = input[i * 8] * bits * 2 + input[1 + i * 8] * bits + input[2 + i * 8] * (bits / 2) + input[3 + i * 8] * (bits / 4) + input[4 + i * 8] * (bits / 8) +
    input[5 + i * 8] * (bits / 16) + input[6 + i * 8] * (bits / 32) + input[7 + i * 8] * (bits / 64);
    }
}


int Encrypt(int M[64], int C[64])
{
    int left[32], right[32], tmp[32];
    int i,i0;

    Permute(M, M, 64, IP_Table);
    for (i = 0; i<32; i++)
    {
    left[i] = M[i];
    }
    for (i = 0; i<32; i++)
    {
    right[i] = M[i + 32];
    }


    for (i0 = 0; i0<16; ++i0)
    {
    cpy(tmp, right, 32);
    F(right, SubKey[i0]);
    Xor(right, right, left, 32);
    cpy(left, tmp, 32);
    }    
    cpy(tmp, right, 32);
    cpy(right, left, 32);    // 32位互换
    cpy(left, tmp, 32);
    for (i = 0; i<32; i++)
    {
        M[i] = left[i];
        M[i + 32] = right[i];
    }


    Permute(C, M, 64, IPR_Table);       //逆初始置换


    return 0;
}
int Decrypt(int C[64], int M[64]) 
{
    int left[32], right[32], tmp[32];
    int i,i0;

    Permute(C, C, 64, IP_Table);

    for (i = 0; i<32; i++)
    {
        left[i] = C[i];
    }
    for (i = 0; i<32; i++)
    {
    right[i] = C[i + 32];
    }
    for (i0 = 15; i0>-1; i0--)
    {
        cpy(tmp, right, 32);
        F(right, SubKey[i0]);
        Xor(right, right, left, 32);
        cpy(left, tmp, 32);
    }
        cpy(tmp, right, 32);
        cpy(right, left, 32);    // 32位互换
        cpy(left, tmp, 32);
        for (i = 0; i<32; i++)
        {
        C[i] = left[i];
        C[i + 32] = right[i];
        }
        Permute(M, C, 64, IPR_Table);       //逆初始置换


        return 0;
}
int DES_main(char miwen[8]) 
{
    char miyao[9] = "american";
    int key0[64], key[56];
    ByteToBit(key0, miyao, 64);
    Permute(key, key0, 56, PC1_Table);
    producekey(key);
    //char miwen[8];
    int i;
    unsigned char mingwen[90] = "abcdefgh";
    int m[64], c[64];
    ByteToBit(m, mingwen, 64);
    printf("明文二进制输出\n");
    /*for (i = 0; i<64; i++)
    {
        printf("%d",m[i]);
    }*/
        Encrypt(m, c);
        /*printf("密文二进制输出\n");
    for (i = 0; i<64; i++)
    {
        printf("%d",c[i]);
}*/
        printf("*****hello miwen*********\n");
        BitToByte(miwen, c, 64);
        printf("密文输出\n");
        printf("*****%s\n",miwen);
        Decrypt(c, m);
        BitToByte(mingwen, m, 64);
        printf("明文输出\n");
        printf("%s\n",mingwen);
        //return 0;

}
int main()
{
    printf("*****hello main*********\n");
    char miwen[8];
    DES_main(miwen);
    printf("%s\n",miwen);
}
