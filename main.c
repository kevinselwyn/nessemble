#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include "nessemble.h"

#define _GNU_SOURCE
#include <stdlib.h>
int fileno(FILE *file);
char *realpath(const char *path, char *resolved_path);

/**
 * Main function
 * @param {int} argc - Argument count
 * @param {char *} argv[] - Argument array
 */
int main(int argc, char *argv[]) {
    int rc = RETURN_OK;
    unsigned int i = 0, l = 0, byte = 0, piped = FALSE;
    char *exec = NULL, *filename = NULL, *outfilename = NULL, *listname = NULL, *recipe = NULL, *registry = NULL;
    FILE *file = NULL, *outfile = NULL;

    // exec
    exec = argv[0];

    // parse args
    for (i = 1, l = (unsigned int)argc; i < l; i++) {
        if (strcmp(argv[i], "init") == 0) {
            rc = init();
            goto cleanup;
        }

        if (strcmp(argv[i], "reference") == 0) {
            rc = reference(argv[i+1], argv[i+2]);
            goto cleanup;
        }

        if (strcmp(argv[i], "registry") == 0) {
            if (i + 1 < l) {
                rc = set_registry(argv[i+1]);
            } else {
                rc = get_registry(&registry);

                if (registry) {
                    printf("%s\n", registry);
                }
            }

            goto cleanup;
        }

        if (strcmp(argv[i], "install") == 0) {
            if (i + 1 < l) {
                while (++argv != NULL && argv[i] != NULL) {
                    if ((rc = lib_install(argv[i])) != RETURN_OK) {
                        goto cleanup;
                    }

                    printf("Installed `%s`\n", argv[i]);
                }
            } else {
                rc = usage(exec);
            }

            goto cleanup;
        }

        if (strcmp(argv[i], "uninstall") == 0) {
            if (i + 1 < l) {
                while (++argv != NULL && argv[i] != NULL) {
                    if ((rc = lib_uninstall(argv[i])) != RETURN_OK) {
                        goto cleanup;
                    }

                    printf("Uninstalled `%s`\n", argv[i]);
                }
            } else {
                rc = usage(exec);
            }

            goto cleanup;
        }

        if (strcmp(argv[i], "info") == 0) {
            if (i + 1 < l) {
                if ((rc = lib_info(argv[i+1])) != RETURN_OK) {
                    fprintf(stderr, "Could not find info for `%s`\n", argv[i+1]);
                    goto cleanup;
                }
            } else {
                rc = usage(exec);
            }

            goto cleanup;
        }

        if (strcmp(argv[i], "ls") == 0) {
            if ((rc = lib_list()) != RETURN_OK) {
                goto cleanup;
            }

            goto cleanup;
        }

        if (strcmp(argv[i], "search") == 0) {
            if (i + 1 < l) {
                if ((rc = lib_search(argv[i+1])) != RETURN_OK) {
                    goto cleanup;
                }
            } else {
                rc = usage(exec);
            }

            goto cleanup;
        }

        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            rc = usage(exec);
            goto cleanup;
        }

        if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0) {
            if (i + 1 < l) {
                outfilename = nessemble_strdup(argv[i + 1]);
            } else {
                rc = usage(exec);
                goto cleanup;
            }

            i += 1;
            continue;
        }

        if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--format") == 0) {
            if (i + 1 < l) {
                if (strcmp(argv[i+1], "nes") == 0 || strcmp(argv[i+1], "NES") == 0) {
                    flags |= FLAG_NES;
                }
            } else {
                rc = usage(exec);
                goto cleanup;
            }

            i += 1;
            continue;
        }

        if (strcmp(argv[i], "-u") == 0 || strcmp(argv[i], "--undocumented") == 0) {
            flags |= FLAG_UNDOCUMENTED;
            continue;
        }

        if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--list") == 0) {
            if (i + 1 < l) {
                listname = nessemble_strdup(argv[i+1]);
            } else {
                rc = usage(exec);
                goto cleanup;
            }

            i += 1;
            continue;
        }

        if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--check") == 0) {
            flags |= FLAG_CHECK;
            continue;
        }

        if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--disassemble") == 0) {
            flags |= FLAG_DISASSEMBLE;
            continue;
        }

        if (strcmp(argv[i], "-R") == 0 || strcmp(argv[i], "--reassemble") == 0) {
            flags |= FLAG_REASSEMBLE;
            continue;
        }

        if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--simulate") == 0) {
            flags |= FLAG_SIMULATE;
            continue;
        }

        if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--recipe") == 0) {
            if (i + 1 < l) {
                recipe = argv[i+1];
            } else {
                rc = usage(exec);
                goto cleanup;
            }

            i += 1;
            continue;
        }

        if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
            rc = version();
            goto cleanup;
        }

        if (strcmp(argv[i], "-L") == 0 || strcmp(argv[i], "--license") == 0) {
            rc = license();
            goto cleanup;
        }

        if (filename != NULL) {
            rc = usage(exec);
            goto cleanup;
        }

        filename = argv[i];
    }

    if (!filename) {
        if (isatty(fileno(stdin)) == FALSE) {
            piped = TRUE;
            filename = nessemble_strdup("stdin");
            strcpy(cwd, "/tmp/" PROGRAM_NAME "-stdin");
            cwd_path = nessemble_strdup("/tmp/" PROGRAM_NAME);

            if ((rc = tmp_save(stdin, cwd)) != RETURN_OK) {
                rc = usage(exec);
                goto cleanup;
            }
        } else {
            rc = usage(exec);
            goto cleanup;
        }
    } else {
        // get cwd
        if (!realpath(filename, cwd)) {
            fprintf(stderr, "Could not find real path of %s\n", filename);

            rc = RETURN_EPERM;
            goto cleanup;
        }

        cwd_path = nessemble_strdup(cwd);

        if (!cwd_path) {
            fprintf(stderr, "Could not find path of %s\n", filename);

            rc = RETURN_EPERM;
            goto cleanup;
        }

        for (i = (unsigned int)strlen(cwd_path), l = 0; i >= l; i--) {
            if (cwd_path[i] == '/') {
                cwd_path[i] = '\0';
                break;
            }
        }
    }

    // simulate
    if (is_flag_simulate() == TRUE) {
        if (!recipe) {
            rc = simulate(cwd, "");
        } else {
            rc = simulate(cwd, recipe);
        }

        goto cleanup;
    }

    // output
    if (!outfilename || strcmp(outfilename, "-") == 0) {
        nessemble_free(outfilename);

        outfilename = nessemble_strdup("/dev/stdout");
    }

    if (!outfilename) {
        fprintf(stderr, "Could not find output\n");

        rc = RETURN_EPERM;
        goto cleanup;
    }

    // disassemble
    if (is_flag_disassemble() == TRUE) {
        rc = disassemble(cwd, outfilename, listname);
        goto cleanup;
    }

    // open file
    file = fopen(cwd, "r");

    if (!file) {
        fprintf(stderr, "Could not open %s\n", filename);

        rc = RETURN_EPERM;
        goto cleanup;
    }

    // save filename
    strcpy(filename_stack[include_stack_ptr], cwd);

    yyin = file;

    // segment
    strcpy(segment, "PRG0");
    segment_type = SEGMENT_PRG;

    // offsets
    for (i = 0, l = MAX_BANKS; i < l; i++) {
        prg_offsets[i] = 0x00;
        chr_offsets[i] = 0x00;
    }

    /* PASS 1 */

    pass = 1;

    do {
        (void)yyparse();
    } while ((yyin != NULL && feof(yyin) == 0) && pass == 1);

    if ((rc = error_exit()) != RETURN_OK) {
        goto cleanup;
    }

    /* PASS 2 */

    // restart
    if (fseek(yyin, 0, SEEK_SET) != 0) {
        fprintf(stderr, "Seek error\n");

        rc = RETURN_EPERM;
        goto cleanup;
    }

    // get offset max
    if (is_flag_nes() == TRUE) {
        offset_max = (int)((ines.prg * BANK_PRG) + (ines.chr * BANK_CHR));
    }

    // create rom
    rom = (unsigned int *)nessemble_malloc(sizeof(unsigned int) * offset_max);

    // set all bytes to 0xFF
    for (i = 0, l = (unsigned int)offset_max; i < l; i++) {
        rom[i] = 0xFF;
    }

    // set all trainer bytes to 0xFF
    for (i = 0, l = TRAINER_MAX; i < l; i++) {
        trainer[i] = 0xFF;
    }

    // clear offsets
    offset_trainer = 0;

    // segments
    strcpy(segment, "PRG0");
    segment_type = SEGMENT_PRG;

    // offsets
    prg_index = 0;
    chr_index = 0;

    for (i = 0, l = MAX_BANKS; i < l; i++) {
        prg_offsets[i] = 0x00;
        chr_offsets[i] = 0x00;
    }

    // reset lineno
    yylineno = 1;
    include_stack_ptr = 0;

    do {
        (void)yyparse();
    } while ((yyin != NULL && feof(yyin) == 0) && pass == 2);

    if ((rc = error_exit()) != RETURN_OK) {
        goto cleanup;
    }

    /* DONE */

    // check
    if (is_flag_check() == TRUE) {
        fprintf(stderr, "No errors\n");
        goto cleanup;
    }

    // write output
    if (strcmp(outfilename, "/dev/stdout") == 0) {
        outfile = stdout;
    } else {
        outfile = fopen(outfilename, "w");
    }

    if (!outfile) {
        fprintf(stderr, "Could not open %s\n", outfilename);

        rc = RETURN_EPERM;
        goto cleanup;
    }

    // write nes header
    if (is_flag_nes() == TRUE) {
        (void)fwrite("NES", 3, 1, outfile); // 0-2

        byte = 0x1A;
        (void)fwrite(&byte, 1, 1, outfile); // 3

        (void)fwrite(&ines.prg, 1, 1, outfile); // 4
        (void)fwrite(&ines.chr, 1, 1, outfile); // 5

        byte = 0;
        byte |= ines.mir & 0x01;
        byte |= (ines.trn & 0x01) << 0x02;
        byte |= (ines.map & 0x0F) << 0x04;

        (void)fwrite(&byte, 1, 1, outfile); // 6

        byte = 0;
        byte |= (ines.map & 0xF0);

        (void)fwrite(&byte, 1, 1, outfile); // 7

        byte = 0;

        (void)fwrite(&byte, 1, 1, outfile); // 8
        (void)fwrite(&byte, 1, 1, outfile); // 9
        (void)fwrite(&byte, 1, 1, outfile); // 10

        for (i = 11, l = 16; i < l; i++) {
            (void)fwrite(&byte, 1, 1, outfile); // 11-15
        }
    }

    // write trainer
    if (ines.trn == 1) {
        for (i = 0, l = TRAINER_MAX; i < l; i++) {
            (void)fwrite(trainer+i, 1, 1, outfile);
        }
    }

    // write rom data
    for (i = 0, l = (unsigned int)offset_max; i < l; i++) {
        (void)fwrite(rom+i, 1, 1, outfile);
    }

    // write list
    if ((rc = output_list(listname)) != RETURN_OK) {
        goto cleanup;
    }

cleanup:
    for (i = 0, l = symbol_index; i < l; i++) {
        nessemble_free(symbols[i].name);
    }

    nessemble_free(rom);
    nessemble_free(outfilename);
    nessemble_free(cwd_path);
    nessemble_free(listname);
    nessemble_free(registry);

    if (file) {
        (void)fclose(file);
    }

    if (outfile) {
        (void)fclose(outfile);
    }

    if (piped == TRUE) {
        tmp_delete(cwd);
    }

    error_free();

    return rc;
}
