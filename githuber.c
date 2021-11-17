//
// Created by yagvil on 2021/11/17.
//
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "extmem.h"

#define BLKSIZE 64
#define FINISHED 9999

typedef struct tuple
{
    int x;
    int y;
}T;
T tuple_value;
int read_tuple(unsigned char *blk, int num)
{
    tuple_value.x = 0;
    tuple_value.y = 0;
    // char x_char[4], y_char[4];
    unsigned char *new_blk = blk + (num - 1) * 8;
    for (int i = 0; i <= 3; i++)
    {
        if(*new_blk != 0 && *new_blk != ' ')
        {
            tuple_value.x *= 10;
            tuple_value.x += *new_blk - '0';
        }
        new_blk++;
    }
    for (int i = 0; i <= 3; i++)
    {
        if(*new_blk != 0 && *new_blk != ' ')
        {
            tuple_value.y *= 10;
            tuple_value.y += *new_blk - '0';
        }
        new_blk++;
    }
    return tuple_value.x;
}

//向内存中写一个元组
void write_tuple(unsigned char *blk, int num)
{
    // tuple_value.x = 257;
    int x = tuple_value.x;
    int y = tuple_value.y;
    char char_x[4];
    char char_y[4];
    unsigned char *new_blk = blk + (num - 1) * 8;

    int value_long = 0;

    for (int i = 0; i <= 3; i++)
    {
        if(x != 0)
        {
            char_x[i] = (x % 10) + '0';
            x = x / 10;
            value_long++;
        }
        else
        {
            char_x[i] = 0;
        }
    }
    for (int i = 0; i <= 3; i++)
    {
        if(value_long != 0)
        {
            value_long--;
            *new_blk = char_x[value_long];
        }
        else
        {
            *new_blk = '\0';
        }
        new_blk++;
    }

    value_long = 0;

    for (int i = 0; i <= 3; i++)
    {
        if(y != 0)
        {
            char_y[i] = (y % 10) + '0';
            y = y / 10;
            value_long++;
        }
        else
        {
            char_y[i] = 0;
        }
    }
    for (int i = 0; i <= 3; i++)
    {
        if(value_long != 0)
        {
            value_long--;
            *new_blk = char_y[value_long];
        }
        else
        {
            *new_blk = '\0';
        }
        new_blk++;
    }
}
unsigned char *getNewBlockInBuffer_clear(Buffer *buf)
{
    unsigned char *blkPtr;
    blkPtr = getNewBlockInBuffer(buf);
    memset(blkPtr, 0, (buf->blkSize) * sizeof(unsigned char));
}

int cmp(const void *a, const void *b)
{
    T c = *(T *)a;
    T d = *(T *)b;
    return c.x - d.x;
}

//找到最小值位置
int find_min_position(unsigned char *blk)
{
    int min_position = 1;
    int min_value = FINISHED;
    for (int i = 1; i <= 8;i++)
    {
        read_tuple(blk, i);
        if(tuple_value.x < min_value)
        {
            min_value = tuple_value.x;
            min_position = i;
        }
    }
    return min_position;
}

int create_index(int rstart, int rfinish, int index_start, Buffer *buf)
{
    unsigned char *blk;
    unsigned char *wblk;
    int index = 1;
    int index_finish = index_start;


    wblk = getNewBlockInBuffer_clear(buf);

    // printf("从%d磁盘块开始建立索引直到");
    for (; rstart <= rfinish; rstart++)
    {
        if ((blk = readBlockFromDisk(rstart, buf)) == NULL)
        {
            perror("Reading Block Failed!\n");
            return -1;
        }
        read_tuple(blk, 1);
        tuple_value.y = rstart;
        write_tuple(wblk, index);
        index++;
        if(index == 8)
        {
            tuple_value.x = index_finish + 1;
            tuple_value.y = 0;
            write_tuple(wblk, index);

            index = 1;
            if (writeBlockToDisk(wblk, index_finish, buf) != 0)
            {
                perror("Writing Block Failed!\n");
                return -1;
            }
            wblk = getNewBlockInBuffer_clear(buf);
            index_finish++;
        }
        freeBlockInBuffer(blk, buf);
    }
    if(index != 1)
    {
        tuple_value.x = index_finish + 1;
        tuple_value.y = 0;
        write_tuple(wblk, 8);
        if (writeBlockToDisk(wblk, index_finish, buf) != 0)
        {
            perror("Writing Block Failed!\n");
            return -1;
        }
        index_finish++;
    }
    else
    {
        freeBlockInBuffer(wblk, buf);
    }
    printf("从%d磁盘块开始建立索引直到%d块\n", index_start, index_finish-1);
    return index_finish - 1;
}

int linear_select_search(int search_start, int search_finish, int wfinish, Buffer *buf)
{
    unsigned char *blk;
    unsigned char *wblk;
    int m = 1;
    int find_result_num = 0;

    wblk = getNewBlockInBuffer_clear(buf);

    for (int i = search_start; i <= search_finish; i++)
    {
        if ((blk = readBlockFromDisk(i, buf)) == NULL)
        {
            perror("Reading Block Failed!\n");
            return -1;
        }
        else
        {
            printf("读入数据块%d\n", i);

            for (int j = 1; j <= 7;j++)
            {
                read_tuple(blk, j);
                // printf("%d, %d\n", tuple_value.x, tuple_value.y);
                if(tuple_value.x == 30)
                {
                    find_result_num++;
                    //一个内存块存满了，先把他写入磁盘
                    if(m > 7)
                    {
                        m = 1;
                        tuple_value.x = wfinish + 1;
                        tuple_value.y = 0;
                        write_tuple(wblk, 8);
                        if (writeBlockToDisk(wblk, wfinish, buf) != 0)
                        {
                            perror("Writing Block Failed!\n");
                            return -1;
                        }
                        wblk = getNewBlockInBuffer_clear(buf);
                        printf("注：结果写入磁盘%d\n", wfinish);
                        wfinish++;
                        // freeBlockInBuffer(wblk, &buf);
                    }
                    printf("(X=%d, Y=%d)\n", tuple_value.x, tuple_value.y);
                    write_tuple(wblk, m);
                    m++;
                }
            }
            freeBlockInBuffer(blk, buf);
        }
    }
    //结果写入磁盘
    tuple_value.x = wfinish + 1;
    tuple_value.y = 0;
    write_tuple(wblk, 8);
    if (writeBlockToDisk(wblk, wfinish, buf) != 0)
    {
        perror("Writing Block Failed!\n");
        return -1;
    }
    printf("注：结果写入磁盘%d\n", wfinish);
    printf("满足条件的元组一共有%d个\n", find_result_num);
    printf("I/O读写一共%d次\n", buf->numIO);

    return 0;
}


int select_sort_in_buf(int start_blk, int finish_blk, Buffer *buf)
{

    int blk_num = finish_blk - start_blk + 1;         //磁盘块数量

    int m = (finish_blk - start_blk) / (buf->numAllBlk - 1) + 1;//划分为m个子集合  这里是7个一组，内存为8，满足7<8

    int now_blk = start_blk;                       //当前磁盘块
    int blk_cnt = 0;
    unsigned char *blk;
    unsigned char *blks[buf->numAllBlk - 1];
    int sstart = start_blk;
    T t1, t2;

    //按组遍历
    for (int i = 0; i < m; i++)
    {
        blk_cnt = 0;
        for (; blk_cnt < (buf->numAllBlk - 1) && now_blk <= finish_blk; now_blk++, blk_cnt++) {
            if ((blks[blk_cnt] = readBlockFromDisk(now_blk, buf)) == NULL) {
                perror("Reading Block Failed!\n");
                exit(-1);
            }
        }
        //内排序
        for (int j = 0; j < blk_cnt * 7 - 1; j++) {
            int tuple_j_index = j / 7;
            int tuple_j_offset = j % 7 + 1;
            for (int k = j + 1; k < blk_cnt * 7; k++) {
                int tuple_k_index = k / 7;
                int tuple_k_offset = k % 7 + 1;
                read_tuple(blks[tuple_j_index], tuple_j_offset);
                t1 = tuple_value;
                read_tuple(blks[tuple_k_index], tuple_k_offset);
                t2 = tuple_value;
                if (t1.x > t2.x) {
                    tuple_value = t2;
                    write_tuple(blks[tuple_j_index], tuple_j_offset);
                    tuple_value = t1;
                    write_tuple(blks[tuple_k_index], tuple_k_offset);
                }
            }
        }

        for (int i = 0; i < blk_cnt; i++)
        {
            blk = blks[i];
            tuple_value.x = sstart + 1;
            tuple_value.y = 0;
            write_tuple(blk, 8);
            if (writeBlockToDisk(blk, sstart, buf) != 0)
            {
                perror("Writing Block Failed!\n");
                return -1;
            }
            sstart++;

        }
    }
}

int sort_merge_join(int R_sort_start, int R_sort_finish, int S_sort_start, int S_sort_finish, int result_start)
{
    printf("----------基于排序的连接操作算法----------\n\n");
    Buffer buf;
    if (!initBuffer(520, 64, &buf))
    {
        perror("Buffer Initialization Failed!\n");
        return -1;
    }

    unsigned char *blk_R;
    unsigned char *blk_S;
    unsigned char *wblk;
    int last_file = 0;
    int blk_R_number = R_sort_start;
    int blk_R_index = 1;
    int finish = 0;
    int first = 1;
    int last_value_S = 0;
    int now_blk_R_number = R_sort_start;
    int result_finish = result_start;
    int save_blk_R_number;
    int save_blk_R_index;
    T value_S;
    T value_R;

    int wblk_index = 1;
    int j_blk_R_index = 1;

    int count = 0;
    int remember_value_R;
    int find = 0;
    //用于写的内存块
    wblk = getNewBlockInBuffer_clear(&buf);

    int last_sort_start;
    int last_i;
    int repeat = 0;
    //对于S的每一块
    for (; S_sort_start <= S_sort_finish;S_sort_start++)
    {
        // printf("%d\n", S_sort_start);
        if ((blk_S = readBlockFromDisk(S_sort_start, &buf)) == NULL)
        {
            perror("Reading R-Block Failed!\n");
            return -1;
        }
        //对S每一块的七个值
        for (int i = 1; i <= 7; i++)
        {
            //同一个S值重复两次读，说明之后的R值一直小于S（R文件的最大值小于当前S值），即join完成了
            if(repeat == 2)
            {
                continue;
            }
            if(S_sort_start == last_sort_start && i == last_i)
            {
                repeat++;
            }
            else
            {
                last_sort_start = S_sort_start;
                last_i = i;
                repeat = 0;
            }

            read_tuple(blk_S, i);
            value_S.x = tuple_value.x;
            value_S.y = tuple_value.y;


            //第一次找R的值
            if(first == 1)
            {
                find = 0;
                while (now_blk_R_number <= R_sort_finish && find == 0)
                {
                    if(last_file != now_blk_R_number)
                    {
                        if(last_file!=0)
                        {
                            freeBlockInBuffer(blk_R, &buf);
                        }
                        if ((blk_R = readBlockFromDisk(now_blk_R_number, &buf)) == NULL)
                        {
                            perror("Reading R-Block Failed!\n");
                            return -1;
                        }
                        last_file = now_blk_R_number;
                    }
                    else
                    {

                    }
                    for (; j_blk_R_index <= 7; j_blk_R_index++)
                    {
                        read_tuple(blk_R, j_blk_R_index);
                        value_R.x = tuple_value.x;
                        value_R.y = tuple_value.y;
                        if (value_R.x >= value_S.x)
                        {
                            find = 1;
                            break;
                        }
                    }
                    //R>=S的位置，记录value_R的值和位置
                    remember_value_R = value_R.x;
                    blk_R_number = now_blk_R_number;
                    blk_R_index = j_blk_R_index;
                    j_blk_R_index = 1;
                    now_blk_R_number++;
                    // freeBlockInBuffer(blk_R, &buf);
                }
                first = 0;
            }
            //R大则S向后推
            if(remember_value_R > value_S.x)
            {
                continue;
            }
                //相等开始join
            else if(remember_value_R == value_S.x)
            {
                now_blk_R_number = blk_R_number;
                j_blk_R_index = blk_R_index;
                find = 0;
                // printf("%d\n", now_blk_R_number);
                while (now_blk_R_number<=R_sort_finish && find == 0)
                {
                    if(last_file != now_blk_R_number)
                    {
                        freeBlockInBuffer(blk_R, &buf);
                        if ((blk_R = readBlockFromDisk(now_blk_R_number, &buf)) == NULL)
                        {
                            perror("Reading R-Block Failed!\n");
                            return -1;
                        }
                        last_file = now_blk_R_number;
                    }
                    else
                    {

                    }
                    for (; j_blk_R_index <= 7;j_blk_R_index++)
                    {
                        read_tuple(blk_R, j_blk_R_index);
                        value_R.x = tuple_value.x;
                        value_R.y = tuple_value.y;
                        if(value_R.x == value_S.x)
                        {
                            // 写入wblk；
                            count++;
                            // printf("%d\n", count);
                            tuple_value.x = value_S.x;
                            tuple_value.y = value_S.y;
                            write_tuple(wblk, wblk_index);
                            wblk_index++;
                            if(wblk_index == 8)
                            {
                                printf("注：结果写入磁盘%d\n", result_finish);
                                wblk_index = 1;
                                tuple_value.x = result_finish + 1;
                                tuple_value.y = 0;
                                write_tuple(wblk, 8);
                                if (writeBlockToDisk(wblk, result_finish, &buf) != 0)
                                {
                                    perror("Writing Block Failed!\n");
                                    return -1;
                                }
                                wblk = getNewBlockInBuffer_clear(&buf);
                                result_finish++;
                            }
                            tuple_value.x = value_R.x;
                            tuple_value.y = value_R.y;
                            write_tuple(wblk, wblk_index);
                            wblk_index++;
                            if(wblk_index == 8)
                            {
                                printf("注：结果写入磁盘%d\n", result_finish);
                                wblk_index = 1;
                                tuple_value.x = result_finish + 1;
                                tuple_value.y = 0;
                                write_tuple(wblk, 8);
                                if (writeBlockToDisk(wblk, result_finish, &buf) != 0)
                                {
                                    perror("Writing Block Failed!\n");
                                    return -1;
                                }
                                wblk = getNewBlockInBuffer_clear(&buf);
                                result_finish++;
                            }
                        }
                            //不相等结束join
                        else
                        {
                            find = 1;
                            break;
                        }
                        // freeBlockInBuffer(blk_R, &buf);
                    }
                    now_blk_R_number++;
                    j_blk_R_index = 1;
                }
            }
                //R小则R向后推直到找到大于等于S的R的值和位置
            else//value_R.x < value_S.x
            {
                i--;
                now_blk_R_number = blk_R_number;
                j_blk_R_index = blk_R_index;
                find = 0;
                while (now_blk_R_number<=R_sort_finish && find == 0)
                {
                    if(last_file != now_blk_R_number)
                    {
                        freeBlockInBuffer(blk_R, &buf);
                        if ((blk_R = readBlockFromDisk(now_blk_R_number, &buf)) == NULL)
                        {
                            perror("Reading R-Block Failed!\n");
                            return -1;
                        }
                        last_file = now_blk_R_number;
                    }
                    else
                    {

                    }
                    for (; j_blk_R_index <= 7;j_blk_R_index++)
                    {
                        read_tuple(blk_R, j_blk_R_index);
                        value_R.x = tuple_value.x;
                        value_R.y = tuple_value.y;
                        if(value_R.x >= value_S.x)
                        {
                            find = 1;
                            break;
                        }
                    }
                    //R>=S的位置，记录value_R的值和位置
                    remember_value_R = value_R.x;
                    blk_R_number = now_blk_R_number;
                    blk_R_index = j_blk_R_index;
                    // freeBlockInBuffer(blk_R, &buf);
                    now_blk_R_number++;
                    j_blk_R_index = 1;
                }
            }
        }
        freeBlockInBuffer(blk_S, &buf);
    }
    if(wblk_index != 1)
    {
        printf("注：结果写入磁盘%d\n", result_finish);
        tuple_value.x = result_finish + 1;
        tuple_value.y = 0;
        write_tuple(wblk, 8);
        if (writeBlockToDisk(wblk, result_finish, &buf) != 0)
        {
            perror("Writing Block Failed!\n");
            return -1;
        }
    }
    printf("总共连接%d次\n", count);
    freeBuffer(&buf);
}
int main()
{
    //line(130);
    //TPMMS();
    //index_select(130);
    sort_merge_join(301,316,317,348,600);
}
