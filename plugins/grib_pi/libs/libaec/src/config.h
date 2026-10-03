/* Static replacement for libaec's generated config.h, decoder only.
 * Without HAVE_DECL___BUILTIN_CLZLL/HAVE_BSR64 decode.c uses __has_builtin
 * or a portable bit scan loop. */
