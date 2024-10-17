#ifndef STUB_PRIV_H
#define STUB_PRIV_H

#define DJSTUB_VERSION 5

struct ldops {
    void *(*read_headers)(int ifile);
    uint32_t (*get_va)(void *handle);
    uint32_t (*get_length)(void *handle);
    uint32_t (*get_entry)(void *handle);
    void (*read_sections)(void *handle, char *ptr, int ifile,
            uint32_t offset);
    void (*close)(void *handle);
};

#endif
