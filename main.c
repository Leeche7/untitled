#include <stdlib.h>
#include <stdio.h>
#include "extmem.h"

//插桩调试用

typedef struct
{
    int x;
    int y;
}tuple;
void write_tuple(unsigned char *blk, int num,tuple tuple_value)
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

void blkshow(unsigned char *blk)
{
    int i;
    char str[5];
    int X;
    int Y;
    for (i = 0; i < 7; i++) //一个blk存7个元组加一个地址
    {

        for (int k = 0; k < 4; k++)
        {
            str[k] = *(blk + i*8 + k);
        }
        X = atoi(str);
        for (int k = 0; k < 4; k++)
        {
            str[k] = *(blk + i*8 + 4 + k);
        }
        Y = atoi(str);
        printf("(%d, %d) ", X, Y);
    }
    printf("\n");
}
//写入写缓冲区
void write_buffer(char *x, char *y, unsigned char *blk ,int blocknumber)
{
    for(int k=0; k < 4; k++)
    {
        *(blk + blocknumber*8 + k) = x[k];
        *(blk + blocknumber*8 + k + 4) = y[k];
    }
}
//交换blk缓冲区块两个元组中的数据
void changeblk(unsigned char *blk1, unsigned char *blk2)
{
    char temp;
    for (int k = 0; k < 8; k++) {
        temp = *(blk1 + k);
        *(blk1 + k) = *(blk2 + k);
        *(blk2 + k) = temp;
    }
    return;
}


int blkcmp(unsigned char *blk1,unsigned char *blk2)
{
    char x1[10], y1[10],x2[10],y2[10];
    for(int k=0;k<4;k++)
    {
        x1[k] = *(blk1 + k);
        x2[k] = *(blk2 + k);
    }
    for(int k=4;k<8;k++)
    {
        y1[k] = *(blk1 + k);
        y2[k] = *(blk2 + k);
    }
    if(atoi(x1)> atoi(x2))
        return 1;
    else if(atoi(x1)== atoi(x2))
    {
        if(atoi(y1)> atoi(y2))
            return 1;
        else
            return -1;
    }
    return -1;
}

//内部排序，做到后面发现好像还要分个组
void blkinsort(int start, int end,Buffer buffer,unsigned char *blk,int blknumber)
{
    unsigned char *nextblk;
    char str_x[5];
    char str_y[5];
    blknumber = blknumber + start;
    for(int i=start;i<=end;i++)
    {
        if ((blk = readBlockFromDisk(i, &buffer)) == NULL)
        {
            perror("Reading Block Failed!\n");
            return;
        }
        //blkshow(blk);
        for(int j = 0; j < 7;j++)
        {
            for (nextblk = blk;nextblk < blk + 48 ; nextblk += 8)
            {
                if(blkcmp(nextblk,nextblk+8)==1)
                {
                    changeblk(nextblk, nextblk + 8);
                }
                //blkshow(blk);
                continue;
            }
        }
        //printf("finnally\n");
        //blkshow(blk);
        dropBlockOnDisk(blknumber);
        if (writeBlockToDisk(blk, blknumber, &buffer) != 0)
        {
            perror("Writing Block Failed!\n");
            return;
        }
        blknumber++;
        freeBlockInBuffer(blk,&buffer);
    }
    //blkshow(blk);
    return;
}

//读取blk块的第num个数
tuple read_tuple(unsigned char *blk, int num)
{
    tuple tuple_value;
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
    return tuple_value;
}
//分组之后的内排序
//输入开始读入块号，结束块号，内存空间，写入块基址
void groupinsort(int start,int end,Buffer* buffer,int blknumber)
{
    int blk_num = end - start + 1;         //磁盘块数量
    int m = (end - start + 1) / 6 + 1;//内排序也需要分组
    int now_blk = start;
    int blk_cnt = 0;
    unsigned char *blk;
    unsigned char *blks[6];//一次只用6块buffer中的空间，相当于我一次读6个blk
    int sstart = blknumber + start;
    tuple t1,t2;
    tuple tuple_value;
    for (int i = 0; i < m; i++)//循环次数，有几个6块buffer
    {
        blk_cnt = 0;

        for (; blk_cnt < 6 && now_blk <= end; now_blk++, blk_cnt++)
        {
            if ((blks[blk_cnt] = readBlockFromDisk(now_blk, buffer)) == NULL)
            {
                perror("Reading Block Failed!\n");
                exit(-1);
            }
        }
        for (int j = 0; j < blk_cnt * 7 - 1; j++)
        {
            int tuple_j_index = j / 7;
            int tuple_j_offset = j % 7 + 1;
            for (int k = j + 1; k < blk_cnt * 7; k++)
            {
                int tuple_k_index = k / 7;
                int tuple_k_offset = k % 7 + 1;
                t1 = read_tuple(blks[tuple_j_index], tuple_j_offset);
                t2 = read_tuple(blks[tuple_k_index], tuple_k_offset);
                if (t1.x > t2.x)
                {
                    tuple_value = t2;
                    write_tuple(blks[tuple_j_index], tuple_j_offset,tuple_value);
                    tuple_value = t1;
                    write_tuple(blks[tuple_k_index], tuple_k_offset,tuple_value);
                }
            }
        }
        for (int i = 0; i < blk_cnt; i++)//把拍好序的六个块写入内存
        {
            blk = blks[i];
            tuple_value.x = sstart + 1;
            tuple_value.y = 0;
            write_tuple(blk, 8,tuple_value);
            dropBlockOnDisk(sstart);
            if (writeBlockToDisk(blk, sstart, buffer) != 0)
            {
                perror("Writing Block Failed!\n");
                return;
            }
            sstart++;
        }

        for (int i=0; i < 6 && i <= end; i++)
        {
            freeBlockInBuffer(blks[i],buffer);//每次循环完毕需要释放内存
        }
    }
}

void line(int C)
{
    Buffer buf;
    int i = 0;
    unsigned char *blk;
    unsigned char *wblk;
    int count=0;//记录满足条件的指令的个数
    int blknumber = 49;//磁盘号
    int blocknumber = 0;//块内偏移
    if (!initBuffer(520, 64, &buf))
    {
        perror("Buffer Initialization Failed!\n");
        return;
    }
    printf("-------------------------------\n");
    printf("line start\n");
    printf("-------------------------------\n");
    blk = getNewBlockInBuffer(&buf);
    wblk = getNewBlockInBuffer(&buf);
    for (i = 0; i < 8; i++)
        *(blk + i) = (char)('1' + i);
    for (i=17;i<49;i++)
    {
        if ((blk = readBlockFromDisk(i, &buf)) == NULL)
        {
            perror("Reading Block Failed!\n");
            return;
        }
        int X = -1;
        int Y = -1;
        int addr = -1;
        char str_x[5];
        char str_y[5];
        printf("block number:%d\n",i);
        for (int j = 0; j < 7; j++) //一个blk存7个元组加一个地址
        {

            for (int k = 0; k < 4; k++)
            {
                str_x[k] = *(blk + j*8 + k);
            }
            X = atoi(str_x);
            if(X==C)
            {
                count++;
                for (int k = 0; k < 4; k++)
                {
                    str_y[k] = *(blk + j*8 + 4 + k);
                }
                Y = atoi(str_y);
                printf("(%d,%d)\n",X,Y);//获取本次匹配到的x,y值
                write_buffer(str_x,str_y,wblk,blocknumber);
                blocknumber ++;
                if(blocknumber==7)//此时当前写入块满，缓冲区写满
                {
                    blocknumber = 0;
                    if (writeBlockToDisk(wblk, blknumber, &buf) != 0)
                    {
                        perror("Writing Block Failed!\n");
                        return;
                    }
                    printf("write buffer to %d.blk\n",blknumber);
                    blknumber++;
                }
            }
        }//以上为读取一个块的内容
        freeBlockInBuffer(blk,&buf);
    }
    if (writeBlockToDisk(wblk, blknumber, &buf) != 0)
    {
        perror("Writing Block Failed!\n");
        return;
    }
    printf("write buffer to %d.blk\n",blknumber);
    blknumber++;
    printf("total result:%d\n",count);
    printf("I/O Number:%d\n",buf.numIO);
    printf("---------------------end-----------------");
    freeBuffer(&buf);
    return;
}
//外排序，对于S应该是3路归并
//对于R应该是6路归并排序

//检查当前指针指向的位置，如果内容是指向下一块，则返回下一块的位置
unsigned char *checkpoint(unsigned char *blk,Buffer *buf,int end,int *flag_point, int *flag_blk)
{
    if(*flag_point != 8)//下一个将要读入的不是切换
        return blk;
    tuple tuple_read;
    tuple_read = read_tuple(blk, 8);
    if(tuple_read.x > end || *flag_blk == 5)//是切换，但是此时已经读完
    {
        *flag_point = 0;//置为0
        return blk;
    }
    unsigned char *nextblk;
    printf("blocknumber:%d flag_blk:%d\n",tuple_read.x,*flag_blk);
    freeBlockInBuffer(blk,buf);
    nextblk = readBlockFromDisk(tuple_read.x,buf);
    *flag_blk = *flag_blk + 1;//块号加一
    *flag_point = 1;//当前值指向重新初始化为块头
    return nextblk;
}
//找到最小值，多个指针
tuple findmin(unsigned char **blks, int *flag_point,Buffer *buf, int end, int *flag_blk)
{
    int i;
    tuple tuple_read;
    tuple tuple_min ;
    tuple_min.x = 114514;
    tuple_min.y = 114514;
    int min_point = 114514;
    for (i = 0; i< 6;i++)//循环六个块，找到最小值对应的元组以及最小值对应的块号
    {
        if(flag_point[i]==0)//说明已经读完或者本身不存在未使用
        {
            continue;
        }
        else
        {
            //tuple_read = read_tuple(blks[i],flag_point[0]);//正常读
            //printf("tuple_read:%d\n",tuple_read.x);
            tuple_read = read_tuple(blks[i],flag_point[i]);//正常读
            //printf("read:%d---%d---%d---%d\n",tuple_read.x,tuple_read.y,flag_point[i],i);
            if(tuple_read.x<tuple_min.x)
            {
                tuple_min = tuple_read;
                min_point = i;//记录最小值对应的段号
            }
            else
                continue;
        }
    }
    //printf("read:%d---%d---%d---%d\n",tuple_read.x,tuple_read.y,flag_point[min_point],min_point);
    if(min_point>10000)
        return tuple_min;
    flag_point[min_point] ++;//指针的值加一
    blks[min_point] = checkpoint(blks[min_point],buf,end,&flag_point[min_point],&flag_blk[min_point]);//检查是否已经读到文件尾或者需要切换到下一块中
    /*
    if(*flag_point != 8)//下一个将要读入的不是切换
        return tuple_min;
    tuple tuple_value;
    tuple_value = read_tuple(blks[min_point], 8);
    if(tuple_value.x > end || flag_blk[min_point] == 5)//是切换，但是此时已经读完
    {
        flag_point[min_point] = 0;//置为0
        return tuple_min;
    }
    unsigned char *nextblk;
    nextblk = readBlockFromDisk(tuple_value.x,buf);
    flag_blk++;//块号加一
    *flag_point = 1;//当前值指向重新初始化为块头
    return nextblk;
    return tuple_min;
     */
    return tuple_min;
}

void groupoutsort(int start, int end,Buffer *buf, int blknumber)
{
    int blk_num = blknumber + start - 400 ;         //磁盘块数量
    int m = (end - start + 1) / 6 + 1;//外排序也需要分组
    int now_blk = start;
    int blk_cnt = start;
    unsigned char *wblk;
    unsigned char *blks[6];//一次只用6块buffer中的空间，相当于我一次读6个blk
    unsigned char *point[6];//指针，指向每个块的当前读入位置
    int flag_point[6]={0};//块内标志位
    int flag_blk[6]={0};//块标志位
    tuple tuple_value;
    tuple t1,t2;
    int count=1;//记录已经放入的块数，如果到达7那么重置并且写入内存
    for(int i=0;i<m;i++)
    {
        blks[i] = readBlockFromDisk(blk_cnt, buf);
        blk_cnt = blk_cnt + 6;//外排序准备
        point[i] = blks[i];//指针指向头部
        flag_point[i] = 1;//每个指针有效位
        flag_blk[i] = 0;
    }//初始化，读取例如401，407，413块放入blks中
    wblk = getNewBlockInBuffer(buf);//申请一个用于写的块，还剩7块
    while (1)
    {
        /*
        for(int i=0;i<m;i++)
        {
            t2 = read_tuple(blks[i],8);//正常读
            printf("tuple_read_x:%d\n",t2.x);
            //printf("i:%d||flag_point:%d||flag_blk:%d||min:%d\n",i,flag_point[i],flag_blk[i],t1.x);
        }
         */
        t1 = findmin(blks,flag_point,buf,end,flag_blk);//归并排序找到最小值
        if(t1.x > 10000)//全部读完
        {
            break;
        }
        else
        {
            write_tuple(wblk,count,t1);//向wblk的第count个位置写入，0<count<8
            count = count + 1;
            if(count == 8)//满了就写入磁盘
            {
                tuple_value.x = blk_num + 1;//指向磁盘号
                tuple_value.y = 0;
                write_tuple(wblk, 8,tuple_value);//写入第八个，组成一个完整的blk
                if (writeBlockToDisk(wblk, blk_num, buf) != 0)//写入磁盘
                {
                    perror("Writing Block Failed!\n");
                    exit(-1);
                }
                blk_num ++ ;//磁盘号自增
                count = 1;//重置写入的块内偏移
            }
        }
    }
}


void TPMMS()
{
    Buffer buf;
    int i = 0;
    unsigned char *blk_S;
    unsigned char *blk_R;
    tuple tuple_read;
    int io=0;
    if (!initBuffer(520, 64, &buf))
    {
        perror("Buffer Initialization Failed!\n");
        return;
    }
    printf("-------------------------------\n");
    printf("TPMMS start\n");
    printf("-------------------------------\n");
    groupinsort(1,16,&buf,400);
    groupoutsort(401,416,&buf,300);
    io = io + buf.numIO;
    freeBuffer(&buf);
    if (!initBuffer(520, 64, &buf))
    {
        perror("Buffer Initialization Failed!\n");
        return;
    }
    groupinsort(17,48,&buf,400);//将排序生成的组文件放在400-448.blk中
    groupoutsort(417,448,&buf,300);
    io = io + buf.numIO;
    printf("done!\n");
    printf("I/O Number:%d\n",io);
    printf("---------------------end-----------------");
}

//建立索引
void build_index(int start,int end,int blknumber)
{
    Buffer buffer;
    if (!initBuffer(520, 64, &buffer))
    {
        perror("Buffer Initialization Failed!\n");
        return;
    }
    printf("-------------------------\n");
    printf("index building...\n");
    printf("-------------------------\n");
    int m = (end -start + 1) / 7 + 1;//索引块数
    int number = start;
    unsigned char *blks[8];
    unsigned char *wblk;
    unsigned char *blk;
    tuple tuple_write;
    int store_number[10];
    for(int i=0;i<m;i++)
    {
        wblk = getNewBlockInBuffer(&buffer);
        for (int j=1;j<8&&number<=end;j++)
        {
            //printf("number:%d\n",number);
            blk = readBlockFromDisk(number,&buffer);
            tuple_write = read_tuple(blk,1);
            tuple_write.y = number;
            printf("tuple_x:%d,tuple_y:%d\n",tuple_write.x,tuple_write.y);
            write_tuple(wblk,j,tuple_write);
            freeBlockInBuffer(blk,&buffer);
            number++;
        }
        tuple_write.x = blknumber + 1;
        tuple_write.y = 0;
        write_tuple(wblk,8,tuple_write);
        if (writeBlockToDisk(wblk, blknumber, &buffer) != 0)//写入磁盘
        {
            perror("Writing Block Failed!\n");
            exit(-1);
        }
        blknumber++;
        printf("-------------\n");
    }
    printf("bufferbuild I/O:%d\n",buffer.numIO);
    printf("build finished\n");
    printf("-------------------------\n");
}

int find(int number,Buffer *buf,int C)
{
    unsigned char *blk;
    unsigned char *wblk;
    blk = readBlockFromDisk(number,buf);
    tuple tuple_read;
    wblk = getNewBlockInBuffer(buf);
    for (int i = 0; i < 16; i++)
        *(wblk + i) = 0;
    int count=1;
    for(int i=0;i<=7;i++)
    {
        tuple_read = read_tuple(blk,i);
        //printf("tuple_x:%d\n",tuple_read.x);
        if(C == tuple_read.x)
        {
            printf("get(%d,%d) in block %d\n",tuple_read.x,tuple_read.y,number);
            write_tuple(wblk, count, tuple_read);
            count ++ ;
        }
    }
    if (writeBlockToDisk(wblk, 510, buf) != 0)//写入磁盘
    {
        perror("Writing Block Failed!\n");
        exit(-1);
    }
    return count;
}

//索引检索，输入C，与索引的开始与结束位置
void index_select(int C)
{
    int m = 16/7 + 1;
    int blknumber = 501;
    unsigned char *blk;
    unsigned char *wblk;
    tuple tuple_1;
    tuple tuple_7;
    tuple tuple_read;
    Buffer buffer;
    int yblk;
    int count=0;
    int flag = 1;
    if (!initBuffer(520, 64, &buffer))
    {
        perror("Buffer Initialization Failed!\n");
        return;
    }
    build_index(301,316,501);
    printf("scaning......\n");
    for(int i = 0; i<m;i++)
    {
        blk = readBlockFromDisk(blknumber,&buffer);
        printf("reading index %d...\n",blknumber);
        for(int j=1;j<=7;j++)
        {
            tuple_read = read_tuple(blk,j);
            if(tuple_read.x < C && j!=7)
            {
                tuple_1 = read_tuple(blk,j+1);
                if(tuple_1.x > C)
                {
                    count += find(tuple_read.y,&buffer,C);
                }
                else
                    continue;
            }
            else if(tuple_read.x < C && j == 7)
            {
                count += find(tuple_read.y,&buffer,C);
            }
            else if(tuple_read.x == C)
            {
                count += find(tuple_1.y,&buffer,C);
            }
            else
            {
                break;
            }
        }
        freeBlockInBuffer(blk,&buffer);
        blknumber++;
    }
    printf("---------------end-----------\n");
    printf("total I/O :%d\n",buffer.numIO);
}


int sort_merge_join(int R_sort_start, int R_sort_finish, int S_sort_start, int S_sort_finish, int result_start)
{
    printf("----------Sort-Merge-Join----------\n\n");
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
    tuple value_S;
    tuple value_R;
    tuple tuple_value;

    int wblk_index = 1;
    int j_blk_R_index = 1;

    int count = 0;
    int remember_value_R;
    int find = 0;
    //用于写的内存块
    wblk = getNewBlockInBuffer(&buf);

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
                            write_tuple(wblk, wblk_index,tuple_value);
                            wblk_index++;
                            if(wblk_index == 8)
                            {
                                printf("blk_number:%d\n", result_finish);
                                wblk_index = 1;
                                tuple_value.x = result_finish + 1;
                                tuple_value.y = 0;
                                write_tuple(wblk, 8,tuple_value);
                                if (writeBlockToDisk(wblk, result_finish, &buf) != 0)
                                {
                                    perror("Writing Block Failed!\n");
                                    return -1;
                                }
                                wblk = getNewBlockInBuffer(&buf);
                                result_finish++;
                            }
                            tuple_value.x = value_R.x;
                            tuple_value.y = value_R.y;
                            write_tuple(wblk, wblk_index,tuple_value);
                            wblk_index++;
                            if(wblk_index == 8)
                            {
                                printf("blk_number:%d\n", result_finish);
                                wblk_index = 1;
                                tuple_value.x = result_finish + 1;
                                tuple_value.y = 0;
                                write_tuple(wblk, 8,tuple_value);
                                if (writeBlockToDisk(wblk, result_finish, &buf) != 0)
                                {
                                    perror("Writing Block Failed!\n");
                                    return -1;
                                }
                                wblk = getNewBlockInBuffer(&buf);
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
        printf("blk_number :%d\n", result_finish);
        tuple_value.x = result_finish + 1;
        tuple_value.y = 0;
        write_tuple(wblk, 8,tuple_value);
        if (writeBlockToDisk(wblk, result_finish, &buf) != 0)
        {
            perror("Writing Block Failed!\n");
            return -1;
        }
    }
    printf("connect_times:%d times\n", count);
    freeBuffer(&buf);
}

int t()
{
    //line(130);
    //TPMMS();
    //index_select(130);
    sort_merge_join(301,316,317,348,600);
}
