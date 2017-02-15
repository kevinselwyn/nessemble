#ifndef _DOWNLOAD_H
#define _DOWNLOAD_H

#define BUFFER_SIZE 512 * 1024

struct write_result {
    char *data;
    size_t pos;
};

#endif /* _DOWNLOAD_H */
