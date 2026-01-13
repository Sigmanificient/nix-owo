#pragma once

/* For closing many file descriptors after forking. */
#define HAVE_CLOSE_RANGE 1

/* Optionally used for changing the files and symlinks. */
#define HAVE_DECL_AT_SYMLINK_NOFOLLOW 1

/* Optionally used for changing the mtime of symlinks. */
#define HAVE_LUTIMES 1

/* Optionally used for creating pipes on Unix. */
#define HAVE_PIPE2 1

/* Optionally used to get more information about processes failing due to a signal on Unix. */
#define HAVE_STRSIGNAL 1

/* Optionally used to try to close more file descriptors (e.g. before forking) on Unix. */
#define HAVE_SYSCONF 1

/* Optionally used for changing the mtime of files and symlinks. */
#define HAVE_UTIMENSAT 1

