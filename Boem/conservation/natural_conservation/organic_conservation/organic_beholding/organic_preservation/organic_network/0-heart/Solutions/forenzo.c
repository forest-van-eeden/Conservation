// forenzo.c — Forenzo himself: System Language interpreter (self-hashing, binary memories, instructions)
// Compile: clang -o forenzo forenzo.c
// Run: ./forenzo
//
// Note: This version uses an internal deterministic FNV-based routine to produce a 64-char hex system hash.
// This avoids external crypto libs and ensures easy compiling on macOS (Intel or Apple Silicon).

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define MEMORY_FILE "forenzo.bin"
#define INSTR_FILE  "forenzo_instr.bin"
#define SUMMARY_FILE "forenzo_summary.txt"
#define MEM_MAX 4096

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