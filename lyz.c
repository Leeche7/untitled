#include <stdlib.h>
#include <stdio.h>
#include "extmem.h"

//shared function
void swapblk(unsigned char *blk1, unsigned char *blk2) {
    char temp;
    for (int k = 0; k <= 7; k++) {
        temp = *(blk1 + k);
        *(blk1 + k) = *(blk2 + k);
        *(blk2 + k) = temp;
    }
}
int cmpblk(unsigned char *blk1, unsigned char *blk2) {
    char ch1[10], ch2[10],ch3[10],ch4[10];
    for (int j = 0; j < 4; j++) {
        ch1[j] = *(blk1 + j);
        ch2[j] = *(blk2 + j);
    }
    for (int j = 4; j < 8; j++) {
        ch3[j-4] = *(blk1 + j);
        ch4[j-4] = *(blk2 + j);
    }
    int a1 = atoi(ch1), a2 = atoi(ch2),a3 = atoi(ch3),a4 = atoi(ch4);
    //return a1 < a2 ? 1 : 0; //swapblk(blk1,blk2);
    if(a1 < a2)return 1;
    else if(a1>a2)return 0;
    else return a3<a4;
}
int isequal(unsigned char *blk1, unsigned char *blk2) {
    for (int j = 0; j < 8; j++)if(blk1[j] != blk2[j])return 0;
    return 1;
}
void getfirst(unsigned char *blki, int j, int *aim) {
    char str[9];
    for (int k = 0; k < 4; k++) {
        str[k] = *(blki + j * 8 + k);
    }
    *aim = atoi(str);
}
int trans(char *str, int aim) {
    char ch[10];
    int i = -1;
    while (aim) {
        ch[++i] = aim % 10 + 48;
        aim /= 10;
    }
    int j = 0;
    while (i >= 0) {
        str[j++] = ch[i--];
    }
    return j;
}
void getaddr(unsigned char *blki,int nowaddr){
    char str[10];
    for(int i = 0;i<10;i++)str[i] = 0;
    trans(str,nowaddr+1);
    for(int i = 56;i<64;i++){
        blki[i] = str[i-56];
    }
}

//aim1
int sol1() {
    Buffer buf; /* A buffer */
    if (!initBuffer(520, 64, &buf)) {
        perror("Buffer Initialization Failed!\n");
        return -1;
    }
    unsigned char *blki, *blko; /* A pointer to a block */
    int i = 0;
    int aim = 0;
    printf("-----------------------------------------------------------\n");
    printf("Selection algorithm based on linear searching: S.C = ");
    scanf("%d", &aim);
    printf("-----------------------------------------------------------\n");

    blki = getNewBlockInBuffer(&buf);
    blko = getNewBlockInBuffer(&buf);

    char str[5];
    int cnt = 0, amount = 49;
    int cntt = 0;
    for (i = 17; i <= 48; i++) {
        if ((blki = readBlockFromDisk(blki, i, &buf)) == NULL) {
            printf("read failed!\n");
            exit(-1);
        }
        printf("read data blocks%d\n", i);
        for (int j = 0; j < 7; j++) {
            for (int k = 0; k < 4; k++) {
                str[k] = *(blki + j * 8 + k);
            }
            if (atoi(str) == aim) {
                //printf("find one : j = %d | i = %d", j, i);
                cntt++;
                printf("(%d ,", aim);
                for (int k = 0; k < 4; k++)*(blko + cnt++) = str[k];
                for (int k = 4; k < 8; k++)str[k - 4] = *(blki + j * 8 + k);
                printf("%d)\n", atoi(str));
                for (int k = 0; k < 4; k++)*(blko + cnt++) = str[k];
                if (cnt == 56) {
                    amount++;
                    getaddr(blko,amount);
                    if (writeBlockToDisk(blko, amount, &buf) != 0) {
                        printf("write error!\n");
                        exit(-1);
                    }
                    printf("Notice: the result was written in blocks%d\n",amount);
                    cnt = 0;
                }
                continue;
            }
            // for(int k = 0;k<4;k++)str[k] = *(blki + i*8 + k);
            //printf("no\n");
        }
    }
    if (cnt) {
        getaddr(blko,++amount);
        if (writeBlockToDisk(blko, amount, &buf) != 0) {
            printf("write error!\n");
            exit(-1);
        }
        printf("Notice: the result was written in blocks%d\n",amount);
    }
    printf("\nFound %d tuples meet the request\n\n",cntt);

    freeBlockInBuffer(blki, &buf);
    freeBlockInBuffer(blko, &buf);
    printf("Total number of IO is %lu\n", buf.numIO);
    return 0;
}

//aim2
int sortblk(int nowp, unsigned char *blk[10], int end, int now, Buffer buf) {
    for (int k = 0; k < 6 && k < end; k++) {
        for (int j = 0; j <= 48; j += 8) {
            int min = j;
            for (int i = j + 8; i <= 48; i += 8) {
                if (cmpblk(blk[k] + i, blk[k] + min)) {
                    min = i;
                }
            }
            swapblk(blk[k] + min, blk[k] + j);
        }
    }
    int pt[7];
    for (int i = 0; i < 6; i++)pt[i] = 0;
    int cnt = 0, min = 0, pos = 0;
    while (cnt < end) {
        int i;
        for (i = 0; i < 6 && i < end; i++) {
            if (pt[i] < 56) {
                min = i++;
                break;
            }
        }
        for (; i < 6 && i < end; i++) {
            if (pt[i] < 56 && (cmpblk(blk[i] + pt[i], blk[min] + pt[min]) == 1)) {
                min = i;
            }
        }
        for (int k = 0; k <= 7; k++) {
            *(blk[6] + pos++) = *(blk[min] + pt[min] + k);
        }
        pt[min] += 8;
        if (pos == 56) {
            getaddr(blk[6],cnt+now);
           // for(i = 0;i<63;i++)printf("%c_",blk[6][i]);
           // printf("\n");
            writeBlockToDisk(blk[6], cnt++ + now, &buf);
            pos = 0;
        }
    }
}

int sortone(int begin, int len, unsigned char *blk[10], Buffer buf, int now) {
    //divied
    int nowp = begin - 1;
    int cnt = 0, end, lent = len;
    while (lent) {
        cnt++;
        end = 0;
        for (int i = 0; i < 6 && lent; i++) {
            readBlockFromDisk(blk[i], ++nowp, &buf);
            lent--;
            end++;
        }
        sortblk(nowp - end + 1, blk, end, now, buf);
        now += end;
    }

    int pt[7], pb[7], tail[7];
    for (int i = 0; i < 6; i++) {
        pt[i] = 0;
    }
    int nowt = now;
    tail[cnt - 1] = now - 1;
    now -= end;
    pb[cnt - 1] = now;
    readBlockFromDisk(blk[cnt - 1], pb[cnt - 1], &buf);
    for (int i = cnt - 2; i >= 0; i--) {
        tail[i] = now - 1;
        now -= 6;
        pb[i] = now;
        readBlockFromDisk(blk[i], pb[i], &buf);
    }

    int min = 0, pos = 0;
    while (lent < len) {
        int i;
        //init i
        for (i = 0; i < 6 && i < cnt; i++) {
            if (pt[i] < 56) {
                min = i++;
                break;
            }
        }
        //choose min record
        for (; i < 6 && i < cnt; i++) {
            if (pt[i] < 56 && cmpblk(blk[i] + pt[i], blk[min] + pt[min])) {
                min = i;
            }
        }
        //write record
        for (int k = 0; k <= 7; k++) {
            *(blk[6] + pos++) = *(blk[min] + pt[min] + k);
        }
        pt[min] += 8;
        if (pt[min] == 56) {//read next block
            if (pb[min] != tail[min]) {
                if (readBlockFromDisk(blk[min], ++pb[min], &buf) == NULL) {
                    exit(1);
                }
                pt[min] = 0;
            }
        }
        if (pos == 56) {
            lent++;
            getaddr(blk[6],nowt+lent);

            for (int j = 0; j <= 7; j++) {
                printf("(");
                for (int k = 0; k < 4; k++) {
                    printf("%c", *(blk[6] + j * 8 + k));
                }
                printf(" ,");
                for (int k = 4; k < 8; k++) {
                    printf("%c", *(blk[6] + j * 8 + k));
                }
                printf(") ");
            }
            printf("\n");

            writeBlockToDisk(blk[6], nowt + lent, &buf);
            pos = 0;
        }
    }
    //printf("io = %lu\n", buf.numIO);
}

int sol2() {
    Buffer buf; /* A buffer */
    if (!initBuffer(520, 64, &buf)) {
        perror("Buffer Initialization Failed!\n");
        return -1;
    }
    unsigned char *blk[10];
    for (int i = 0; i < 7; i++) {
        blk[i] = getNewBlockInBuffer(&buf);
    }
    sortone(1, 16, blk, buf, 52);
    printf("The result of sorting R is saved in block69 to block84\n");
    sortone(17, 32, blk, buf, 86);
    printf("The result of sorting S is saved in block119 to block150\n");
}

//aim3
int CreateIndex(Buffer buf) {
    int leafindex = 153;
    int writeidx = 0;
    int idxval;
    unsigned char *blki, *blko;
    char str[5];

    blki = getNewBlockInBuffer(&buf);
    blko = getNewBlockInBuffer(&buf);
    if ((blki = readBlockFromDisk(blki, 119, &buf)) == NULL) {
        printf("read failed!\n");
        exit(-1);
    }

    for (int j = 0; j < 4; j++) {
        str[j] = *(blko + writeidx++) = *(blki + j);
    }
    *(blko + writeidx++) = '1';
    *(blko + writeidx++) = '1';
    *(blko + writeidx++) = '9';
    writeidx++;
    idxval = atoi(str) + 5;
    if (idxval > 160) {
        printf("only one index!\n");
        return 0;
    }
    for (int i = 1; i < 7; i++) {
        for (int j = 0; j < 4; j++) {
            str[j] = *(blki + i * 8 + j);
        }
        if (atoi(str) < idxval)continue;
        else {
            // printf("findone,i = %d,j = %d\n",i);
            trans(str, idxval);
            for (int k = 0; k < 4; k++) {
                *(blko + writeidx++) = str[k];
            }
            *(blko + writeidx++) = '1';
            *(blko + writeidx++) = '1';
            *(blko + writeidx++) = '9';
            writeidx++;
            idxval += 5;
        }
    }
    for (int i = 120; i <= 150; i++) {
        if ((blki = readBlockFromDisk(blki, i, &buf)) == NULL) {
            printf("read failed!\n");
            exit(-1);
        }
        for (int j = 0; j < 7; j++) {
            for (int k = 0; k < 4; k++) {
                str[k] = *(blki + j * 8 + k);
            }
            if (atoi(str) < idxval)continue;
            else {
                //   printf("findone,i = %d,j = %d\n",i,j);
                trans(str, idxval);
                for (int k = 0; k < 4; k++) {
                    *(blko + writeidx++) = str[k];
                    //      printf("now wirteidx = %d\n",writeidx);
                }
                trans(str, i);
                for (int k = 0; k < 4; k++) {
                    *(blko + writeidx++) = str[k];
                    //    printf("now wirteidx = %d\n",writeidx);
                }
                idxval += 5;
                if (writeidx == 56) {
                    leafindex++;
                    trans(str, leafindex);
                    for (int k = 0; k < 4; k++) {
                        *(blko + writeidx++) = str[k];
                        //       printf("now wirteidx = %d\n",writeidx);
                    }
                    if (writeBlockToDisk(blko, leafindex - 1, &buf) != 0) {
                        printf("write error!\n");
                        exit(-1);
                    }
                    writeidx = 0;
                }
            }
        }
    }
    writeidx = 56;
    leafindex++;
    trans(str, leafindex);
    for (int k = 0; k < 4; k++) {
        *(blko + writeidx++) = str[k];
        //       printf("now wirteidx = %d\n",writeidx);
    }
    if (writeBlockToDisk(blko, leafindex - 1, &buf) != 0) {
        printf("write error!\n");
        exit(-1);
    }
    for(int c = 0;c<64;c++)blko[c] = 0;
    // for(int c = 0;c<63;c++)printf("%c_",blko[c]);
    //printf("\n");
    freeBlockInBuffer(blki, &buf);
    freeBlockInBuffer(blko, &buf);
    //  printf("num io is %u\n",buf.numIO);
    buf.numIO = 0;
    return leafindex - 153;
}

int FindOnIndex(int aim, int nidx, Buffer buf) {
    unsigned char *blki, *blko, *blki2;
    char str[5];
    int writeidx = 0, readidx = 0, blkidx = 160, flag = 0;
    int cntt=0;
    blki = getNewBlockInBuffer(&buf);
    blko = getNewBlockInBuffer(&buf);
    blki2 = getNewBlockInBuffer(&buf);
    for (int i = 0; i < nidx; i++) {
        if ((blki = readBlockFromDisk(blki, i + 153, &buf)) == NULL) {
            printf("read failed!\n");
            exit(-1);
        }
        printf("read index block%d\n", i + 153);
        int mark = 0;
        for (int j = 0; j < 7; j++) {
            for (int k = 0; k < 4; k++) {
                str[k] = *(blki + j * 8 + k);
            }
            if (aim < atoi(str) + 5 || (i == nidx - 1 && (*(blki + j * 8 + 8) == 0) || j == 6)) {
                mark = 1;
                for (int k = 4; k < 8; k++) {
                    str[k - 4] = *(blki + j * 8 + k);
                }
                if ((blki2 = readBlockFromDisk(blki2, atoi(str), &buf)) == NULL) {
                    printf("read failed!\n");
                    exit(-1);
                }
                printf("read data block%d\n", atoi(str));
                for (int k = 0; k < 4; k++) {
                    str[k] = *(blki2 + k);
                }
                readidx = 0;
                while (atoi(str) < aim) {
                    if (readidx == 56) {
                        for (int k = 0; k < 4; k++) {
                            str[k] = *(blki2 + readidx + k);
                        }
                        if (atoi(str) > 150) {
                            printf("no find!\n");
                            freeBlockInBuffer(blki, &buf);
                            freeBlockInBuffer(blko, &buf);
                            freeBlockInBuffer(blki2, &buf);
                            return 0;
                        }
                        if ((blki2 = readBlockFromDisk(blki2, atoi(str), &buf)) == NULL) {
                            printf("read failed!\n");
                            exit(-1);
                        }
                        printf("read data block%d\n", atoi(str));
                        readidx = 0;
                    }
                    readidx += 8;
                    for (int k = 0; k < 4; k++) {
                        str[k] = *(blki2 + readidx + k);
                    }
                }
                while (atoi(str) == aim) {
                    cntt++;
                    printf("(X=%d,", atoi(str));
                    for (int k = 4; k < 8; k++) {
                        str[k - 4] = *(blki2 + readidx + k);
                    }
                    printf("Y=%d)\n", atoi(str));
                    for (int k = 0; k <= 7; k++) {
                        *(blko + writeidx++) = *(blki2 + readidx++);
                    }
                    //printf("now readidx is %d\n",readidx);
                    if (readidx == 56) {
                        for (int k = 0; k < 4; k++) {
                            str[k] = *(blki2 + readidx + k);
                        }
                        if ((blki2 = readBlockFromDisk(blki2, atoi(str), &buf)) == NULL) {
                            printf("read failed!\n");
                            exit(-1);
                        }
                        printf("read data block%d\n", atoi(str));
                        readidx = 0;
                    }
                    if (writeidx == 56) {
                        getaddr(blko,blkidx);
                        if (writeBlockToDisk(blko, blkidx++, &buf) != 0) {
                            printf("write error!\n");
                            exit(-1);
                        }
                        printf("Notice: the result was written in block%d \n", blkidx - 1);
                        for (int k = 0; k < 63; k++)blko[k] = 0;
                        writeidx = 0;
                    }
                    for (int k = 0; k < 4; k++) {
                        str[k] = *(blki2 + readidx + k);
                    }
                    flag = 1;
                }
                if (!flag) {
                    printf("no find!\n");
                    freeBlockInBuffer(blki, &buf);
                    freeBlockInBuffer(blko, &buf);
                    freeBlockInBuffer(blki2, &buf);
                    return 0;
                }
                break;
            }
        }
        if (flag)break;
        if(mark)printf("No tuple meet the request\n");
    }
    if (flag && writeidx != 0) {
        getaddr(blko,blkidx);
        if (writeBlockToDisk(blko, blkidx++, &buf) != 0) {
            printf("write error!\n");
            exit(-1);
        }
        printf("Notice: the result was written in block%d \n", blkidx - 1);
    }
    freeBlockInBuffer(blki, &buf);
    freeBlockInBuffer(blko, &buf);
    freeBlockInBuffer(blki2, &buf);
    printf("\nFound %d tuples meet the request\n\n",cntt);
    printf("Total number of IO is %lu\n", buf.numIO);
    return buf.numIO;
}

int sol3() {

    Buffer buf;
    if (!initBuffer(520, 64, &buf)) {
        perror("Buffer Initialization Failed!\n");
        return -1;
    }
    int aim, nidx;
    nidx = CreateIndex(buf);
    printf("-----------------------------------------------------------\n");
    printf("Selection algorithm based on index: S.C = ");
    scanf("%d", &aim);
    printf("-----------------------------------------------------------\n");
    FindOnIndex(aim, nidx, buf);
    return 0;
}

//aim4
int LinkRWS(Buffer buf) {
    unsigned char *blkir, *blko, *blkis;
    char str[5];
    int idxr = 0, idxs = 0, idxw = 0, blkr = 69, blks = 119, blkw = 180;
    int valr, vals;
    int flag = 0;
    int cntt =0 ;
    blkir = getNewBlockInBuffer(&buf);
    blkis = getNewBlockInBuffer(&buf);
    blko = getNewBlockInBuffer(&buf);
    if ((blkir = readBlockFromDisk(blkir, blkr++, &buf)) == NULL) {
        printf("read failed!\n");
        exit(-1);
    }
    if ((blkis = readBlockFromDisk(blkis, blks++, &buf)) == NULL) {
        printf("read failed!\n");
        exit(-1);
    }
    getfirst(blkir, 0, &valr);
    getfirst(blkis, 0, &vals);
    while (blkr <= 85 && blkr <= 151) {
        if (flag)break;
        while (valr != vals) {
            if (valr < vals) {
                idxr++;
                if (idxr == 7 || *(blkir + 8 * idxr) == 0) {
                    if (blkr == 85) {
                        flag = 1;
                        break;
                    }
                    if ((blkir = readBlockFromDisk(blkir, blkr++, &buf)) == NULL) {
                        printf("read failed!\n");
                        exit(-1);
                    }
                    idxr = 0;
                }
                getfirst(blkir, idxr, &valr);
            } else {
                idxs++;
                if (idxs == 7 || *(blkis + 8 * idxs) == 0) {
                    if (blks == 151) {
                        flag = 1;
                        break;
                    }
                    if ((blkis = readBlockFromDisk(blkis, blks++, &buf)) == NULL) {
                        printf("read failed!\n");
                        exit(-1);
                    }
                    idxs = 0;
                }
                getfirst(blkis, idxs, &vals);
            }
        }
       // printf("get!blkir = %d,idxr = %d |blkis = %d,idxs = %d\n", blkr, idxr, blks, idxs);
        if (flag)break;
        int hblkr = blkr - 1, hidxr = idxr;
        while (valr == vals) {//move s
            while (valr == vals) {//move r
               // printf("link(");
                for (int k = 0; k < 8; k++) {
                    *(blko + idxw++) = *(blkis + idxs * 8 + k);
                 //   printf("%c",*(blko + idxw-1));
                }
                cntt++;
                if (idxw == 56) {
                    getaddr(blko,blkw);
                    if (writeBlockToDisk(blko, blkw++, &buf) != 0) {
                        printf("write error!\n");
                        exit(-1);
                    }
                    printf("The result was written in block%d \n", blkw - 1);
                    idxw = 0;
                }
              //  printf(",");
                for (int k = 0; k < 8; k++) {
                    *(blko + idxw++) = *(blkir + idxr * 8 + k);
                   // printf("%c",*(blko + idxw-1));
                }
              //  printf(")\n");
                idxr++;
                if (idxw == 56) {
                    if (writeBlockToDisk(blko, blkw++, &buf) != 0) {
                        printf("write error!\n");
                        exit(-1);
                    }
                    printf("The result was written in block%d \n", blkw - 1);
                    idxw = 0;
                }
                if (idxr == 7 || *(blkir + 8 * idxr) == 0) {
                    if (blkr == 85) {
                        break;
                    }
                    if ((blkir = readBlockFromDisk(blkir, blkr++, &buf)) == NULL) {
                        printf("read failed!\n");
                        exit(-1);
                    }
                    idxr = 0;
                }
                getfirst(blkir, idxr, &valr);
            }
            idxs++;
            if (idxs == 7 || *(blkis + 8 * idxs) == 0) {
                if (blks == 151) {
                    flag = 1;
                    break;
                }
                if ((blkis = readBlockFromDisk(blkis, blks, &buf)) == NULL) {
                    //  printf("?");
                    printf("read failed!\n");
                    exit(-1);
                }
                blks++;
                idxs = 0;
            }
            getfirst(blkis, idxs, &vals);
            blkr = hblkr;
            idxr = hidxr;
            if ((blkir = readBlockFromDisk(blkir, blkr++, &buf)) == NULL) {
                printf("read failed!\n");
                exit(-1);
            }
            getfirst(blkir, idxr, &valr);
            //  printf("next\n");
        }
        if (flag)break;
    }
    if (idxw) {
        if (writeBlockToDisk(blko, blkw++, &buf) != 0) {
            printf("write error!\n");
            exit(-1);
        }
        for(int c = 0;c<64;c++)blko[c] = 0;
        idxw = 0;
    }
    printf("\nThe total number of operation link is %d\n",cntt);
    freeBlockInBuffer(blkir, &buf);
    freeBlockInBuffer(blko, &buf);
    freeBlockInBuffer(blkis, &buf);
}

int sol4() {
    Buffer buf;
    if (!initBuffer(520, 64, &buf)) {
        perror("Buffer Initialization Failed!\n");
        return -1;
    }
    printf("-----------------------------------------------------------\n");
    printf("Sort-Merge-Join algorithm\n");
    printf("-----------------------------------------------------------\n");
    LinkRWS(buf);
    return 0;
}

//aim5
void divide(int begin, int len, unsigned char *blk[10], Buffer buf, int now) {
    //divied
    int nowp = begin - 1;
    int cnt = 0, end, lent = len;
    while (lent) {
        cnt++;
        end = 0;
        for (int i = 0; i < 6 && lent; i++) {
            readBlockFromDisk(blk[i], ++nowp, &buf);
            lent--;
            end++;
        }
        sortblk(nowp - end + 1, blk, end, now, buf);
        now += end;
    }
}

void mergesort(int begin1, int end1, int begin2, int end2, int now, unsigned char *blk[10], Buffer buf) {
    int cnt1 = begin1, cnt2 = begin2, idx1 = 0, idx2 = 0, idxw = 0, val1 = 0, val2 = 0;
    if ((blk[1] = readBlockFromDisk(blk[1], cnt1, &buf)) == NULL) {
        printf("read failed!\n");
        exit(-1);
    }
    if ((blk[2] = readBlockFromDisk(blk[2], cnt2, &buf)) == NULL) {
        printf("read failed!\n");
        exit(-1);
    }
    while (cnt1 <= end1 && cnt2 <= end2) {
        getfirst(blk[1], idx1, &val1);
        getfirst(blk[2], idx2, &val2);
        if (val1 < val2) {
            for (int k = 0; k < 8; k++) {
                *(blk[3] + idxw++) = *(blk[1] + idx1 * 8 + k);
            }
            idx1++;
        } else {
            for (int k = 0; k < 8; k++) {
                *(blk[3] + idxw++) = *(blk[2] + idx2 * 8 + k);
            }
            idx2++;
        }
        if (idxw == 56) {
            if (writeBlockToDisk(blk[3], now++, &buf) != 0) {
                printf("write error!\n");
                exit(-1);
            }
            for (int i = 0; i < 56; i++)*(blk[3]) = 0;
            idxw = 0;
        }
        if (idx1 == 7 || *(blk[1] + 8 * idx1) == 0) {
            if (cnt1 == end1) {
                break;
            }
            if ((blk[1] = readBlockFromDisk(blk[1], ++cnt1, &buf)) == NULL) {
                printf("read failed!\n");
                exit(-1);
            }
            idx1 = 0;
        }
        if (idx2 == 7 || *(blk[2] + 8 * idx2) == 0) {
            if (cnt2 == end2) {
                break;
            }
            if ((blk[2] = readBlockFromDisk(blk[2], ++cnt2, &buf)) == NULL) {
                printf("read failed!\n");
                exit(-1);
            }
            idx2 = 0;
        }
    }
    while (cnt1 != end1 || (end1 == cnt1 && idx1 != 7 && *(blk[1] + 8 * idx1) != 0)) {
        for (int k = 0; k < 8; k++) {
            *(blk[3] + idxw++) = *(blk[1] + idx1 * 8 + k);
        }
        idx1++;
        if (idxw == 56) {
            if (writeBlockToDisk(blk[3], now++, &buf) != 0) {
                printf("write error!\n");
                exit(-1);
            }
            for (int i = 0; i < 56; i++)*(blk[3]) = 0;
            idxw = 0;
        }
        if (idx1 == 7 || *(blk[1] + 8 * idx1) == 0) {
            if (cnt1 == end1) {
                break;
            }
            if ((blk[1] = readBlockFromDisk(blk[1], ++cnt1, &buf)) == NULL) {
                printf("read failed!\n");
                exit(-1);
            }
            idx1 = 0;
        }
    }
    while (cnt2 != end2 || (end2 == cnt2 && idx2 != 7 && *(blk[2] + 8 * idx2) != 0)) {
        for (int k = 0; k < 8; k++) {
            *(blk[3] + idxw++) = *(blk[2] + idx2 * 8 + k);
        }
        idx2++;
        if (idxw == 56) {
            if (writeBlockToDisk(blk[3], now++, &buf) != 0) {
                printf("write error!\n");
                exit(-1);
            }
            for (int i = 0; i < 56; i++)*(blk[3]) = 0;
            idxw = 0;
        }
        if (idx2 == 7 || *(blk[2] + 8 * idx2) == 0) {
            if (cnt2 == end2) {
                break;
            }
            if ((blk[2] = readBlockFromDisk(blk[2], ++cnt2, &buf)) == NULL) {
                printf("read failed!\n");
                exit(-1);
            }
            idx2 = 0;
        }
    }
    if (idxw != 0) {
        if (writeBlockToDisk(blk[3], now++, &buf) != 0) {
            printf("write error!\n");
            exit(-1);
        }
        for (int i = 0; i < 56; i++)*(blk[3]) = 0;
        idxw = 0;
    }
}

void merge(int op,unsigned char *blk[10],Buffer buf) {
    int idxb[6],idxt[6],end[6],idxw = 0,cntw = 380,rend[6],reach = 1;
    for(int i = 280;i<=295;i+=6){
        idxb[(i-280)/6] = i;
        idxt[(i-280)/6] = 0;
        end[(i-280)/6] = i+5;
        rend[(i-280)/6] = 0;
        if ((blk[(i-280)/6] = readBlockFromDisk(blk[(i-280)/6], i, &buf)) == NULL) {
            printf("read failed!\n");
            exit(-1);
        }
    }
    end[2] -=2;
    for(int i = 340;i<=371;i+=12){
        idxb[(i-340)/12+3] = i;
        idxt[(i-340)/12+3] = 0;
        end[(i-340)/12+3] = i+11;
        rend[(i-340)/12+3] = 0;
        if ((blk[(i-340)/12+3] = readBlockFromDisk(blk[(i-340)/12+3], i, &buf)) == NULL) {
            printf("read failed!\n");
            exit(-1);
        }
    }
    end[5] -=4;
    int min1 = 0,min2 = 0;
    while (reach) {
        int i,j;
        min1 = min2 = -1;
        for (i = 0; i < 3; i++) {
            if (idxt[i] < 7&&!rend[i]) {
                min1 = i++;
                break;
            }
        }
        for (; i < 3; i++) {
            if (idxt[i] < 7 &&!rend[i]&& cmpblk(blk[i] + idxt[i]*8, blk[min1] + idxt[min1]*8)) {
                min1 = i;
            }
        }

        for (i = 3; i < 6; i++) {
            if (idxt[i] < 7&&!rend[i]) {
                min2 = i++;
                break;
            }
        }
        for (; i < 6 ; i++) {
            if (idxt[i] < 7 &&!rend[i]&& cmpblk(blk[i] + idxt[i]*8, blk[min2] + 8*idxt[min2])) {
                min2 = i;
            }
        }

        //judge
        //judge(blk,buf,min1,min2,op,idxt,idxb,end,&idxw,&cntw,&cnt1,&cnt2);

        if(min1 == -1) {
            for (int k = 0; k < 8; k++) {
                *(blk[6] + idxw++) = *(blk[min2] + idxt[min2] * 8 + k);
            }
            idxt[min2]++;
            printf("now idxb[min2] is %d\n",idxb[min2]);
            if (idxt[min2] == 7 || *(blk[min2] + 8 * idxt[min2]) == 0) {
                if (idxb[min2] != end[min2]) {
                    if ((blk[min2] = readBlockFromDisk(blk[min2], ++idxb[min2], &buf)) == NULL) {
                        printf("read failed!\n");
                        exit(-1);
                    }
                    idxt[min2] = 0;
                }
                else rend[min2] = 1;
            }
        }
        else if(min2 == -1){
            for (int k = 0; k < 8; k++) {
                *(blk[6] + idxw++) = *(blk[min1] + idxt[min1] * 8 + k);
            }
            idxt[min1]++;
            if (idxt[min1] == 7 || *(blk[min1] + 8 * idxt[min1]) == 0) {
                if (idxb[min1] != end[min1]) {
                    if ((blk[min1] = readBlockFromDisk(blk[min1], ++idxb[min1], &buf)) == NULL) {
                        printf("read failed!\n");
                        exit(-1);
                    }
                    idxt[min1] = 0;
                }
                else rend[min1] = 1;
            }
        }
        else if(op == 0){
            if(isequal(blk[min1] + 8*idxt[min1],blk[min2]+8*idxt[min2])){
                for (int k = 0; k < 8; k++) {
                    *(blk[6] + idxw++) = *(blk[min1] + idxt[min1] * 8 + k);
                }
            }
            else{
                for (int k = 0; k < 8; k++) {
                    *(blk[6] + idxw++) = *(blk[min1] + idxt[min1] * 8 + k);
                }
                if (idxw == 56) {
                    if (writeBlockToDisk(blk[6], cntw++, &buf) != 0) {
                        printf("write error!\n");
                        exit(-1);
                    }
                    idxw = 0;
                    for(int c = 0;c<64;c++)blk[6][c] = 0;
                }
                for (int k = 0; k < 8; k++) {
                    *(blk[6] + idxw++) = *(blk[min1] + idxt[min1] * 8 + k);
                }
            }
            idxt[min1]++;
            if (idxt[min1] == 7 || *(blk[min1] + 8 * idxt[min1]) == 0) {
                if (idxb[min1] != end[min1]) {
                    if ((blk[min1] = readBlockFromDisk(blk[min1], ++idxb[min1], &buf)) == NULL) {
                        printf("read failed!\n");
                        exit(-1);
                    }
                    idxt[min1] = 0;
                }
                else rend[min1] = 1;
            }
            idxt[min2]++;
            printf("now idxb[min2] is %d\n",idxb[min2]);
            if (idxt[min2] == 7 || *(blk[min2] + 8 * idxt[min2]) == 0) {
                if (idxb[min2] != end[min2]) {
                    if ((blk[min2] = readBlockFromDisk(blk[min2], ++idxb[min2], &buf)) == NULL) {
                        printf("read failed!\n");
                        exit(-1);
                    }
                    idxt[min2] = 0;
                }
                else rend[min2] = 1;
            }
        }
        else if(op == 1){
            if(isequal(blk[min1] + 8*idxt[min1],blk[min2]+8*idxt[min2])){
                for (int k = 0; k < 8; k++) {
                    *(blk[6] + idxw++) = *(blk[min1] + idxt[min1] * 8 + k);
                }
                idxt[min1]++;
                if (idxt[min1] == 7 || *(blk[min1] + 8 * idxt[min1]) == 0) {
                    if (idxb[min1] != end[min1]) {
                        if ((blk[min1] = readBlockFromDisk(blk[min1], ++idxb[min1], &buf)) == NULL) {
                            printf("read failed!\n");
                            exit(-1);
                        }
                        idxt[min1] = 0;
                    }
                    else rend[min1] = 1;
                }
                idxt[min2]++;
                printf("now idxb[min2] is %d\n",idxb[min2]);
                if (idxt[min2] == 7 || *(blk[min2] + 8 * idxt[min2]) == 0) {
                    if (idxb[min2] != end[min2]) {
                        if ((blk[min2] = readBlockFromDisk(blk[min2], ++idxb[min2], &buf)) == NULL) {
                            printf("read failed!\n");
                            exit(-1);
                        }
                        idxt[min2] = 0;
                    }
                    else rend[min2] = 1;
                }
            }
            else {
                if (cmpblk(blk[min1] + 8 * idxt[min1], blk[min2] + 8 * idxt[min2])) {
                    idxt[min1]++;
                    if (idxt[min1] == 7 || *(blk[min1] + 8 * idxt[min1]) == 0) {
                        if (idxb[min1] != end[min1]) {
                            if ((blk[min1] = readBlockFromDisk(blk[min1], ++idxb[min1], &buf)) == NULL) {
                                printf("read failed!\n");
                                exit(-1);
                            }
                            idxt[min1] = 0;
                        }
                        else rend[min1] = 1;
                    }

                } else {
                    idxt[min2]++;
                    printf("now idxb[min2] is %d\n",idxb[min2]);
                    if (idxt[min2] == 7 || *(blk[min2] + 8 * idxt[min2]) == 0) {
                        if (idxb[min2] != end[min2]) {
                            if ((blk[min2] = readBlockFromDisk(blk[min2], ++idxb[min2], &buf)) == NULL) {
                                printf("read failed!\n");
                                exit(-1);
                            }
                            idxt[min2] = 0;
                        }
                        else rend[min2] = 1;
                    }
                }
            }
        }
        else if(op == 2){
            if(isequal(blk[min1] + 8*idxt[min1],blk[min2]+8*idxt[min2])){
                idxt[min2]++;
                printf("now idxb[min2] is %d\n",idxb[min2]);
                if (idxt[min2] == 7 || *(blk[min2] + 8 * idxt[min2]) == 0) {
                    if (idxb[min2] != end[min2]) {
                        if ((blk[min2] = readBlockFromDisk(blk[min2], ++idxb[min2], &buf)) == NULL) {
                            printf("read failed!\n");
                            exit(-1);
                        }
                        idxt[min2] = 0;
                    }
                    else rend[min2] = 1;
                }
            }
            else {
                if (cmpblk(blk[min1] + 8 * idxt[min1], blk[min2] + 8 * idxt[min2])) {
                    idxt[min1]++;
                    if (idxt[min1] == 7 || *(blk[min1] + 8 * idxt[min1]) == 0) {
                        if (idxb[min1] != end[min1]) {
                            if ((blk[min1] = readBlockFromDisk(blk[min1], ++idxb[min1], &buf)) == NULL) {
                                printf("read failed!\n");
                                exit(-1);
                            }
                            idxt[min1] = 0;
                        }
                        else rend[min1] = 1;
                    }
                } else {
                    for (int k = 0; k < 8; k++) {
                        *(blk[6] + idxw++) = *(blk[min2] + idxt[min2] * 8 + k);
                    }
                    idxt[min2]++;
                    printf("now idxb[min2] is %d\n",idxb[min2]);
                    if (idxt[min2] == 7 || *(blk[min2] + 8 * idxt[min2]) == 0) {
                        if (idxb[min2] != end[min2]) {
                            if ((blk[min2] = readBlockFromDisk(blk[min2], ++idxb[min2], &buf)) == NULL) {
                                printf("read failed!\n");
                                exit(-1);
                            }
                            idxt[min2] = 0;
                        }
                        else rend[min2] = 1;
                    }
                }
            }
        }

        if (idxw == 56) {
            if (writeBlockToDisk(blk[6], cntw++, &buf) != 0) {
                printf("write error!\n");
                exit(-1);
            }
            idxw = 0;
            for(int c = 0;c<64;c++)blk[6][c] = 0;
        }
        reach = !(rend[0] && rend[1] && rend[2] && rend[3] && rend[4] && rend[5]);
        for(int i = 0;i<6;i++)printf("now end%d = %d,idxb = %d,rendi = %d,idxt = %d\n",i,end[i],idxb[i],rend[i],idxt[i]);
        printf("\n");
    }
    if (idxw) {
        if (writeBlockToDisk(blk[6], cntw++, &buf) != 0) {
            printf("write error!\n");
            exit(-1);
        }
        idxw = 0;
        for(int c = 0;c<64;c++)blk[6][c] = 0;
    }

}

int sol5() {
    Buffer buf; /* A buffer */
    if (!initBuffer(520, 64, &buf)) {
        perror("Buffer Initialization Failed!\n");
        return -1;
    }
    unsigned char *blk[10];
    for (int i = 0; i < 7; i++) {
        blk[i] = getNewBlockInBuffer(&buf);
    }
    divide(1, 16, blk, buf, 280);//280-295
    divide(17, 32, blk, buf, 300);
    mergesort(300, 305, 306, 311, 340, blk, buf);
    mergesort(312, 317, 318, 323, 352, blk, buf);
    mergesort(324, 329, 330, 331, 364, blk, buf);
    printf("cut here\n");
    merge(0,blk,buf);
}

//aimex
int solex1(){
    Buffer buf; /* A buffer */
    if (!initBuffer(520, 64, &buf)) {
        perror("Buffer Initialization Failed!\n");
        return -1;
    }
    unsigned char *blk[10];
    for (int i = 0; i < 7; i++) {
        blk[i] = getNewBlockInBuffer(&buf);
    }
    divide(1, 16, blk, buf, 280);//280-295
    divide(17, 32, blk, buf, 300);
    mergesort(300, 305, 306, 311, 340, blk, buf);
    mergesort(312, 317, 318, 323, 352, blk, buf);
    mergesort(324, 329, 330, 331, 364, blk, buf);
    printf("cut here\n");
    merge(1,blk,buf);
}
int solex2(){
    Buffer buf; /* A buffer */
    if (!initBuffer(520, 64, &buf)) {
        perror("Buffer Initialization Failed!\n");
        return -1;
    }
    unsigned char *blk[10];
    for (int i = 0; i < 7; i++) {
        blk[i] = getNewBlockInBuffer(&buf);
    }
    divide(1, 16, blk, buf, 280);//280-295
    divide(17, 32, blk, buf, 300);
    mergesort(300, 305, 306, 311, 340, blk, buf);
    mergesort(312, 317, 318, 323, 352, blk, buf);
    mergesort(324, 329, 330, 331, 364, blk, buf);
    printf("cut here\n");
    merge(2,blk,buf);
}

int main() {
    sol1();
    //sol2();
    //sol3();
    //sol4();
    //sol5();
    //solex1();
    //solex2();
    return 0;
}

