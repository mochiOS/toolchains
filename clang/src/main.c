#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if defined(x86_64)
#define DRIVER_NAME "x86_64-mochios-clang"
#define CLANG_TARGET "x86_64-elf"
#elif defined(aarch64)
#define DRIVER_NAME "aarch64-mochios-clang"
#define CLANG_TARGET "aarch64-elf"
#endif

static const char *find_sdk(const char *sdk) {
	const char *environment_sdk;

	if (sdk != NULL) return sdk;

	environment_sdk = getenv("MOCHIOS_SDK");
	if (environment_sdk != NULL && environment_sdk[0] != '\0')
		return environment_sdk;

	return NULL;
}

static int compile_only(int argc, char **argv) {
	int i;

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "-S") == 0 ||
		    strcmp(argv[i], "-E") == 0)
			return 1;
	}

	return 0;
}

static void help(void) {
	fprintf(stderr, "usage: " DRIVER_NAME
			" [--mochios-sdk <path>] <clang arguments...>\n");
}

int main(int argc, char **argv) {
	const char *clang;
	const char *explicit_sdk;
	const char *sdk;
	char target[64];
	char sysroot[PATH_MAX];
	char **clang_argv;
	int input_index;
	int output_index;

	clang = getenv("CLANG");
	if (clang == NULL || clang[0] == '\0') clang = "clang";

	explicit_sdk = NULL;

	if (argc < 2) {
		help();
		return 1;
	}

	if (!compile_only(argc, argv)) {
		fprintf(stderr, DRIVER_NAME ": linking is not implemented yet; "
					    "use -c, -S, or -E\n");
		return 1;
	}

	for (input_index = 1; input_index < argc; input_index++) {
		if (strcmp(argv[input_index], "--mochios-sdk") != 0) continue;

		if (input_index + 1 >= argc) {
			fprintf(stderr, DRIVER_NAME
				": --mochios-sdk requires a path\n");
			return 1;
		}

		explicit_sdk = argv[++input_index];
	}

	sdk = find_sdk(explicit_sdk);
	if (sdk == NULL) {
		fprintf(stderr, DRIVER_NAME
			": mochiOS SDK not found\n"
			"set MOCHIOS_SDK or pass --mochios-sdk <path>\n");
		return 1;
	}

	if (snprintf(target, sizeof(target), "--target=%s", CLANG_TARGET) >=
	    (int)sizeof(target)) {
		fprintf(stderr, DRIVER_NAME ": target is too long\n");
		return 1;
	}

	if (snprintf(sysroot, sizeof(sysroot), "--sysroot=%s/sysroot", sdk) >=
	    (int)sizeof(sysroot)) {
		fprintf(stderr, DRIVER_NAME ": SDK path is too long\n");
		return 1;
	}

	clang_argv = calloc((size_t)argc + 16, sizeof(*clang_argv));
	if (clang_argv == NULL) {
		fprintf(stderr, DRIVER_NAME ": out of memory\n");
		return 1;
	}

	output_index = 0;

	clang_argv[output_index++] = (char *)clang;
	clang_argv[output_index++] = target;
	clang_argv[output_index++] = sysroot;
	clang_argv[output_index++] = "-D__mochios__=1";
	clang_argv[output_index++] = "-ffreestanding";
	clang_argv[output_index++] = "-mno-red-zone";
	clang_argv[output_index++] = "-fno-pie";
	clang_argv[output_index++] = "-fno-stack-protector";

	for (input_index = 1; input_index < argc; input_index++) {
		if (strcmp(argv[input_index], "--mochios-sdk") == 0) {
			input_index++;
			continue;
		}

		clang_argv[output_index++] = argv[input_index];
	}

	clang_argv[output_index] = NULL;

	execvp(clang, clang_argv);

	fprintf(stderr, DRIVER_NAME ": failed to execute %s: %s\n", clang,
		strerror(errno));

	free(clang_argv);

	return 127;
}
