
#include "logger.h"

uint16_t log_levels = LOG_FLAG_INFO | LOG_FLAG_WARN | LOG_FLAG_ERROR | LOG_FLAG_FATAL;

void log_header(const char* type) {

	time_t rawtime = time(0);

	struct tm ts;
	char buffer[255];

	ts = *localtime(&rawtime);
	strftime(buffer, sizeof(buffer), "%H:%M:%S", &ts);

	printf("[%s] %s: ", buffer, type);

}

void log_setlevel(uint16_t bitset) {
	log_levels = bitset;
}

bool log_chklevel(uint16_t flag) {
	return log_levels & flag;
}
