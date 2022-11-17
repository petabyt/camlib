#include <string.h>
#include "enum.h"

int ptp_enum_index(char *string, int *value, int i) {
	if (i >= ptp_enums_length) {
		return 1;
	}

	strcpy(string, ptp_enums[i].name);
	value[0] = ptp_enums[i].value;
	return 0;
}

int ptp_enum_all(char *string) {
	for (int i = 0; i < ptp_enums_length; i++) {
		if (!strcmp(string, ptp_enums[i].name)) {
			return ptp_enums[i].value;
		}
	}

	return -1;
}

int ptp_enum(int type, char *string) {
	for (int i = 0; i < ptp_enums_length; i++) {
		if (!strcmp(string, ptp_enums[i].name) && ptp_enums[i].type == type) {
			return ptp_enums[i].value;
		}
	}

	return -1;
}

char *ptp_get_enum(int id) {
	for (int i = 0; i < ptp_enums_length; i++) {
		if (id == ptp_enums[i].value) {
			return ptp_enums[i].name;
		}
	}

	return NULL;
}
