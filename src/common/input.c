
#include "input.h"

#include "common/logger.h"

static char input_norm(char chr) {
	if (chr == '\n') return ' ';
	if (chr == '\t') return ' ';

	return chr;
}

static bool input_lexeme(InputLine* line, char* buffer, uint32_t limit, bool string) {

	char* from = line->line + line->offset;
	uint32_t src = 0;
	uint32_t dst = 0;
	char terminator = string ? '"' : ' ';

	// skip leading whitespaces
	while (src < line->length && from[src] == ' ') {
		src ++;
	}

	// check if string begins with quote
	if (string && from[src ++] != '"') {
		printf("Syntax error, expected string!\n");
		return false;
	}

	while (src < line->length && input_norm(from[src]) != terminator && dst < limit - 1) {
		buffer[dst ++] = from[src ++];
	}

	if (string && from[src ++] != '"') {
		printf("Syntax error, unexpected end of input!\n");
		return false;
	}

	line->offset += src;
	buffer[dst] = 0;
	return true;

}

static bool input_strtol(long* num, char* buffer, int base) {

	long tmp = 0;

	if (strlen(buffer) > 2) {
		if (buffer[0] == '0' && (buffer[1] == 'x' || buffer[1] == 'X')) {
			printf("Syntax error, invalid number prefix!\n");
			return false;
		}
	}

	char* last;

	long value = strtol(buffer, &last, base);

	if (*last != 0) {
		printf("Syntax error, invalid number format!\n");
		return false;
	}

	*num = value;
	return true;

}

void input_readline(InputLine* line) {
	getline(&line->line, &line->length, stdin);
	line->offset = 0;
}

void input_free(InputLine* line) {
	free(line->line);
}

bool input_token(InputLine* line, char* buffer, uint32_t limit) {
	return input_lexeme(line, buffer, limit, false);
}

bool input_string(InputLine* line, char* buffer, uint32_t limit) {
	return input_lexeme(line, buffer, limit, true);
}

bool input_number(InputLine* line, long* num) {

	char buffer[128];

	if (!input_token(line, buffer, 128)) {
		return false;
	}

	if (!input_parse(num, buffer)) {
		return false;
	}

	return true;

}

bool input_parse(long* num, char* buffer) {

	if (strlen(buffer) > 2) {
		if (buffer[0] == '0' && buffer[1] == 'b') return input_strtol(num, buffer + 2, 2);
		if (buffer[0] == '0' && buffer[1] == 'o') return input_strtol(num, buffer + 2, 8);
		if (buffer[0] == '0' && buffer[1] == 'x') return input_strtol(num, buffer + 2, 16);
	}

	return input_strtol(num, buffer, 10);

}
