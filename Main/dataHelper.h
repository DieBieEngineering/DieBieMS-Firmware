#include <stdint.h>

typedef union 
{
    uint16_t Val;
    uint8_t v[2];
    struct
    {
        uint8_t LB;
        uint8_t HB;
    } byte;
} UINT16_VAL;

typedef union
{
    uint32_t Val;
    uint16_t w[2];
    uint8_t  v[4];
    struct
    {
        uint16_t LW;
        uint16_t HW;
    } word;
    struct
    {
        uint8_t LB;
        uint8_t HB;
        uint8_t UB;
        uint8_t MB;
    } byte;
    struct
    {
        UINT16_VAL low;
        UINT16_VAL high;
    }wordUnion;
} UINT32_VAL;

typedef union
{
    uint64_t Val;
    uint32_t d[2];
    uint16_t w[4];
    uint8_t v[8];
    struct
    {
        uint32_t LD;
        uint32_t HD;
    } dword;
    struct
    {
        uint16_t LW;
        uint16_t HW;
        uint16_t UW;
        uint16_t MW;
    } word;
} UINT64_VAL;
