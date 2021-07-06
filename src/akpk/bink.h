#ifndef BINK_H
#define BINK_H

#include <inttypes.h>

struct bkhd_header {
    uint32_t _bkhd; // 0x44484b42
    uint32_t size;
    uint32_t revision; // ?
    uint32_t bank_id;
    uint32_t soundbank_id;
    uint32_t _reserved;
    uint32_t version;
};

struct didx {
    uint32_t _didx; // 0x58444944
    uint32_t size;
};

struct didx_entry {
    uint32_t wem_id;
    uint32_t data_offset;
    uint32_t size;
};

struct didx_data {
    uint32_t _data;
    uint32_t size;
};

struct wem {
    uint32_t _riff; // 0x46464952

};

#endif // BINK_H
