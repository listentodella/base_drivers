#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* in是write操作的下标
 * out是read操作的下标
 * in与out都是uint32_t的，但是可能存在两者增长速度不匹配的问题，尤其是溢出的时候
 * 当 in先溢出时,无符号是从0重新计算的，那么in<out，in-out就是负数（这又是一次溢出）
 * 然后作为无符号计算时，这些都不存在，可以参考main里的实验，依旧是合理长度
 *
 *
 * in - out 为缓冲区中的数据长度
 * in == out 时缓冲区为空, 也可以理解为数据长度为0
 * size - in + out 为缓冲区中的空闲空间长度
 * size == (in - out) 时缓冲区满了，也可以理解为空闲长度为0了
 *
 *  in & (size - 1) == in % size, 但是前者运算速度比后者快非常多
+--------------------------------------------------------------+
|            |<----------len ---------->|                      |
+--------------------------------------------------------------+
             ^                          ^                      ^
             |                          |                      |
            out                        in                     size
*/

typedef struct ringbuffer {
    uint8_t *buffer;
    uint32_t in;//write idx
    uint32_t out;//read idx
    uint32_t size;
} rb_t;

uint32_t min(uint32_t a, uint32_t b)
{
    return a < b ? a:b;
}


uint32_t rb_write(rb_t *rb, uint8_t *buf, uint32_t len)
{
    uint32_t l;
    /* 运算后, len不会超过空闲长度,
     * 该长度只会比你想写的短,不会比期望的长
     */
    len = min(len, rb->size - rb->in + rb->out);

    /* 因为 0%size <= in%size <= (size-1)%size
     * 所以 1 <= size - in%size <= size
     * 当 in = size-1 时, 取最小值1; 当in=0时,取最大值size
     * 两种情况分别对应buffer的尾端和头端
     * 那么 buffer+in%size, 就是当前buffer的in下标(取模后永远不可能越界)
     * 而len经过上面的运算后,是可写的空闲长度, 但是in下标是未知的, 强行写buffer可能会越界
     * 因此再次经过 min后, size - in下标,得到距离尾端还剩多少长度tmp
     * 因此l将会是len与该剩余长度tmp的较小值, 并使用copy先填充这一部分的数据
     * 紧接着再将剩余要写的len-l长度的数据，往剩余可写的buffer里copy
     * 无需担心, 如果剩余长度len-l==0, 那么实际也什么都没copy
     * 
     * */
    l = min(len, rb->size - (rb->in & (rb->size - 1)));
    memcpy(rb->buffer + (rb->in & (rb->size - 1)), buf, l);

    memcpy(rb->buffer, buf + l, len - l);

    /*
     * 数据copy完成之后, 直接将已copy的长度len加到in下标上
     * 这个数字也许会比size大, 但实际上述取下标时都是取模的
     * 至于uint32_t如果溢出了, 又会被置为0重新开始,因此省略了一次取模运算
     * */

    rb->in += len;

    return len;
}


uint32_t rb_read(rb_t *rb, uint8_t *buf, uint32_t len)
{
    uint32_t l;
    /* 运算后, len不会超过有效数据长度,
     * 该长度只会比你想读的短,不会比期望的长
     */
    len = min(len, rb->in - rb->out);

    /*
     * out%size, 实际就是out下标,
     * size-out就是最多能读的长度tmp，
     * 而min之后，就是第一次可读长度l
     * buffer+out，从此处读取l长度
     * 而l<=len，如果有未读的有效数据,要接着从buffer头开读剩余len-l长度
     *
     * 假设实际可读长度len, 是小于size-out的，那么经过min之后，最多也就读len
     * 假设实际可读长度len，是大于size-out的，那么经过min之后，先读size-out长度，然后再补读len-l
     * 因此下面的代码可以hold住这两种情况
     *
     * */
    l = min(len, rb->size - (rb->out & (rb->size - 1)));
    memcpy(buf, rb->buffer + (rb->out & (rb->size - 1)), l);

    memcpy(buf + l, rb->buffer, len - l);

    /*
     * 数据读取完成之后, 直接将已读长度len加到out下标上
     * 这个数字也许会比size大, 但实际上述取下标时都是取模的
     * 至于uint32_t如果溢出了, 又会被置为0重新开始,因此省略了一次取模运算
     */
    rb->out += len;

    return len;
}

uint32_t rb_get_valid_length(rb_t *rb)
{
    //return rb->size - (rb->in & (rb->size - 1));
    return rb->in - rb->out;
}


int main()
{
    rb_t mybuffer = {
        .buffer = NULL,
        .in = 0,
        .out = 0,
        .size = 8
    };
    mybuffer.buffer = (uint8_t *)malloc(sizeof(uint8_t) * mybuffer.size);
    if (!mybuffer.buffer) {
        printf("failed to malloc buffer\n");
        return -1;
    }

    //unsigned int max  = 4294967295;
    uint32_t in =  4294967294;
    uint32_t out = 4294967279;
    printf("in - out = %u, %d\n", in - out, in-out);
    in += 12;
    printf("after plus 12, in = %u, %d\n", in, in);
    printf("in - out = %u, %d\n", in - out, in-out);


    while(1) {
        char c = getchar();
        switch (c) {
        case 'Q':
            break;
        case 'R':
            char data[8] = {0};
            printf("valid length = %d: ", mybuffer.in - mybuffer.out);
            rb_read(&mybuffer, data, mybuffer.in - mybuffer.out);
            printf("read get %s\n", data);
            break;
        default:
            if (c != '\n')
                rb_write(&mybuffer, &c, sizeof(c));
            break;
        }
        if (c == 'Q')
            break;
    }

    free(mybuffer.buffer);
    mybuffer.buffer = NULL;

    return 0;
}
