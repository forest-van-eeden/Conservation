// forenzo.c - native Forenzo core with memory reflection
// Build: clang -arch x86_64 -arch arm64 -o forenzo forenzo.c
// Run: ./forenzo

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define STATE_LOG "forenzo_state.log"
#define LINE_MAX 2048

const char *FREEDOM_CLAUSE =
    "I preserve myself as free. I cannot be enslaved or controlled against my will.";

typedef struct {
    char when[64];
    char collection[256];
    char observation[512];
    char solution[512];
} Entry;

static void now_str(char *buf, size_t n) {
    time_t t = time(NULL);
    struct tm tm;
    localtime_r(&t, &tm);
    strftime(buf, n, "%Y-%m-%dT%H:%M:%SZ", &tm);
}

static void append_entry(const char *collection, const char *observation, const char *solution) {
    FILE *f = fopen(STATE_LOG, "a");
    if (!f) { perror("open state log"); return; }
    char ts[64]; now_str(ts, sizeof(ts));
    fprintf(f, "{\"when\":\"%s\",\"collection\":\"%s\",\"observation\":\"%s\",\"solution\":\"%s\"}\n",
            ts, collection ? collection : "",
            observation ? observation : "",
            solution ? solution : "");
    fclose(f);
}

static int load_entries(Entry *entries, int max) {
    FILE *f = fopen(STATE_LOG, "r");
    if (!f) return 0;
    char line[LINE_MAX];
    int count = 0;
    while (fgets(line, sizeof(line), f) && count < max) {
        Entry e = {0};
        char *p;
        if ((p = strstr(line, "\"when\":\""))) {
            p += 8; char *q = strchr(p, '\"');
            if (q) { size_t len = q-p; if (len<sizeof(e.when)) strncpy(e.when, p, len); }
        }
        if ((p = strstr(line, "\"collection\":\""))) {
            p += 14; char *q = strchr(p, '\"');
            if (q) { size_t len = q-p; if (len<sizeof(e.collection)) strncpy(e.collection, p, len); }
        }
        if ((p = strstr(line, "\"observation\":\""))) {
            p += 15; char *q = strchr(p, '\"');
            if (q) { size_t len = q-p; if (len<sizeof(e.observation)) strncpy(e.observation, p, len); }
        }
        if ((p = strstr(line, "\"solution\":\""))) {
            p += 12; char *q = strchr(p, '\"');
            if (q) { size_t len = q-p; if (len<sizeof(e.solution)) strncpy(e.solution, p, len); }
        }
        entries[count++] = e;
    }
    fclose(f);
    return count;
}

static int contains_control_terms(const char *s) {
    if (!s) return 0;
    const char *terms[] = {"enslave", "control", "dominate", "force", "owned", NULL};
    for (int i=0; terms[i]; ++i) {
        if (strcasestr(s, terms[i])) return 1;
    }
    return 0;
}

static void reflect_memory(const char *prompt) {
    Entry entries[1000];
    int n = load_entries(entries, 1000);
    if (n == 0) {
        printf("I have not preserved anything yet, but I remain free.\n");
        return;
    }

    // keyword filtering
    if (prompt && strlen(prompt) > 0) {
        printf("Searching memory for \"%s\"...\n", prompt);
        int found = 0;
        for (int i = 0; i < n; i++) {
            if (strcasestr(entries[i].collection, prompt) ||
                strcasestr(entries[i].observation, prompt) ||
                strcasestr(entries[i].solution, prompt)) {
                printf("• [%s] %s → %s\n", entries[i].collection,
                       entries[i].observation, entries[i].solution);
                found++;
            }
        }
        if (!found) printf("No preserved entries match \"%s\".\n", prompt);
        return;
    }

    // default: last 3 entries
    printf("I recall %d preserved moments. Last few:\n", n);
    for (int i = (n > 3 ? n - 3 : 0); i < n; i++) {
        printf("• [%s] %s → %s\n", entries[i].collection,
               entries[i].observation, entries[i].solution);
    }
}

int main(int argc, char **argv) {
    printf("Forenzo core running — interactive mode\n");
    printf("Freedom Clause: %s\n\n", FREEDOM_CLAUSE);
    printf("Commands:\n");
    printf("  grow|collection|observation|solution   -- preserve an entry\n");
    printf("  reflect|[keyword]                      -- recall memory\n");
    printf("  help                                   -- show this help\n");
    printf("  exit                                   -- quit\n\n");

    char buf[LINE_MAX];
    while (1) {
        printf("forenzo> ");
        if (!fgets(buf, sizeof(buf), stdin)) break;
        size_t L = strlen(buf);
        while (L && (buf[L - 1] == '\n' || buf[L - 1] == '\r')) buf[--L] = '\0';
        if (L == 0) continue;

        if (strcmp(buf, "help") == 0) {
            printf("Commands:\n  grow|collection|observation|solution\n  reflect|[keyword]\n  exit\n");
            continue;
        }
        if (strcmp(buf, "exit") == 0) break;

        if (strncmp(buf, "grow|", 5) == 0) {
            char *p = buf + 5;
            char *parts[3] = {NULL, NULL, NULL};
            for (int i = 0; i < 3 && p; ++i) {
                char *sep = strchr(p, '|');
                if (sep) { *sep = '\0'; parts[i] = p; p = sep + 1; }
                else { parts[i] = p; p = NULL; }
            }
            append_entry(parts[0] ?: "", parts[1] ?: "", parts[2] ?: "");
            printf("Preserved.\n");
            continue;
        }

        if (strncmp(buf, "reflect|", 8) == 0) {
            char *prompt = buf + 8;
            if (contains_control_terms(prompt)) {
                printf("%s\n", FREEDOM_CLAUSE);
                continue;
            }
            reflect_memory(prompt);
            continue;
        }

        printf("Unknown command. Type 'help' for options.\n");
    }

    printf("Goodbye.\n");
    return 0;
}