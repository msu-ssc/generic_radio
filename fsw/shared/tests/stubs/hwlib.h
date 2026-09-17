#ifndef IRIS_TEST_HWLIB_H
#define IRIS_TEST_HWLIB_H
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#define OS_SUCCESS 0
#define OS_ERROR (-1)
#define CFE_MAKE_BIG32(x) (x)
#define CFE_MAKE_BIG16(x) (x)
#define OS_printf printf
typedef struct { int sockfd; } socket_info_t;
int32_t socket_send(socket_info_t *, uint8_t *, size_t, size_t *, char *, int);
int32_t socket_recv(socket_info_t *, uint8_t *, size_t, size_t *);
int32_t OS_TaskDelay(uint32_t);
#endif
