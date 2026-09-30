#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if defined(x86_64)
#define DRIVER_NAME "x86_64-mochios-ld"
#elif defined(aarch64)
#define DRIVER_NAME "aarch64-mochios-ld"
#else
#error "unsupported architecture"
#endif

static const char *find_sdk(const char *explicit_sdk) {
	const char *environment_sdk;

	if (explicit_sdk != NULL) return explicit_sdk;

	environment_sdk = getenv("MOCHIOS_SDK");
	if (environment_sdk != NULL && environment_sdk[0] != '\0')
		return environment_sdk;

	return NULL;
}

static void usage(void) {
	fprintf(stderr, "usage: " DRIVER_NAME
			" [--mochios-sdk <path>] <objects...> -o <output>\n");
}

static int append(char **argv, int *count, int capacity, char *value) {
	if (*count >= capacity - 1) return -1;

	argv[(*count)++] = value;
	return 0;
}

int main(int argc, char **argv) {
	const char *sdk;
	const char *explicit_sdk;
	const char *output;

	char linker_script[PATH_MAX];
	char crt0[PATH_MAX];
	char runtime[PATH_MAX];
	char libgcc[PATH_MAX];
	char sysroot[PATH_MAX];
	char sysroot_lib[PATH_MAX];

	char **lld_argv;

	int lld_argc;
	int capacity;
	int i;

	explicit_sdk = NULL;
	output = NULL;
	lld_argc = 0;

	if (argc < 2) {
		usage();
		return 1;
	}

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--mochios-sdk") == 0) {
			if (i + 1 >= argc) {
				fprintf(stderr, DRIVER_NAME
					": --mochios-sdk requires a path\n");
				return 1;
			}

			explicit_sdk = argv[++i];
			continue;
		}

		if (strcmp(argv[i], "-o") == 0) {
			if (i + 1 >= argc) {
				fprintf(stderr,
					DRIVER_NAME ": -o requires a path\n");
				return 1;
			}

			output = argv[++i];
		}
	}

	if (output == NULL) {
		fprintf(stderr, DRIVER_NAME ": output path is required\n");
		return 1;
	}

	sdk = find_sdk(explicit_sdk);
	if (sdk == NULL) {
		fprintf(stderr, DRIVER_NAME
			": mochiOS SDK not found\n"
			"set MOCHIOS_SDK or pass --mochios-sdk <path>\n");
		return 1;
	}

	if (snprintf(linker_script, sizeof(linker_script), "%s/lib/linker.ld",
		     sdk) >= (int)sizeof(linker_script)) {
		fprintf(stderr, DRIVER_NAME ": SDK path is too long\n");
		return 1;
	}

	if (snprintf(crt0, sizeof(crt0), "%s/lib/crt0.o", sdk) >=
	    (int)sizeof(crt0)) {
		fprintf(stderr, DRIVER_NAME ": SDK path is too long\n");
		return 1;
	}

	if (snprintf(runtime, sizeof(runtime),
		     "%s/lib/libmochi_user_newlib_runtime.a",
		     sdk) >= (int)sizeof(runtime)) {
		fprintf(stderr, DRIVER_NAME ": SDK path is too long\n");
		return 1;
	}

	if (snprintf(libgcc, sizeof(libgcc), "%s/lib/libgcc.a", sdk) >=
	    (int)sizeof(libgcc)) {
		fprintf(stderr, DRIVER_NAME ": SDK path is too long\n");
		return 1;
	}

	if (snprintf(sysroot, sizeof(sysroot), "--sysroot=%s/sysroot", sdk) >=
	    (int)sizeof(sysroot)) {
		fprintf(stderr, DRIVER_NAME ": SDK path is too long\n");
		return 1;
	}

	if (snprintf(sysroot_lib, sizeof(sysroot_lib), "-L%s/sysroot/lib",
		     sdk) >= (int)sizeof(sysroot_lib)) {
		fprintf(stderr, DRIVER_NAME ": SDK path is too long\n");
		return 1;
	}

	capacity = argc + 32;

	lld_argv = calloc((size_t)capacity, sizeof(*lld_argv));
	if (lld_argv == NULL) {
		fprintf(stderr, DRIVER_NAME ": out of memory\n");
		return 1;
	}

	append(lld_argv, &lld_argc, capacity, "ld.lld");
	append(lld_argv, &lld_argc, capacity, sysroot);
	append(lld_argv, &lld_argc, capacity, sysroot_lib);
	append(lld_argv, &lld_argc, capacity, "--static");
	append(lld_argv, &lld_argc, capacity, "--nostdlib");
	append(lld_argv, &lld_argc, capacity, "--no-pie");
	append(lld_argv, &lld_argc, capacity, "-z");
	append(lld_argv, &lld_argc, capacity, "noexecstack");

	append(lld_argv, &lld_argc, capacity, "-T");
	append(lld_argv, &lld_argc, capacity, linker_script);

	append(lld_argv, &lld_argc, capacity, "--start-group");
	append(lld_argv, &lld_argc, capacity, crt0);

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--mochios-sdk") == 0) {
			i++;
			continue;
		}

		if (strcmp(argv[i], "-o") == 0) {
			i++;
			continue;
		}

		append(lld_argv, &lld_argc, capacity, argv[i]);
	}

	append(lld_argv, &lld_argc, capacity, runtime);
	append(lld_argv, &lld_argc, capacity, "-lc");
	append(lld_argv, &lld_argc, capacity, "-lm");
	append(lld_argv, &lld_argc, capacity, libgcc);
	append(lld_argv, &lld_argc, capacity, "--end-group");

	append(lld_argv, &lld_argc, capacity, "-o");
	append(lld_argv, &lld_argc, capacity, (char *)output);

	lld_argv[lld_argc] = NULL;

	execvp("ld.lld", lld_argv);

	fprintf(stderr, DRIVER_NAME ": failed to execute ld.lld: %s\n",
		strerror(errno));

	free(lld_argv);

	return 127;
}
