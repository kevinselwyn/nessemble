#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <getopt.h>
#include <setjmp.h>
#include "nessemble.h"

/**
 * Main function
 * @param {int} argc - Argument count
 * @param {char *} argv[] - Argument array
 */
int main(int argc, char *argv[]) {
    int option_index = 0, c = 0, empty_byte = 0xFF;
    unsigned int rc = RETURN_OK;
    unsigned int i = 0, l = 0, byte = 0, piped = FALSE, ref_count = 0;
    char *exec = NULL, *filename = NULL, *outfilename = NULL;
    char *listname = NULL, *recipe = NULL, *package = NULL;
    char *registry = NULL, *config = NULL, *install_path = NULL;
    FILE *file = NULL, *outfile = NULL;

    translate_init();

#if !defined(IS_WINDOWS) && !defined(IS_JAVASCRIPT)
    /* setup error catch */
    if (setjmp(error_jmp) != 0) {
        goto cleanup;
    }

    //error_signal();
#endif /* !IS_WINDOWS && !IS_JAVASCRIPT */

    /* exec */
    exec = argv[0];

    /* parse args */
    while (TRUE == 1) {
        c = getopt_long(argc, argv, "cCde:f:hlLo:p:rRsuv", commandline_options, &option_index);

        if (c == -1) {
            break;
        }

        switch (c) {
        case 'h':
            rc = usage(exec);
            goto cleanup;

            break;
        case 'o':
            if (optarg == NULL) {
                rc = usage(exec);
                goto cleanup;
            }

            outfilename = nessemble_strdup(optarg);
            break;
        case 'f':
            if (optarg == NULL) {
                rc = usage(exec);
                goto cleanup;
            }

            if (strcmp(optarg, "nes") == 0 || strcmp(optarg, "NES") == 0) {
                flags |= FLAG_NES;
            }

            break;
        case 'e':
            if (optarg == NULL) {
                rc = usage(exec);
                goto cleanup;
            }

            empty_byte = hex2int(optarg);

            break;
        case 'u':
            flags |= FLAG_UNDOCUMENTED;
            break;
        case 'l':
            if (optarg == NULL) {
                rc = usage(exec);
                goto cleanup;
            }

            listname = nessemble_strdup(optarg);
            break;
        case 'p':
            if (optarg == NULL) {
                rc = usage(exec);
                goto cleanup;
            }

            pseudoname = nessemble_strdup(optarg);
            break;
        case 'c':
            flags |= FLAG_CHECK;
            break;
        case 'C':
            flags |= FLAG_COVERAGE;
            break;
        case 'd':
            flags |= FLAG_DISASSEMBLE;
            break;
        case 'R':
            flags |= FLAG_REASSEMBLE;
            break;
        case 's':
            flags |= FLAG_SIMULATE;
            break;
        case 'r':
            if (optarg == NULL) {
                rc = usage(exec);
                goto cleanup;
            }

            recipe = optarg;
            break;
        case 'v':
            rc = version();
            goto cleanup;

            break;
        case 'L':
            rc = license();
            goto cleanup;

            break;
        default:
            rc = usage(exec);
            goto cleanup;

            break;
        }
    }

    while (optind < argc) {
        if (strcmp(argv[optind], "init") == 0) {
            if ((rc = init(argc-(optind + 1), argv+(optind + 1))) != RETURN_OK) {
                error_program_output(_("Could not initialize project"));
            }

            goto cleanup;
        }

        if (strcmp(argv[optind], "scripts") == 0) {
            if ((rc = install_scripts(&install_path)) != RETURN_OK) {
                error_program_output(_("Could not install scripts"));
            }

            printf(_("Installed scripts to %s"), install_path);
            printf("\n");

            goto cleanup;
        }

        if (strcmp(argv[optind], "reference") == 0) {
            if (argv[optind+1] == NULL) {
                ref_count = 0;
            } else if (argv[optind+2] == NULL) {
                ref_count = 1;
            } else {
                ref_count = 2;
            }

            if ((rc = reference(ref_count, argv[optind+1], argv[optind+2])) != RETURN_OK) {
                error_program_output(_("Could not get reference info"));
            }

            goto cleanup;
        }

        if (strcmp(argv[optind], "config") == 0) {
            if (optind + 2 < argc) {
                if ((rc = set_config(argv[optind+2], argv[optind+1])) != RETURN_OK) {
                    error_program_output(_("Could not set `%s` config"), argv[optind+1]);
                }
            } else if (optind + 1 < argc) {
                if ((rc = get_config(&config, argv[optind+1])) != RETURN_OK) {
                    error_program_output(_("Could not get `%s` config"), argv[optind+1]);
                }
            } else {
                if ((rc = list_config(&config)) != RETURN_OK) {
                    error_program_output(_("Could not list config"));
                    goto cleanup;
                }
            }

            if (config) {
                printf("%s\n", config);
            }

            goto cleanup;
        }

        if (strcmp(argv[optind], "registry") == 0) {
            if (optind + 1 < argc) {
                rc = set_registry(argv[optind+1]);
            } else {
                if ((rc = get_registry(&registry)) != RETURN_OK) {
                    printf("%s\n", _("No registry set"));
                    goto cleanup;
                }

                printf("%s\n", registry);
            }

            goto cleanup;
        }

        if (strcmp(argv[optind], "install") == 0) {
            if (optind + 1 < argc) {
                while (++argv != NULL && argv[optind] != NULL) {
                    if ((rc = lib_install(argv[optind])) != RETURN_OK) {
                        error_program_output(_("Could not install `%s`"), argv[optind]);
                        goto cleanup;
                    }

                    printf(_("Installed `%s`"), argv[optind]);
                    printf("\n");
                }
            } else {
                rc = usage(exec);
            }

            goto cleanup;
        }

        if (strcmp(argv[optind], "uninstall") == 0) {
            if (optind + 1 < argc) {
                while (++argv != NULL && argv[optind] != NULL) {
                    if ((rc = lib_uninstall(argv[optind])) != RETURN_OK) {
                        error_program_output(_("Could not uninstall `%s`"), argv[optind]);
                        goto cleanup;
                    }

                    printf(_("Uninstalled `%s`"), argv[optind]);
                    printf("\n");
                }
            } else {
                rc = usage(exec);
            }

            goto cleanup;
        }

        if (strcmp(argv[optind], "publish") == 0) {
            if (optind + 1 < argc) {
                while (++argv != NULL && argv[optind] != NULL) {
                    if ((rc = lib_publish(argv[optind], &package)) != RETURN_OK) {
                        if (!package) {
                            error_program_output(_("Could not publish"));
                        } else {
                            error_program_output(_("Could not publish `%s`"), package);
                        }

                        goto cleanup;
                    }

                    printf(_("Published `%s`"), package);
                    printf("\n");
                }
            } else {
                rc = usage(exec);
            }

            goto cleanup;
        }

        if (strcmp(argv[optind], "info") == 0) {
            if (optind + 1 < argc) {
                if ((rc = lib_info(argv[optind+1])) != RETURN_OK) {
                    error_program_output(_("Could not find info for `%s`"), argv[optind+1]);
                    goto cleanup;
                }
            } else {
                rc = usage(exec);
            }

            goto cleanup;
        }

        if (strcmp(argv[optind], "ls") == 0) {
            if ((rc = lib_list()) != RETURN_OK) {
                goto cleanup;
            }

            goto cleanup;
        }

        if (strcmp(argv[optind], "search") == 0) {
            if (optind + 1 < argc) {
                if ((rc = lib_search(argv[optind+1])) != RETURN_OK) {
                    error_program_output(_("Could not search"));
                    goto cleanup;
                }
            } else {
                rc = usage(exec);
            }

            goto cleanup;
        }

        if (strcmp(argv[optind], "adduser") == 0) {
            if ((rc = user_create()) != RETURN_OK) {
                error_program_output(_("Could not add user"));
                goto cleanup;
            }

            printf("%s\n", _("User created"));

            goto cleanup;
        }

        if (strcmp(argv[optind], "login") == 0) {
            if ((rc = user_login()) != RETURN_OK) {
                error_program_output(_("Could not login"));
                goto cleanup;
            }

            printf("Login successful");
            printf("\n");

            goto cleanup;
        }

        if (strcmp(argv[optind], "logout") == 0) {
            if ((rc = user_logout()) != RETURN_OK) {
                error_program_output(_("Could not logout"));
                goto cleanup;
            }

            printf("%s\n", _("Logout successful"));

            goto cleanup;
        }

        filename = argv[optind++];
    }

    if (!filename) {
        if (isatty(fileno(stdin)) == FALSE) {
            piped = TRUE;
            filename = nessemble_strdup("stdin");
            strcpy(cwd, SEP "tmp" SEP PROGRAM_NAME "-stdin");
            cwd_path = nessemble_strdup(SEP "tmp" SEP PROGRAM_NAME);

            if ((rc = tmp_save(stdin, cwd)) != RETURN_OK) {
                rc = usage(exec);
                goto cleanup;
            }
        } else {
            rc = usage(exec);
            goto cleanup;
        }
    } else {
        /* get cwd */
        if (!nessemble_realpath(filename, cwd)) {
            error_program_output(_("Could not open `%s`"), filename);

            rc = RETURN_EPERM;
            goto cleanup;
        }

        cwd_path = nessemble_strdup(cwd);

        if (!cwd_path) {
            error_program_output(_("Could not open `%s`"), filename);

            rc = RETURN_EPERM;
            goto cleanup;
        }

        for (i = (unsigned int)strlen(cwd_path), l = 0; i >= l; i--) {
            if (cwd_path[i] == SEP_CHAR) {
                cwd_path[i] = '\0';
                break;
            }
        }
    }

    /* simulate */
    if (is_flag_simulate() == TRUE) {
        if ((rc = simulate(cwd, !recipe ? "" : recipe)) != RETURN_OK) {
            error_program_output(_("Could not simulate"));
        }

        goto cleanup;
    }

    /* output */
    if (!outfilename || strcmp(outfilename, "-") == 0) {
        nessemble_free(outfilename);

        outfilename = nessemble_strdup(STDOUT_FILE);
    }

    if (!outfilename) {
        error_program_output(_("Could not find output"));

        rc = RETURN_EPERM;
        goto cleanup;
    }

    /* disassemble */
    if (is_flag_disassemble() == TRUE) {
        if ((rc = disassemble(cwd, outfilename, listname)) != RETURN_OK) {
            error_program_output(_("Could not disassemble `%s`"), filename);
        }

        goto cleanup;
    }

    /* open file */
    file = fopen(cwd, "r");

    if (!file) {
        error_program_output(_("Could not open `%s`"), cwd);

        rc = RETURN_EPERM;
        goto cleanup;
    }

    /* save filename */
    strcpy(filename_stack[include_stack_ptr], cwd);

    yyin = file;

    /* segment */
    strcpy(segment, "PRG0");
    segment_type = SEGMENT_PRG;

    /* offsets */
    for (i = 0, l = MAX_BANKS; i < l; i++) {
        prg_offsets[i] = 0x00;
        chr_offsets[i] = 0x00;
    }

    /* PASS 1 */

    pass = 1;

    do {
        UNUSED(yyparse());
    } while ((yyin != NULL && feof(yyin) == 0) && pass == 1);

    if ((rc = error_exit()) != RETURN_OK) {
        goto cleanup;
    }

    /* PASS 2 */

    /* restart */
    if (fseek(yyin, 0, SEEK_SET) != 0) {
        error_program_output(_("Could not rewind input file"));

        rc = RETURN_EPERM;
        goto cleanup;
    }

    /* get offset max */
    if (is_flag_nes() == TRUE) {
        offset_max = (int)((ines.prg * BANK_PRG) + (ines.chr * BANK_CHR));
    }

    /* create rom */
    rom = (unsigned int *)nessemble_malloc(sizeof(unsigned int) * offset_max);
    coverage = (unsigned int *)nessemble_malloc(sizeof(unsigned int) * offset_max);

    /* set all bytes to empty_byte */
    for (i = 0, l = (unsigned int)offset_max; i < l; i++) {
        rom[i] = empty_byte;
        coverage[i] = FALSE;
    }

    /* set all trainer bytes to empty_byte */
    for (i = 0, l = TRAINER_MAX; i < l; i++) {
        trainer[i] = empty_byte;
    }

    /* clear offsets */
    offset_trainer = 0;

    /* segments */
    strcpy(segment, "PRG0");
    segment_type = SEGMENT_PRG;

    /* offsets */
    prg_index = 0;
    chr_index = 0;

    for (i = 0, l = MAX_BANKS; i < l; i++) {
        prg_offsets[i] = 0x00;
        chr_offsets[i] = 0x00;
    }

    /* reset lineno */
    yylineno = 1;
    include_stack_ptr = 0;

    do {
        UNUSED(yyparse());
    } while ((yyin != NULL && feof(yyin) == 0) && pass == 2);

    if ((rc = error_exit()) != RETURN_OK) {
        goto cleanup;
    }

    /* DONE */

    /* check */
    if (is_flag_check() == TRUE) {
        printf("%s\n", _("No errors"));
        goto cleanup;
    }

    /* write output */
    if (is_stdout(outfilename) == TRUE) {
        outfile = stdout;
    } else {
        outfile = fopen(outfilename, "w");
    }

    if (!outfile) {
        error_program_output(_("Could not open `%s`"), outfilename);

        rc = RETURN_EPERM;
        goto cleanup;
    }

    /* write nes header */
    if (is_flag_nes() == TRUE) {
        UNUSED(fwrite("NES", 3, 1, outfile)); /* 0-2 */

        byte = 0x1A;
        UNUSED(fwrite(&byte, 1, 1, outfile)); /* 3 */

        UNUSED(fwrite(&ines.prg, 1, 1, outfile)); /* 4 */
        UNUSED(fwrite(&ines.chr, 1, 1, outfile)); /* 5 */

        byte = 0;
        byte |= ines.mir & 0x01;
        byte |= (ines.trn & 0x01) << 0x02;
        byte |= (ines.map & 0x0F) << 0x04;

        UNUSED(fwrite(&byte, 1, 1, outfile)); /* 6 */

        byte = 0;
        byte |= (ines.map & 0xF0);

        UNUSED(fwrite(&byte, 1, 1, outfile)); /* 7 */

        byte = 0;

        UNUSED(fwrite(&byte, 1, 1, outfile)); /* 8 */
        UNUSED(fwrite(&byte, 1, 1, outfile)); /* 9 */
        UNUSED(fwrite(&byte, 1, 1, outfile)); /* 10 */

        for (i = 11, l = 16; i < l; i++) {
            UNUSED(fwrite(&byte, 1, 1, outfile)); /* 11-15 */
        }
    }

    /* write trainer */
    if (ines.trn == 1) {
        for (i = 0, l = TRAINER_MAX; i < l; i++) {
            UNUSED(fwrite(trainer+i, 1, 1, outfile));
        }
    }

    /* write rom data */
    for (i = 0, l = (unsigned int)offset_max; i < l; i++) {
        UNUSED(fwrite(rom+i, 1, 1, outfile));
    }

    /* write list */
    if ((rc = output_list(listname)) != RETURN_OK) {
        goto cleanup;
    }

    /* write coverage */
    if (is_flag_coverage() == TRUE && is_flag_nes() == TRUE && is_stdout(outfilename) == FALSE) {
        if ((rc = get_coverage()) != RETURN_OK) {
            goto cleanup;
        }
    }

cleanup:
    for (i = 0, l = symbol_index; i < l; i++) {
        nessemble_free(symbols[i].name);
    }

    nessemble_free(rom);
    nessemble_free(coverage);
    nessemble_free(outfilename);
    nessemble_free(cwd_path);
    nessemble_free(listname);
    nessemble_free(pseudoname);
    nessemble_free(registry);
    nessemble_free(config);
    nessemble_free(package);
    nessemble_free(install_path);
    nessemble_fclose(file);
    nessemble_fclose(outfile);

    if (piped == TRUE) {
        tmp_delete(cwd);
    }

    error_free();
    translate_free();

    return (int)rc;
}
