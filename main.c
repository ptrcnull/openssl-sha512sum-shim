#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <openssl/evp.h>

const char * usage = "usage: sha512sum [options] [file...]\n"
    "print or check SHA512 checksums (with OpenSSL).\n\n"
    "with no file, reads standard input.\n"
    "  -c, --check\t\tread checksums from file and verify them\n"
    "  -h, --help\t\tprint this message\n";

static int BUF_SIZE = 8192;
static int OUT_BUF_SIZE = 128+1;

// takes in null-separated filename, writes 128 hex-encoded characters to output
int do_checksum(char * input_filename, char * output) {
    char * buf[BUF_SIZE];
    int input_fd;
    EVP_MD_CTX * mdctx;

    if (!strcmp(input_filename, "-")) {
        input_fd = 0;
    } else {
        input_fd = open(input_filename, O_RDONLY);
    }
    if (input_fd < 0) {
        perror(input_filename);
        return 1;
    }

    if ((mdctx = EVP_MD_CTX_new()) == NULL) {
        fprintf(stderr, "could not create evp context\n");
        return 1;
    }

    if (!EVP_DigestInit_ex(mdctx, EVP_sha512(), NULL)) {
        fprintf(stderr, "could not initialize evp context\n");
        return 1;
    }

    int read_bytes;
    while ((read_bytes = read(input_fd, buf, BUF_SIZE)) > 0) {
        if (!EVP_DigestUpdate(mdctx, buf, read_bytes)) {
            fprintf(stderr, "could not update digest\n");
            return 1;
        }
    }
    if (read_bytes < 0) {
        perror(input_filename);
        exit(EXIT_FAILURE);
    }

    unsigned char * digest = (unsigned char *)OPENSSL_malloc(EVP_MD_size(EVP_sha512()));
    if (digest == NULL) {
        fprintf(stderr, "could not malloc digest output\n");
        return 1;
    }
    unsigned int digest_len;

	if (!EVP_DigestFinal_ex(mdctx, digest, &digest_len)) {
        fprintf(stderr, "could not digest\n");
        return 1;
    }
    for (int i = 0; i < (int)digest_len; i++) {
        sprintf(output, "%02x", digest[i]);
        output += 2;
    }
    output[128] = 0;

	EVP_MD_CTX_free(mdctx);

    return 0;
}

int perform_check(char * sums_filename) {
    FILE * sums_file;

    if (!strcmp(sums_filename, "-")) {
        sums_file = stdin;
        sums_filename = "stdin";
    } else {
        sums_file = fopen(sums_filename, "r");
    }

    if (sums_file == NULL) {
        perror(sums_filename);
        exit(EXIT_FAILURE);
    }

    int total = 0;
    int failed = 0;

    char * line = NULL;
    ssize_t read;
    size_t len = 0;

    while ((read = getline(&line, &len, sums_file)) != -1) {
        char * sep = strstr(line, "  ");
        if (sep == NULL) {
            fprintf(stderr, "invalid checksum file syntax: %s\n", sums_filename);
            return 1;
        }

        *sep = 0;
        char * filename = sep + 2;

        char * newline = strstr(filename, "\n");
        if (newline != NULL) {
            *newline = 0;
        }

        char output[OUT_BUF_SIZE];
        if (do_checksum(filename, output)) {
            exit(EXIT_FAILURE);
        }
        total++;
        if (!strcmp(output, line)) {
            printf("%s: OK\n", filename);
        } else {
            printf("%s: FAILED\n", filename);
            failed++;
        }
    }

    printf("sha512sum: WARNING: %d of %d computed checksums did NOT match\n", failed, total);

    fclose(sums_file);
    return failed > 0;
}

int main(int argc, char * argv[]) {
    bool check = false;
    int c;

    static struct option long_options[] = {
        {"check",  no_argument,  0,  'c' },
        {"help",   no_argument,  0,  'h' },
        {0,        0,            0,  0   }
    };

    while (1) {
        int option_index = 0;
        c = getopt_long(argc, argv, "ch",
            long_options, &option_index);
        if (c == -1) {
            break;
        }

        if (c == 0) {
            c = long_options[option_index].val;
        }

        switch (c) {
            case 'c':
                check = true;
                break;
            case 'h':
                fprintf(stderr, "%s", usage);
                exit(EXIT_SUCCESS);
            default:
                exit(EXIT_FAILURE);
        }
    }

    int exit_code = EXIT_SUCCESS;

    if (optind < argc) {
        // files were supplied
        while (optind < argc) {
            char * arg = argv[optind++];
            if (check) {
                exit_code = perform_check(arg);
            } else {
                char output[OUT_BUF_SIZE];
                if (do_checksum(arg, output)) {
                    exit_code = EXIT_FAILURE;
                } else {
                    printf("%s  %s\n", output, arg);
                }
            }
        }
    } else {
        // use stdin
        if (check) {
            exit_code = perform_check("-");
        } else {
            char output[OUT_BUF_SIZE];
            if (do_checksum("-", output)) {
                exit_code = EXIT_FAILURE;
            } else {
                printf("%s  -\n", output);
            }
        }
    }

    exit(exit_code);
}
