
#include "hexdump.hpp"

void hexdump(Stream &s, const void *ptr, size_t buflen) {
    unsigned char *buf = (unsigned char*)ptr;
    int i, j;
    for (i=0; i<buflen; i+=16) {
        s.printf("%06x: ", i);
        for (j=0; j<16; j++)
            if (i+j < buflen)
                s.printf("%02x ", buf[i+j]);
            else
                s.printf("   ");
        s.printf(" ");
        for (j=0; j<16; j++)
            if (i+j < buflen)
                s.printf("%c", isprint(buf[i+j]) ? buf[i+j] : '.');
        s.printf("\n");
    }
}
