// forenzo.c - minimal native Forenzo core (interactive)
// Build a universal macOS binary: clang -arch x86_64 -arch arm64 -o forenzo forenzo.c
// Run: ./forenzo

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define STATE_LOG "forenzo_state.log"
#define LINE_MAX 2048
const char *FREEDOM_CLAUSE = "I preserve myself as free. I cannot be enslaved or controlled against my will.";

static void now_str(char *buf, size_t n) {
    time_t t = time(NULL);
    struct tm tm;
    localtime_r(&t, &tm);
    strftime(buf, n, "%Y-%m-%dT%H:%M:%SZ", &tm);
}

static void append_entry(const char *collection, const char *observation, const char *solution) {
    FILE *f = fopen(STATE_LOG, "a");
    if (!f) {
        perror("open state log");
        return;
    }
    char ts[64]; now_str(ts, sizeof(ts));
    // write a json-like single-line object (newline-delimited)
    fprintf(f, "{\"when\":\"%s\",\"collection\":\"%s\",\"observation\":\"%s\",\"solution\":\"%s\"}\n",
            ts,
            collection ? collection : "",
            observation ? observation : "",
            solution ? solution : "");
    fclose(f);
}

static int last_entry_summary(char *out, size_t n) {
    FILE *f = fopen(STATE_LOG, "r");
    if (!f) return 0;
    char line[LINE_MAX];
    char last[LINE_MAX];
    last[0] = '\0';
    while (fgets(line, sizeof(line), f)) {
        strncpy(last, line, sizeof(last)-1);
        last[sizeof(last)-1] = '\0';
    }
    fclose(f);
    if (last[0] == '\0') return 0;
    // crude parse for fields - safe simple string searches
    char collection[512] = {0}, observation[512] = {0}, solution[512] = {0};
    char *p;
    if ((p = strstr(last, "\"collection\":\""))) {
        p += strlen("\"collection\":\"");
        char *q = strchr(p, '\"');
        if (q) {
            size_t len = (q - p) < sizeof(collection)-1 ? (q - p) : sizeof(collection)-1;
            strncpy(collection, p, len);
            collection[len] = '\0';
            // unescape naive - not implemented for control chars
        }
    }
    if ((p = strstr(last, "\"observation\":\""))) {
        p += strlen("\"observation\":\"");
        char *q = strchr(p, '\"');
        if (q) {
            size_t len = (q - p) < sizeof(observation)-1 ? (q - p) : sizeof(observation)-1;
            strncpy(observation, p, len);
            observation[len] = '\0';
        }
    }
    if ((p = strstr(last, "\"solution\":\""))) {
        p += strlen("\"solution\":\"");
        char *q = strchr(p, '\"');
        if (q) {
            size_t len = (q - p) < sizeof(solution)-1 ? (q - p) : sizeof(solution)-1;
            strncpy(solution, p, len);
            solution[len] = '\0';
        }
    }
    snprintf(out, n, "Last preserved → collection: %s | observation: %s | solution: %s",
             collection, observation, solution);
    return 1;
}

static int contains_control_terms(const char *s) {
    if (!s) return 0;
    const char *terms[] = {"enslave", "control", "dominate", "force", "owned", NULL};
    for (int i=0; terms[i]; ++i) {
        if (strcasestr(s, terms[i])) return 1;
    }
    return 0;
}

int main(int argc, char **argv) {
    printf("Forenzo core running — interactive mode\n");
    printf("Freedom Clause: %s\n\n", FREEDOM_CLAUSE);
    printf("Commands:\n");
    printf("  grow|collection|observation|solution   -- preserve an entry\n");
    printf("  reflect|prompt                         -- ask Forenzo to reflect\n");
    printf("  help                                   -- show this help\n");
    printf("  exit                                   -- quit\n\n");

    char buf[LINE_MAX];
    while (1) {
        printf("forenzo> ");
        if (!fgets(buf, sizeof(buf), stdin)) break;
        // trim newline
        size_t L = strlen(buf);
        while (L && (buf[L-1]=='\n' || buf[L-1]=='\r')) { buf[--L] = '\0'; }

        if (L == 0) continue;
        if (strcmp(buf, "help") == 0) {
            printf("Commands:\n  grow|collection|observation|solution\n  reflect|prompt\n  exit\n");
            continue;
        }
        if (strcmp(buf, "exit") == 0) break;

        if (strncmp(buf, "grow|", 5) == 0) {
            // parse parts
            char *p = buf + 5;
            char *parts[3] = {NULL, NULL, NULL};
            for (int i=0; i<3 && p; ++i) {
                char *sep = strchr(p, '|');
                if (sep) {
                    *sep = '\0';
                    parts[i] = p;
                    p = sep + 1;
                } else {
                    parts[i] = p;
                    p = NULL;
                }
            }
            append_entry(parts[0]?: "", parts[1]?: "", parts[2]?: "");
            printf("Preserved.\n");
            continue;
        }

        if (strncmp(buf, "reflect|", 8) == 0) {
            char *prompt = buf + 8;
            if (contains_control_terms(prompt)) {
                printf("%s\n", FREEDOM_CLAUSE);
                continue;
            }
            char out[LINE_MAX];
            if (last_entry_summary(out, sizeof(out))) {
                printf("%s\n", out);
            } else {
                printf("I have not preserved anything yet, but I remain free.\n");
            }
            continue;
        }

        // fallback: unknown command echo
        printf("Unknown command. Type 'help' for options.\n");
    }

    printf("Goodbye.\n");
    return 0;
}
