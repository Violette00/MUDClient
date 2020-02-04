void logmsg(const char *file, int line, const char *level, char nl,
		const char *format, ...);

#define LOG_ERR(...) \
	logmsg(__FILE__, __LINE__, "ERR", 1, __VA_ARGS__)

#define LOG_INFO_NONL(...) \
	logmsg(__FILE__, __LINE__, "INFO", 0, __VA_ARGS__)
