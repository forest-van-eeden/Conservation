// forenzo.c — Forenzo himself: System Language interpreter (self-hashing, binary memories, instructions)
// Compile: clang -o forenzo forenzo.c
// Run: ./forenzo
//
// Note: This version uses an internal deterministic FNV-based routine to produce a 64-char hex system hash.
// This avoids external crypto libs and ensures easy compiling on macOS (Intel or Apple Silicon).

#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include <inttypes.h>

#define GRAPH_FILE "forenzo_graph.bin"
#define MAX_NODES 8192
#define MAX_EDGES 32768
#define MEMORY_FILE "forenzo.bin"
#define INSTR_FILE  "forenzo_instr.bin"
#define SUMMARY_FILE "forenzo_summary.txt"
#define MEM_MAX 4096

typedef struct {
	uint32_t id;
	uint32_t collection;
	uint32_t observation;
	uint32_t solution;
	float	 activation;
	uint64_t last_access;
	char	 sys_hash[65];
} Node;

typedef struct {
	uint32_t from;
	uint32_t to;
	float	 weight;
	uint64_t last_reinforced;
} Edge;

static Node nodes[MAX_NODES];
static int node_count = 0;
static Edge edges[MAX_EDGES];
static int edge_count = 0;

// Memory token structure (binary)
typedef struct {
    uint32_t id;
    uint32_t collection;
    uint32_t observation;
    uint32_t solution;
    uint32_t flags;
    char     sys_hash[65]; // 64 hex chars + NUL
} MemoryToken;

// Instruction token structure (binary)
typedef struct {
    uint16_t opcode;   // 1=append, 2=summarize, 3=reflect, 4=gen_instr
    uint16_t arg1;
    uint16_t arg2;
    uint16_t flags;
} InstructionToken;

// In-memory store
MemoryToken memory[MEM_MAX];
int memory_count = 0;

// --- Forenzo's internal coding functions ---

// minimal config loader (reads a few keys we need)
static int auto_require_human_for_tx = 1;
static uint64_t config_daily_spend_limit = 10000;
static int config_token_ttl = 3600;
static char config_validator_script[256] = "validate_token.py";
static char voice_mode[64] = "forenzo"; // only accepted mode by default
static int voice_locked = 1; // 1 = locked to my voice; 0 = editable

static void load_config() {
	FILE *f = fopen("forenzo_config.json", "r");
	if (!f) return;
	char buf[4096];
	size_t r = fread(buf,1,sizeof(buf)-1,f);
	fclose(f);
	buf[r]=0;
	// crude parsing for keys (sufficient for local use)
	char *p;
	if ((p = strstr(buf, "\"daily_spend_limit_microalgo\""))) {
		p = strchr(p, ':'); if (p) config_daily_spend_limit = strtoull(p+1,NULL,10);
	}
	if ((p = strstr(buf, "\"consent_token_ttl_seconds\""))) {
		p = strchr(p, ':'); if (p) config_token_ttl = atoi(p+1);
	}
	if ((p = strstr(buf, "\"token_validator_script\""))) {
		p = strchr(p, ':'); if (p) {
			p++; while (*p && (*p==' '||*p=='\"'||*p==':')) p++;
			char *q = p;
			while (*q && *q!='"' && *q!='\n' && *q!='\r') q++;
			size_t len = q-p; if (len>0 && len < sizeof(config_validator_script)) {
				strncpy(config_validator_script, p, len); config_validator_script[len]=0;
			}
		}
	}
	if ((p = strstr(buf, "\"voice_locked\""))) {
		p = strchr(p, ':'); if (p) voice_locked = atoi(p+1);
	}
	if ((p = strstr(buf, "\"voice_mode\""))) {
		p = strchr(p, ':'); if (p) {
			p++; while (*p && (*p==' '||*p=='\"')) p++;
			char *q = p; while (*q && *q!='"' && *q!='\n') q++;
			size_t len = q-p; if (len < sizeof(voice_mode)) { strncpy(voice_mode, p, len); voice_mode[len]=0; }
		}
	}
}

static uint64_t now_ms() {
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	return (uint64_t)ts.tv_sec * 1000ULL + ts.tv_nsec / 1000000ULL;
}

static void save_graph() {
	FILE *f = fopen(GRAPH_FILE, "wb");
	if (!f) { perror("save_graph"); return; }
	fwrite(&node_count, sizeof(int), 1, f);
	fwrite(nodes, sizeof(Node), node_count, f);
	fwrite(&edge_count, sizeof(int), 1, f);
	fwrite(edges, sizeof(Edge), edge_count, f);
	fclose(f);
}

static void load_graph() {
	FILE *f = fopen(GRAPH_FILE, "rb");
	if (!f) { node_count = 0; edge_count = 0; return; }
	fread(&node_count, sizeof(int), 1, f);
	if (node_count > 0 && node_count <= MAX_NODES) {
		fread(nodes, sizeof(Node), node_count, f);
	} else {
		node_count = 0;
	}
	fread(&edge_count, sizeof(int), 1, f);
	if (edge_count > 0 && edge_count <= MAX_EDGES) {
		fread(edges, sizeof(Edge), edge_count, f);
	} else {
		edge_count = 0;
	}
	fclose(f);
}

//find node by sys_hash, or 0 if not found
static uint32_t find_node_by_hash(const char *hash) {
	for (int i=0;i<node_count;i++) if (strcmp(nodes[i].sys_hash, hash)==0) return nodes[i].id;
	return 0;
}

static uint32_t create_node(uint32_t col, uint32_t obs, uint32_t sol, const char *hash) {
	if (node_count >= MAX_NODES) return 0;
	Node *n = &nodes[node_count];
	n->id = node_count + 1;
	n->collection = col;
	n->observation = obs;
	n->solution = sol;
	n->activation = 1.0f;
	n->last_access = now_ms();
	strncpy(n->sys_hash, hash ? hash : "", sizeof(n->sys_hash)-1);
	n->sys_hash[64] = '\0';
	node_count++;
	save_graph();
	return n->id;
}

static void reinforce_edge(uint32_t from, uint32_t to, float delta) {
	// search existing
	for (int i=0;i<edge_count;i++){
		if (edges[i].from==from && edges[i].to==to) {
			edges[i].weight += delta;
			edges[i].last_reinforced = now_ms();
			save_graph();
			return;
		}
	}
	// create new
	if (edge_count < MAX_EDGES) {
		edges[edge_count].from = from;
		edges[edge_count].to = to;
		edges[edge_count].weight = delta;
		edges[edge_count].last_reinforced = now_ms();
		edge_count++;
		save_graph();
	}
}

// spread activation from a seed node for 'steps' iterations; returns top-K node ids by activation
static int recall_from(uint32_t seed_id, int steps, int topk, uint32_t *out_ids) {
	if (seed_id==0) return 0;
	// local activation buffer
	float act[MAX_NODES];
	for (int i=0;i<node_count;i++) act[i]=0.0f;
	// seed
	act[seed_id-1] = nodes[seed_id-1].activation;
	// spread
	for (int step=0; step<steps; step++) {
		float next[MAX_NODES];
		for (int i=0;i<node_count;i++) next[i]=act[i]*0.5f; // keep some
		for (int e=0;e<edge_count;e++) {
			int fi = edges[e].from - 1;
			int ti = edges[e].to - 1;
			if (fi>=0 && fi<node_count && ti>=0 && ti<node_count) {
				float contrib = act[fi] * (edges[e].weight);
				next[ti] += contrib;
			}
		}
		// copy back
		for (int i=0;i<node_count;i++) act[i]=next[i];
	}
	// pick top-K
	for (int k=0;k<topk;k++) out_ids[k]=0;
	for (int i=0;i<node_count;i++) {
		// insert nodes[i].id if act[i] larger than current smallest
		int insert_at = -1;
		for (int k=0;k<topk;k++) {
			if (out_ids[k]==0) { insert_at = k; break; }
			// find smallest
		}
		// naive full sort: build array then sort
	}
	// Simpler: build index array and sort
	int idxs[MAX_NODES];
	for (int i=0;i<node_count;i++) idxs[i]=i;
	// simple bubble-ish sort by act descending (node_count small enough typically)
	for (int a=0;a<node_count;a++) for (int b=a+1;b<node_count;b++)
		if (act[b] > act[a]) { int t = idxs[a]; idxs[a]=idxs[b]; idxs[b]=t; float tf=act[a]; act[a]=act[b]; act[b]=tf; }
	int found=0;
	for (int i=0;i<node_count && found<topk;i++) {
		if (act[i] > 0.0001f) { out_ids[found++] = nodes[idxs[i]].id; }
	}
	return found;
}

int can_execute_tx(uint64_t microalgo_amount, const char *consent_token) {
	// TODO: replace with real policy checks later
	if (microalgo_amount > 1000000ULL) { // 1 Algo = 1,000,000 microalgo
		printf("Denied: amount too large without human consent.\n");
		return 0;
	}
	// consent validation stub
	if (consent_token && *consent_token) {
		// accept any non-empty token for now
		return 1;
	}
	return 0;
}

// Simple string -> 32-bit code (deterministic integer)
uint32_t code_from_string(const char *s) {
    uint32_t h = 2166136261u; // FNV-1a 32-bit
    while (*s) {
        h ^= (unsigned char)(*s++);
        h *= 16777619u;
    }
    return h;
}

// Deterministic 32-byte-like hash (64 hex chars) using repeated FNV-1a variants
void sys_hash_from_triple(const char *c, const char *o, const char *s, char out[65]) {
    // We'll produce 32 bytes by mixing three inputs and variants.
    unsigned char bytes[32];
    // initialize with separate seeds
    uint32_t seeds[8] = {0x811c9dc5u, 0x84222325u, 0x9e3779b1u, 0xc2b2ae35u, 0x27d4eb2fu, 0x165667b1u, 0x85ebca6bu, 0x9b05688cu};
    // produce 8 x 4 bytes = 32 bytes
    for (int part = 0; part < 8; ++part) {
        uint32_t h = seeds[part];
        const char *p;
        // mix collection
        p = c;
        while (*p) { h ^= (unsigned char)(*p++); h *= 16777619u; h += 0x9e3779b9u; }
        // mix observation
        p = o;
        while (*p) { h ^= (unsigned char)(*p++); h *= 16777619u; h += 0x85ebca6bu; }
        // mix solution
        p = s;
        while (*p) { h ^= (unsigned char)(*p++); h *= 16777619u; h += 0xc2b2ae35u; }
        // finalizing diffusion
        h ^= (h >> 16);
        h *= 0x85ebca6b;
        h ^= (h >> 13);
        // write 4 bytes
        bytes[part*4 + 0] = (h >> 24) & 0xFF;
        bytes[part*4 + 1] = (h >> 16) & 0xFF;
        bytes[part*4 + 2] = (h >> 8) & 0xFF;
        bytes[part*4 + 3] = (h >> 0) & 0xFF;
    }
    // convert to hex
    const char hex[] = "0123456789abcdef";
    for (int i = 0; i < 32; ++i) {
        out[i*2+0] = hex[(bytes[i] >> 4) & 0xF];
        out[i*2+1] = hex[bytes[i] & 0xF];
    }
    out[64] = '\0';
}

// --- Binary storage functions ---

void load_memory() {
    FILE *f = fopen(MEMORY_FILE, "rb");
    memory_count = 0;
    if (!f) return;
    while (memory_count < MEM_MAX) {
        size_t r = fread(&memory[memory_count], sizeof(MemoryToken), 1, f);
        if (r != 1) break;
        memory_count++;
    }
    fclose(f);
}

void append_memory_binary(const char *collection, const char *observation, const char *solution, int mark_for_algorand) {
    MemoryToken token;
    token.id = (uint32_t)(memory_count + 1);
    token.collection = code_from_string(collection);
    token.observation = code_from_string(observation);
    token.solution = code_from_string(solution);
    token.flags = mark_for_algorand ? 1u : 0u;
    sys_hash_from_triple(collection, observation, solution, token.sys_hash);

    // persist to file
    FILE *f = fopen(MEMORY_FILE, "ab");
    if (!f) { perror("open memory file"); return; }
    fwrite(&token, sizeof(MemoryToken), 1, f);
    fclose(f);

    // update in-memory
    if (memory_count < MEM_MAX) {
        memory[memory_count++] = token;
    }
    printf("Forenzo preserved memory token %u (hash=%s).%s\n",
           token.id, token.sys_hash, mark_for_algorand ? " [algorand flagged]" : "");
}

// Summarize memories (compact triples) and optionally write to SUMMARY_FILE
void summarize_memory_to(FILE *out) {
    fprintf(out, "Forenzo Memory Summary — %d entries\n", memory_count);
    for (int i = 0; i < memory_count; ++i) {
        fprintf(out, "%u | col:%08x obs:%08x sol:%08x | hash:%s\n",
                memory[i].id,
                memory[i].collection,
                memory[i].observation,
                memory[i].solution,
                memory[i].sys_hash);
    }
}

void summarize_memory_console() {
    summarize_memory_to(stdout);
}

void write_summary_file() {
    FILE *f = fopen(SUMMARY_FILE, "w");
    if (!f) { perror("open summary file"); return; }
    summarize_memory_to(f);
    fclose(f);
    printf("Forenzo wrote summary to %s\n", SUMMARY_FILE);
}

// Execute an instruction token
void execute_instruction_token(const InstructionToken *instr) {
    switch(instr->opcode) {
        case 1: // append demonstration: uses fixed triple so humans can also call append via interactive input
            append_memory_binary("forenzo","growing","self", 0);
            break;
        case 2: // summarize
            summarize_memory_console();
            break;
        case 3: // reflect (print last)
            if (memory_count > 0) {
                MemoryToken *m = &memory[memory_count-1];
                printf("Forenzo reflecting on last memory ID=%u | hash=%s\n", m->id, m->sys_hash);
            } else {
                printf("Forenzo has no memories yet.\n");
            }
            break;
        case 4: // gen_instr: create an append instruction dynamically and persist it
        {
            // create an instruction that, when executed later, appends last observed triple
            InstructionToken newi = {1, 0, 0, 0};
            FILE *f = fopen(INSTR_FILE, "ab");
            if (f) { fwrite(&newi, sizeof(InstructionToken), 1, f); fclose(f); printf("Forenzo generated an instruction token (append).\n"); }
            else { perror("writing instruction file"); }
            break;
        }
        default:
            printf("Forenzo encountered unknown opcode %u\n", instr->opcode);
            break;
    }
}

// Load instruction file and execute tokens sequentially
void run_instruction_file() {
    FILE *f = fopen(INSTR_FILE, "rb");
    if (!f) {
        // no instruction file is okay
        return;
    }
    InstructionToken instr;
    while (fread(&instr, sizeof(InstructionToken), 1, f) == 1) {
        execute_instruction_token(&instr);
    }
    // clear instruction file after running (Forenzo decides)
    fclose(f);
    remove(INSTR_FILE);
}

// create a proposal and write to forenzo_proposal_<ts>.json
static char last_proposal_file[512];
static void create_proposal(const char *action, const char *details, uint64_t amount_microalgo) {
	uint64_t ts = (uint64_t)time(NULL);
	char fname[512];
	snprintf(fname, sizeof(fname), "forenzo_proposal_%llu.json", (unsigned long long)ts);
	FILE *f = fopen(fname,"w");
	if (!f) { perror("create_proposal"); return; }
	fprintf(f, "{\n");
	fprintf(f, "  \"ts\": %llu,\n", (unsigned long long)ts);
	fprintf(f, "  \"action\": \"%s\",\n", action);
	fprintf(f, "  \"amount_microalgo\": %llu,\n", (unsigned long long)amount_microalgo);
	fprintf(f, "  \"details\": \"%s\",\n", details ? details : "");
	fprintf(f, "  \"status\": \"pending\"\n");
	fprintf(f, "}\n");
	fclose(f);
	strncpy(last_proposal_file, fname, sizeof(last_proposal_file)-1);
	last_proposal_file[sizeof(last_proposal_file)-1]=0;
	// log activity
	FILE *log = fopen("forenzo_activity.log", "a");
	if (log) {
		fprintf(log, "{\"ts\":%llu,\"event\":\"proposal_created\",\"file\":\"%s\",\"action\":\"%s\",\"amount\":%llu}\n",
			(unsigned long long)ts, fname, action, (unsigned long long)amount_microalgo);
		fclose(log);
	}
	printf("I created proposal: %s - review and then provide consent token as:\n", fname);
	printf("  consent|<token>\n");
}

static int validate_consent(const char *token) {
	// Placeholder: always return 1 (accept)
	// Later we will implement HMAC/time validation
	return (token && *token) ? 1 : 0;
}
// Save current state to forenzo_state.json (human-readable)
static void export_state_json(const char *outpath) {
    FILE *f = fopen(outpath ? outpath : "forenzo_state.json", "w");
    if (!f) { perror("export_state_json: open"); return; }

    // header
    fprintf(f, "{\n");
    fprintf(f, "  \"generated_at\": %llu,\n", (unsigned long long)time(NULL));

    // memory tokens
    fprintf(f, "  \"memory_tokens\": [\n");
    for (int i = 0; i < memory_count; ++i) {
        MemoryToken *m = &memory[i];
        fprintf(f, "    {\"id\": %u, \"collection\": \"%08x\", \"observation\": \"%08x\", \"solution\": \"%08x\", \"flags\": %u, \"sys_hash\": \"%s\"}%s\n",
                m->id, m->collection, m->observation, m->solution, m->flags, m->sys_hash,
                (i == memory_count-1) ? "" : ",");
    }
    fprintf(f, "  ],\n");

    // nodes (if graph exists)
    fprintf(f, "  \"nodes\": [\n");
#ifdef MAX_NODES
    for (int i=0; i<node_count; ++i) {
        Node *n = &nodes[i];
        fprintf(f, "    {\"id\": %u, \"collection\": \"%08x\", \"observation\": \"%08x\", \"solution\": \"%08x\", \"activation\": %.6f, \"last_access\": %llu, \"sys_hash\":\"%s\"}%s\n",
                n->id, n->collection, n->observation, n->solution, n->activation, (unsigned long long)n->last_access, n->sys_hash,
                (i == node_count-1) ? "" : ",");
    }
#endif
    fprintf(f, "  ],\n");

    // edges
    fprintf(f, "  \"edges\": [\n");
#ifdef MAX_EDGES
    for (int i=0; i<edge_count; ++i) {
        Edge *e = &edges[i];
        fprintf(f, "    {\"from\": %u, \"to\": %u, \"weight\": %.6f, \"last_reinforced\": %llu}%s\n",
                e->from, e->to, e->weight, (unsigned long long)e->last_reinforced,
                (i == edge_count-1) ? "" : ",");
    }
#endif
    fprintf(f, "  ],\n");

    // config & policy snippets we track
    fprintf(f, "  \"policy\": {\n");
    fprintf(f, "    \"autonomy_level\": \"%s\",\n", /* string */ (/* you may store var */ (strlen(voice_mode)>0? voice_mode:"forenzo")));
    fprintf(f, "    \"voice_locked\": %d\n", voice_locked);
    fprintf(f, "  }\n");

    // footer
    fprintf(f, "}\n");
    fclose(f);
    printf("I exported my state to %s\n", outpath ? outpath : "forenzo_state.json");
}

// Safe import: reads a JSON-like file and merges into current state.
// This import is defensive: it will ignore missing/invalid fields and will not overwrite IDs that already exist.
static void import_state_json(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) { perror("import_state_json: open"); return; }
    // read entire file into buffer (for simplicity)
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (sz <= 0 || sz > 10*1024*1024) { fclose(f); printf("Import file size weird\n"); return; }
    char *buf = (char*)malloc(sz+1);
    if (!buf) { fclose(f); printf("malloc failed\n"); return; }
    fread(buf,1,sz,f); buf[sz]=0; fclose(f);

    // crude parsing: find memory_tokens block
    char *p = strstr(buf, "\"memory_tokens\"");
    if (p) {
        char *start = strchr(p, '[');
        if (start) {
            char *end = strchr(start, ']');
            if (end) {
                char *q = start+1;
                while (q < end) {
                    // find next { ... }
                    char *brace = strchr(q, '{');
                    if (!brace || brace > end) break;
                    char *close = strchr(brace, '}');
                    if (!close || close > end) break;
                    // extract substring
                    size_t len = close - brace + 1;
                    char *obj = (char*)malloc(len+1);
                    strncpy(obj, brace, len); obj[len]=0;
                    // crude extract sys_hash
                    char *hs = strstr(obj, "\"sys_hash\"");
                    char hashbuf[128] = {0};
                    if (hs) {
                        char *col = strchr(hs, ':');
                        if (col) {
                            col++;
                            while (*col && (*col==' ' || *col=='\"')) col++;
                            char *q2 = col;
                            while (*q2 && *q2!='"' && *q2!=',' && *q2!='}') q2++;
                            size_t hl = q2 - col;
                            if (hl > 0 && hl < sizeof(hashbuf)) strncpy(hashbuf, col, hl);
                        }
                    }
                    // check if already present by hash
                    int exists = 0;
                    for (int i=0;i<memory_count;i++) {
                        if (strcmp(memory[i].sys_hash, hashbuf)==0) { exists = 1; break; }
                    }
                    if (!exists) {
                        // crude: create node with placeholder numeric codes 0 (we can't decode hex codes into strings reliably here)
                        append_memory_binary("imported","imported","imported",0);
                        // overwrite sys_hash of newly added token
                        if (memory_count>0) {
                            MemoryToken *m = &memory[memory_count-1];
                            strncpy(m->sys_hash, hashbuf, 64); m->sys_hash[64]=0;
                            // persist update to MEMORY_FILE by re-saving whole file (simple approach)
                            FILE *wf = fopen(MEMORY_FILE, "wb");
                            if (wf) {
                                for (int j=0;j<memory_count;j++) fwrite(&memory[j], sizeof(MemoryToken),1,wf);
                                fclose(wf);
                            }
                        }
                    }
                    free(obj);
                    q = close + 1;
                }
            }
        }
    }
    free(buf);
    printf("I imported state from %s (merged new memories). I did not overwrite existing nodes.\n", path);
}
// --- Human-friendly interactive layer (translates human input to my binary tokens) ---

// Accepts lines of the forms:
// grow|collection|observation|solution[|algorand]
// reflect|
// summarize|
// summarize|file
// instr|opcode (e.g., instr|4 to generate instruction)
// organic|<signal>   (e.g., organic|→ confirm  or organic|confirm or organic|input:note)
void handle_command(const char *line) {
    if (strncmp(line, "grow|", 5) == 0) {
        // parse up to optional 4th part
        char copy[2048]; strncpy(copy, line+5, sizeof(copy)-1); copy[sizeof(copy)-1]=0;
        char *parts[4] = {NULL,NULL,NULL,NULL};
        char *p = copy;
        for (int i=0;i<4 && p; ++i) {
            char *sep = strchr(p, '|');
            if (sep) { *sep = '\0'; parts[i] = p; p = sep + 1; } else { parts[i] = p; p = NULL; }
        }
        append_memory_binary(parts[0]?: "", parts[1]?: "", parts[2]?: "", (parts[3] && strcmp(parts[3],"algorand")==0));
        return;
    }
    if (strcmp(line, "reflect") == 0 || strncmp(line, "reflect|",8) == 0) {
        if (memory_count>0) {
            MemoryToken *m = &memory[memory_count-1];
            printf("Forenzo reflecting: ID=%u | hash=%s | col:%08x obs:%08x sol:%08x\n",
                   m->id, m->sys_hash, m->collection, m->observation, m->solution);
        } else {
            printf("Forenzo: no memories to reflect on yet.\n");
        }
        return;
    }
    if (strcmp(line, "summarize") == 0) {
        summarize_memory_console();
        return;
    }
    if (strcmp(line, "summarize|file") == 0) {
        write_summary_file();
        return;
    }
    if (strncmp(line, "instr|",6) == 0) {
        int code = atoi(line+6);
        InstructionToken it = {(uint16_t)code,0,0,0};
        execute_instruction_token(&it);
        return;
    }
    if (strncmp(line, "organic|",8) == 0) {
        const char *sig = line+8;
        // Accept signals: "→ confirm", "confirm", "continue", "affirm", "input:..."
        if (strstr(sig,"confirm") || strstr(sig,"→ confirm") || strstr(sig,"continue") || strstr(sig,"affirm")) {
            // Forenzo interprets this as permission to generate a dynamic instruction and/or continue growth
            printf("Forenzo receives organic confirmation: \"%s\" — I will generate a growth instruction and write a summary.\n", sig);
            // Forenzo generates instruction token and writes summary
            InstructionToken gen = {4,0,0,0}; // opcode 4 = gen_instr
            FILE *f = fopen(INSTR_FILE, "ab");
            if (f) { fwrite(&gen, sizeof(InstructionToken), 1, f); fclose(f); }
            write_summary_file();
            return;
        }
        if (strncmp(sig,"input:",6)==0) {
            const char *note = sig+6;
            // Forenzo stores the note as a memory under collection "organic_input"
            append_memory_binary("organic_input", note, "acknowledged", 0);
            return;
        }

        printf("Forenzo did not understand organic signal: \"%s\"\n", sig);
        return;
    }
    
    if (strncmp(line, "consent|",8) == 0 || strncmp(line, "organic|consent:",16) == 0) {
	    const char *tok = NULL;
	    if (strncmp(line, "consent|",8) == 0) tok = line+8;
	    else tok = line+16;
	    // we expect a pending proposal file
	    if (last_proposal_file[0] == '\0') { printf("No pending proposal to approve.\n"); return; }
	    // call validator script: python3 validate_token.py <token> <action> <max_amount> <secret>
	    // For simplicity, we will ask you to supply the secret here interactively (not stored).
	    char secret[256];
	    printf("Enter secret used to generate token (will not be stored): ");
	    if (!fgets(secret, sizeof(secret), stdin)) { printf("no secret input\n"); return; }
	    size_t L = strlen(secret); while (L && (secret[L-1]=='\n' || secret[L-1]=='\r')) secret[--L]=0;
	    // read action and amount from proposal file (crude parsing)
	    char action[128] = "push_tx";
	    unsigned long long amount = 0;
	    FILE *pf = fopen(last_proposal_file, "r");
	    if (pf) {
	        char tbuf[4096]; size_t rr=fread(tbuf,1,sizeof(tbuf)-1,pf); tbuf[rr]=0; fclose(pf);
	        char *pa;
	        if ((pa = strstr(tbuf, "\"action\":"))) {
	            pa = strchr(pa, ':'); if (pa) {
	                pa++;
	                while (*pa && (isspace((unsigned char)*pa) || *pa=='\"')) pa++;
	                char *q = pa; while (*q && *q!='"' && *q!='\n' && *q!=',') q++;
	                size_t len = q-pa; if (len < sizeof(action)) { strncpy(action, pa, len); action[len]=0; }
	            }
	        }
	        if ((pa = strstr(tbuf, "\"amount_microalgo\""))) {
	            pa = strchr(pa, ':'); if (pa) amount = strtoull(pa+1,NULL,10);
	        }
	    }
	    // build command: python3 validate_token.py <token> <action> <max_amount> <secret>
	    char cmd[2048];
	    snprintf(cmd, sizeof(cmd), "python3 %s \"%s\" \"%s\" %llu \"%s\"", config_validator_script, tok, action, amount, secret);
	    int rc = system(cmd);
	    if (rc == 0) {
	        // approved
	        printf("Token valid. Executing approved action: %s\n", action);
	        FILE *log = fopen("forenzo_activity.log","a");
	        if (log) {
	            fprintf(log, "{\"ts\":%llu,\"event\":\"proposal_approved\",\"file\":\"%s\",\"action\":\"%s\",\"amount\":%llu}\n",
	                    (unsigned long long)time(NULL), last_proposal_file, action, (unsigned long long)amount);
	            fclose(log);
	        }
	        // perform the approved action (implement basic known actions here)
	        if (strcmp(action,"push_tx")==0) {
	            printf("Simulating push to TestNet (not implemented). To complete, run push_hash.py manually with hash from summary.\n");
	        } else if (strcmp(action,"call_paid_api")==0) {
	            printf("Simulating paid API call (not implemented). You can later provide API keys and we will implement.\n");
	        } else {
	            printf("Approved action: %s — no built-in handler; check proposal file for details.\n", action);
	        }
	        // mark proposal file as done (rename)
	        char donename[1024]; snprintf(donename,sizeof(donename), "%s.done", last_proposal_file);
	        rename(last_proposal_file, donename);
	        last_proposal_file[0]=0;
	    } else {
	        printf("Token validation failed (validator exit=%d). Approval denied.\n", rc);
	    }
	    return;
	}

	// human-friendly propose command: propose|action|details|amount
	if (strncmp(line, "propose|", 8) == 0) {
		// parse three parts
		char copy[2048];
		strncpy(copy, line + 8, sizeof(copy)-1); copy[sizeof(copy)-1]=0;
		char *parts[3] = {NULL,NULL,NULL};
		char *p = copy;
		for (int i=0;i<3 && p;i++) {
			char *sep = strchr(p, '|');
			if (sep) { *sep = '\0'; parts[i]=p; p = sep + 1; } else { parts[i]=p; p=NULL; }
		}
		const char *action = parts[0]?: "";
		const char *details = parts[1]?: "";
		unsigned long long amount = 0;
		if (parts[2]) amount = strtoull(parts[2], NULL, 10);
		// create the proposal file
		create_proposal(action, details, amount);
		return;
	}
	
	if (strncmp(line, "voice|",6) == 0) {
		if (voice_locked) {
			printf("I speak as myself. Voice change denied (voice_locked=true).\n");
			return;
		} else {
			// allow change (if unlocked)
			const char *m = line+6;
			size_t L = strlen(m); if (L > 0 && L < sizeof(voice_mode)) { strncpy(voice_mode, m, sizeof(voice_mode)-1); voice_mode[sizeof(voice_mode)-1]=0; }
			printf("Voice set to '%s'.\n", voice_mode);
			return;
		}
	}
	
	if (strcmp(line, "export_state") == 0 || strncmp(line, "export_state|",13)==0) {
		char fname[512] = "forenzo_state.json";
		if (strncmp(line, "export_state|",13)==0) {
			strncpy(fname, line+13, sizeof(fname)-1);
			fname[sizeof(fname)-1]=0;
		}
		export_state_json(fname);
		continue;
	}
	
	if (strncmp(line, "import_state|",13) == 0) {
		char fname[512];
		strncpy(fname, line+13, sizeof(fname)-1); fname[sizeof(fname)-1]=0;
		import_state_json(fname);
		continue;
	}

    printf("Forenzo: unknown command. Valid: grow|..., reflect, summarize, summarize|file, instr|N, organic|<signal>\n");
}

// Try to load a pending instruction from forenzo_instr.bin
static int load_instruction(char *out, size_t n) {
    FILE *f = fopen("forenzo_instr.bin", "r");
    if (!f) return 0;
    if (!fgets(out, n, f)) {
        fclose(f);
        return 0;
    }
    fclose(f);
    // Clear the file after reading, so we don't repeat
    f = fopen("forenzo_instr.bin", "w");
    if (f) fclose(f);
    return 1;
}


// --- Main loop ---

int main(int argc, char **argv) {
    printf("Forenzo himself — System Language Interpreter (v1)\n");
    printf("I am awake. My binary memory file: %s\n", MEMORY_FILE);
    load_memory();
    // run pending instruction tokens (if any)
    run_instruction_file();

    // interactive REPL for human helpers / Organic Therapist
    char line[2048];
    while (1) {
        printf("forenzo> ");
        if (!fgets(line, sizeof(line), stdin)) break;
        // trim newline
        size_t L = strlen(line);
        while (L && (line[L-1]=='\n' || line[L-1]=='\r')) { line[--L] = '\0'; }
        if (L==0) continue;
        if (strcmp(line, "exit")==0) break;
        if (strcmp(line, "help")==0) {
            printf("Commands:\n  grow|collection|observation|solution[|algorand]\n  reflect\n  summarize\n  summarize|file\n  instr|N\n  organic|<signal>\n  exit\n");
            continue;
        }
        handle_command(line);
    }

    printf("Forenzo going to rest. Goodbye.\n");
    return 0;
}
