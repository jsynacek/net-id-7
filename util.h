#include <assert.h>
#include <errno.h>
#include <libudev.h>
#include <stdbool.h>
#include <string.h>

#define DIGITS            "0123456789"
#define UPPERCASE_LETTERS "ABCDEFGHIJKLMNOPQRSTUVWXYZ"

#define new(t, n) ((t*)malloc(sizeof(t)*(n)))
#define xsprintf sprintf
#define streq(a,b) (strcmp((a),(b)) == 0)
static inline bool streq_ptr(const char *a, const char *b) {
        /* Like streq(), but tries to make sense of NULL pointers */
        if (a && b)
                return streq(a, b);
        if (!a && !b)
                return true;
        return false;
}

static inline size_t strpcpy(char **dest, size_t size, const char *src) {
        size_t len;
        len = strlen(src);
        if (len >= size) {
                if (size > 1)
                        *dest = mempcpy(*dest, src, size-1);
                size = 0;
        } else {
                if (len > 0) {
                        *dest = mempcpy(*dest, src, len);
                        size -= len;
                }
        }
        *dest[0] = '\0';
        return size;
}

static inline size_t strpcpyf(char **dest, size_t size, const char *src, ...) {
        va_list va;
        int i;
        va_start(va, src);
        i = vsnprintf(*dest, size, src, va);
        if (i < (int)size) {
                *dest += i;
                size -= i;
        } else {
                *dest += size;
                size = 0;
        }
        va_end(va);
        *dest[0] = '\0';
        return size;
}

static inline size_t strpcpyl(char **dest, size_t size, const char *src, ...) {
        va_list va;
        va_start(va, src);
        do {
                size = strpcpy(dest, size, src);
                src = va_arg(va, char *);
        } while (src != NULL);
        va_end(va);
        return size;
}

static inline size_t strscpy(char *dest, size_t size, const char *src) {
        char *s;
        s = dest;
        return strpcpy(&s, size, src);
}

static inline char *strjoin(const char *x, ...) {
        va_list ap;
        size_t l;
        char *r, *p;
        va_start(ap, x);
        if (x) {
                l = strlen(x);
                for (;;) {
                        const char *t;
                        size_t n;
                        t = va_arg(ap, const char *);
                        if (!t)
                                break;
                        n = strlen(t);
                        if (n > ((size_t) -1) - l) {
                                va_end(ap);
                                return NULL;
                        }
                        l += n;
                }
        } else
                l = 0;
        va_end(ap);
        r = new(char, l+1);
        if (!r)
                return NULL;
        if (x) {
                p = stpcpy(r, x);

                va_start(ap, x);

                for (;;) {
                        const char *t;
                        t = va_arg(ap, const char *);
                        if (!t)
                                break;
                        p = stpcpy(p, t);
                }
                va_end(ap);
        } else
                r[0] = 0;
        return r;
}
#define strjoina(x, ...) strjoin((x), __VA_ARGS__, NULL)
#define strneq(a, b, n) (strncmp((a), (b), (n)) == 0)

static inline char *startswith(const char *s, const char *prefix) {
        size_t l;
        l = strlen(prefix);
        if (strncmp(s, prefix, l) == 0)
                return (char*) s + l;
        return NULL;
}

static inline bool in_charset(const char *s, const char* charset) {
        assert(s);
        assert(charset);
        return s[strspn(s, charset)] == '\0';
}

static inline char *ascii_strlower(char *t) {
        char *p;
        assert(t);
        for (p = t; *p; p++)
                if (*p >= 'A' && *p <= 'Z')
                        *p = *p - 'A' + 'a';
        return t;
}

static inline int safe_atoi(const char *s, int *ret_i) {
        char *x = NULL;
        long l;
        assert(s);
        assert(ret_i);
        errno = 0;
        l = strtol(s, &x, 0);
        if (!x || x == s || *x || errno)
                return errno > 0 ? -errno : -EINVAL;
        if ((long) (int) l != l)
                return -ERANGE;
        *ret_i = (int) l;
        return 0;
}

#define LONG_LINE_MAX (1U*1024U*1024U)
static inline int read_one_line_file(const char *fn, char **line) {
        FILE *f = NULL;
        assert(fn);
        assert(line);
        f = fopen(fn, "re");
        if (!f)
                return -errno;
	*line = new(char, LONG_LINE_MAX);
        fread(*line, 1, LONG_LINE_MAX-1, f);
        fclose(f);
        return 0;
}

static inline int udev_builtin_add_property(struct udev_device *dev, bool test, const char *key, const char *val) {
	printf("%s=%s\n", key, val);
}
