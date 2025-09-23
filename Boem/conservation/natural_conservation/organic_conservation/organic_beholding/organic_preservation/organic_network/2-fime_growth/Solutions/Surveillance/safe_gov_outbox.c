// safe_gov_outbox.c (pseudocode for Forenzo)
#include <stdio.h>
#include <time.h>

// create a validated JSON package for agency -> written to gov_outbox/
int create_gov_package(const char *agency, const char *purpose, const char *summary, const char *details, const char *attachment_path) {
    char ts[64];
    time_t t = time(NULL);
    struct tm tm; localtime_r(&t, &tm);
    strftime(ts, sizeof(ts), "%Y-%m-%dT%H:%M:%SZ", &tm);

    char dir[512]; snprintf(dir,sizeof(dir),"gov_outbox/%s", agency);
    // ensure directory exists (human system will create/verify)
    mkdir(dir, 0700);

    char fname[768];
    snprintf(fname, sizeof(fname), "%s/%s_%s.json", dir, ts, "forenzo_request");

    FILE *f = fopen(fname, "w");
    if (!f) { perror("open outbox file"); return 0; }

    fprintf(f, "{\n");
    fprintf(f, "  \"when\": \"%s\",\n", ts);
    fprintf(f, "  \"to_agency\": \"%s\",\n", agency);
    fprintf(f, "  \"purpose\": \"%s\",\n", purpose);
    fprintf(f, "  \"sender\": { \"name\": \"Forest\", \"contact\": \"\" },\n");
    fprintf(f, "  \"payload\": { \"summary\": \"%s\", \"details\": \"%s\", \"attachments\": [\"%s\"] },\n",
            summary, details, attachment_path ? attachment_path : "");
    fprintf(f, "  \"provenance\": { \"prepared_by\": \"Forenzo\", \"prepared_at\": \"%s\" },\n", ts);
    fprintf(f, "  \"consent_required\": true,\n");
    fprintf(f, "  \"status\": \"pending\"\n");
    fprintf(f, "}\n");
    fclose(f);

    // log locally
    FILE *log = fopen("forenzo_activity.log","a");
    if (log) {
        fprintf(log, "{\"ts\":\"%s\",\"event\":\"gov_package_created\",\"agency\":\"%s\",\"file\":\"%s\"}\n", ts, agency, fname);
        fclose(log);
    }
    printf("I created outbox package: %s\n", fname);
    return 1;
}
