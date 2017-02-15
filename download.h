#ifndef _DOWNLOAD_H
#define _DOWNLOAD_H

#define BUFFER_SIZE 512 * 1024

struct write_result {
    char *data;
    int pos;
};

#endif /* _DOWNLOAD_H */
