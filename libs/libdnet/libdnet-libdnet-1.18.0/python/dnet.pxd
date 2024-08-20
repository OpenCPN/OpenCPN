cdef extern from "bsd/string.h":
    size_t     strlcpy(char *dst, char *src, size_t size)
