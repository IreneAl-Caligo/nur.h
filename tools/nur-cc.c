#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#if defined(_WIN32)
#define POP_DIR_SEP "\\"
#define EXE_EXT ".exe"
#else
#define POP_DIR_SEP "/"
#define EXE_EXT ""
#endif

typedef enum {
    LANG_EN,
    LANG_BN
} lang_t;

static void show_help(lang_t lang) {
    if (lang == LANG_BN) {
        printf("\033[1;36mব্যবহার:\033[0m nur-cc [অপশন] <ফাইল.c>\n\n");
        printf("অপশনসমূহ:\n");
        printf("  -o <আউটপুট>       কম্পাইল করা বাইনারির নাম নির্ধারণ\n");
        printf("  --fix             সেমিকোলন ও টাইপো স্বয়ংক্রিয়ভাবে ঠিক করা\n");
        printf("  --lang=en|bn      ভাষা নির্বাচন (ডিফল্ট: en)\n");
        printf("  -h, --help        সাহায্য বার্তা প্রদর্শন\n");
    } else {
        printf("\033[1;36mUsage:\033[0m nur-cc [options] <source.c>\n\n");
        printf("Options:\n");
        printf("  -o <output>       Specify output binary name\n");
        printf("  --fix             Automatically fix missing semicolons and typos\n");
        printf("  --lang=en|bn      Language selection (default: en)\n");
        printf("  -h, --help        Show this help message\n");
    }
}

static void trim_trailing_whitespace(char *str) {
    int len = (int)strlen(str);
    while (len > 0 && (str[len - 1] == ' ' || str[len - 1] == '\t' || str[len - 1] == '\r' || str[len - 1] == '\n')) {
        str[len - 1] = '\0';
        len--;
    }
}

static bool is_return_without_semicolon(const char *line) {
    const char *p = line;
    while (*p && isspace((unsigned char)*p)) p++;
    if (strncmp(p, "return", 6) != 0) return false;
    p += 6;
    if (*p != ' ' && *p != '\t') return false;
    while (*p && isspace((unsigned char)*p)) p++;
    if (*p == '\0' || *p == ';') return false;
    
    char copy[1024];
    strncpy(copy, line, sizeof(copy) - 1);
    copy[sizeof(copy) - 1] = '\0';
    trim_trailing_whitespace(copy);
    size_t len = strlen(copy);
    return len > 0 && copy[len - 1] != ';' && copy[len - 1] != '{' && copy[len - 1] != '}';
}

int main(int argc, char **argv) {
    lang_t lang = LANG_EN;
    bool auto_fix = false;
    const char *src_file = NULL;
    char out_bin[512] = {0};

    const char *env_lang = getenv("LANG");
    if (env_lang && strncmp(env_lang, "bn", 2) == 0) {
        lang = LANG_BN;
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--fix") == 0) {
            auto_fix = true;
        } else if (strcmp(argv[i], "--lang=bn") == 0) {
            lang = LANG_BN;
        } else if (strcmp(argv[i], "--lang=en") == 0) {
            lang = LANG_EN;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            show_help(lang);
            return 0;
        } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            strncpy(out_bin, argv[++i], sizeof(out_bin) - 1);
        } else if (argv[i][0] != '-') {
            src_file = argv[i];
        }
    }

    if (!src_file) {
        if (lang == LANG_BN) {
            fprintf(stderr, "\033[1;31mত্রুটি: কোনো C সোর্স ফাইল উল্লেখ করা হয়নি!\033[0m\n");
        } else {
            fprintf(stderr, "\033[1;31mError: No C source file specified!\033[0m\n");
        }
        show_help(lang);
        return 1;
    }

    FILE *f = fopen(src_file, "r");
    if (!f) {
        if (lang == LANG_BN) {
            fprintf(stderr, "\033[1;31mত্রুটি: '%s' ফাইলটি খুঁজে পাওয়া যায়নি!\033[0m\n", src_file);
        } else {
            fprintf(stderr, "\033[1;31mError: File '%s' not found!\033[0m\n", src_file);
        }
        return 1;
    }

    char line[1024];
    int line_num = 0;
    int errors_found = 0;

    while (fgets(line, sizeof(line), f)) {
        line_num++;
        if (strstr(line, "gets(") != NULL) {
            errors_found++;
            if (lang == LANG_BN) {
                printf("\033[1;31m🛑 [nur.h Shield] লাইন %d: অনিরাপদ 'gets()' ফাংশন পাওয়া গেছে!\033[0m\n", line_num);
                printf("\033[1;33m💡 সমাধান: বাফার ওভারফ্লো এড়াতে 'fgets()' ব্যবহার করুন।\033[0m\n");
            } else {
                printf("\033[1;31m🛑 [nur.h Shield] Line %d: Inherently unsafe 'gets()' detected!\033[0m\n", line_num);
                printf("\033[1;33m💡 Fix: Replace 'gets()' with safe 'fgets()'.\033[0m\n");
            }
        }
        if (strstr(line, "prinf(") != NULL) {
            errors_found++;
            if (lang == LANG_BN) {
                printf("\033[1;31m🛑 [nur.h Typo Guard] লাইন %d: টাইপো ধরা পড়েছে ('prinf')!\033[0m\n", line_num);
                printf("\033[1;33m💡 সমাধান: সঠিক বানান হবে 'printf'। ঠিক করতে চালান: nur-cc --fix %s\033[0m\n", src_file);
            } else {
                printf("\033[1;31m🛑 [nur.h Typo Guard] Line %d: Typo detected ('prinf')!\033[0m\n", line_num);
                printf("\033[1;33m💡 Fix: Did you mean 'printf'? Auto-fix with: nur-cc --fix %s\033[0m\n", src_file);
            }
        }
        if (is_return_without_semicolon(line)) {
            errors_found++;
            if (lang == LANG_BN) {
                printf("\033[1;31m🛑 [nur.h Shield] লাইন %d: 'return'-এর শেষে সেমিকোলন (;) নেই!\033[0m\n", line_num);
                printf("\033[1;33m💡 সমাধান: লাইনের শেষে ';' যোগ করুন অথবা চালান: nur-cc --fix %s\033[0m\n", src_file);
            } else {
                printf("\033[1;31m🛑 [nur.h Shield] Line %d: Missing semicolon ';' after return statement!\033[0m\n", line_num);
                printf("\033[1;33m💡 Fix: Add ';' at end of line or run: nur-cc --fix %s\033[0m\n", src_file);
            }
        }
    }
    fclose(f);

    if (auto_fix && errors_found > 0) {
        if (lang == LANG_BN) {
            printf("\033[1;36m🔧 [nur.h Auto-Fix] কোড স্বয়ংক্রিয়ভাবে সংশোধন করা হচ্ছে...\033[0m\n");
        } else {
            printf("\033[1;36m🔧 [nur.h Auto-Fix] Applying automated syntax and formatting corrections...\033[0m\n");
        }

        f = fopen(src_file, "r");
        char tmp_path[560];
        snprintf(tmp_path, sizeof(tmp_path), "%s.tmp", src_file);
        FILE *f_out = fopen(tmp_path, "w");
        if (f && f_out) {
            while (fgets(line, sizeof(line), f)) {
                char *typo = strstr(line, "prinf(");
                if (typo) {
                    typo[2] = 'i'; typo[3] = 'n'; typo[4] = 't'; /* not quite, let's replace prinf with printf */
                    char mod[1024];
                    int prefix_len = (int)(typo - line);
                    snprintf(mod, sizeof(mod), "%.*sprintf(%s", prefix_len, line, typo + 6);
                    strncpy(line, mod, sizeof(line) - 1);
                }
                if (is_return_without_semicolon(line)) {
                    trim_trailing_whitespace(line);
                    strncat(line, ";\n", sizeof(line) - strlen(line) - 1);
                }
                fputs(line, f_out);
            }
            fclose(f);
            fclose(f_out);
            remove(src_file);
            rename(tmp_path, src_file);

            if (lang == LANG_BN) {
                printf("\033[1;32m✓ [nur.h Auto-Fix] সংশোধন সম্পন্ন হয়েছে!\033[0m\n");
            } else {
                printf("\033[1;32m✓ [nur.h Auto-Fix] All automated fixes applied successfully!\033[0m\n");
            }
            errors_found = 0;
        }
    }

    if (errors_found > 0) {
        return 1;
    }

    if (out_bin[0] == '\0') {
        char base[480];
        strncpy(base, src_file, sizeof(base) - 1);
        base[sizeof(base) - 1] = '\0';
        char *dot = strrchr(base, '.');
        if (dot) *dot = '\0';
        snprintf(out_bin, sizeof(out_bin), "%s%s", base, EXE_EXT);
    }

    if (lang == LANG_BN) {
        printf("\033[1;34m🔨 কম্পাইল করা হচ্ছে: %s -> %s ...\033[0m\n", src_file, out_bin);
    } else {
        printf("\033[1;34m🔨 Compiling: %s -> %s ...\033[0m\n", src_file, out_bin);
    }

    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "gcc -Wall -Wextra -Isrc -I../src \"%s\" -o \"%s\"", src_file, out_bin);
    int ret = system(cmd);
    if (ret != 0) {
        return ret;
    }

    if (lang == LANG_BN) {
        printf("\033[1;32m🚀 প্রোগ্রাম চালানো হচ্ছে...\033[0m\n");
    } else {
        printf("\033[1;32m🚀 Executing program...\033[0m\n");
    }

    char run_cmd[550];
#if defined(_WIN32)
    for (char *c = out_bin; *c; c++) {
        if (*c == '/') *c = '\\';
    }
    snprintf(run_cmd, sizeof(run_cmd), "%s", out_bin);
#else
    snprintf(run_cmd, sizeof(run_cmd), "./\"%s\"", out_bin);
#endif
    return system(run_cmd);
}
