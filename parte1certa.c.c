// ra2_main.c
// Implementação RA2 - cache de 10 textos, 100 arquivos no "disco".
// Algoritmos: FIFO, LRU, LFU. Modo interativo (número do texto), 0 sai, -1 -> simulação.
// Compile: gcc -O2 -std=c11 -o ra2_main ra2_main.c
// Rodar: ./ra2_main

#define _POSIX_C_SOURCE 200809L
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h> // usleep
#include <stdint.h>

#define NUM_TEXTS 100
#define CACHE_SIZE 10
#define MIN_TEXT_WORDS 1000
#define SIM_USERS 3
#define SIM_REQUESTS_PER_USER 200

// Simulated disk read delays (microseconds)
#define DISK_READ_US 120000     // 120 ms for "slow" disk
#define CACHE_READ_US 5000      // 5 ms for cache hit

typedef enum { ALGO_FIFO=0, ALGO_LRU=1, ALGO_LFU=2 } algo_t;

typedef struct {
    int id;             // text id 1..100
    char *content;      // loaded content (NULL if not loaded)
    uint64_t load_time_us; // time taken to load from "disk" in the last load
} text_entry_t;

/* Cache entry */
typedef struct cache_entry {
    int text_id;                // 1..100
    char *content;              // pointer to loaded text content (duplicated)
    int freq;                   // for LFU
    struct cache_entry *prev;   // for LRU doubly-linked list or FIFO queue
    struct cache_entry *next;
} cache_entry_t;

/* Cache base structure (used by all algorithms) */
typedef struct {
    cache_entry_t *head; // for LRU front (most recently used)
    cache_entry_t *tail; // for LRU back (least recently used)
    int size;            // current number entries
    cache_entry_t *map[NUM_TEXTS+1]; // lookup by text id (1..100)
    // For FIFO we use head/tail as queue (head = front older?)
} cache_t;

/* Stats collection */
typedef struct {
    uint64_t hits;
    uint64_t misses;
    uint64_t total_time_us; // sum of latencies
    uint64_t per_text_miss_count[NUM_TEXTS+1];
} stats_t;

/* Utility time functions */
static inline uint64_t now_us() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000ULL + ts.tv_nsec / 1000ULL;
}

/* Read text from disk path "./texts/<id>.txt".
   Returns malloc'd string (caller frees) or simulated content if file absent. */
char* read_text_from_disk(int id, uint64_t *elapsed_us_out) {
    char path[256];
    snprintf(path, sizeof(path), "texts/%d.txt", id);
    uint64_t t0 = now_us();
    FILE *f = fopen(path, "rb");
    if (!f) {
        // simulate slow read by sleeping and returning synthetic content
        usleep(DISK_READ_US);
        uint64_t t1 = now_us();
        *elapsed_us_out = t1 - t0;
        // create a simulated content string (not large)
        char *sim = malloc(256);
        snprintf(sim, 256, "SIMULATED CONTENT FOR TEXT %d (file not found)\n", id);
        return sim;
    }
    // file exists -> read fully
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = malloc(sz + 1);
    if (!buf) { fclose(f); *elapsed_us_out = now_us() - t0; return NULL;}
    fread(buf, 1, sz, f);
    buf[sz] = '\0';
    fclose(f);
    // simulate extra disk latency proportional to size (to emphasize cache benefits)
    usleep(20000 + (sz / 50)); // small base + size factor
    uint64_t t1 = now_us();
    *elapsed_us_out = t1 - t0;
    return buf;
}

/* Cache helpers */
void cache_init(cache_t *c) {
    c->head = c->tail = NULL;
    c->size = 0;
    for (int i=0;i<=NUM_TEXTS;i++) c->map[i] = NULL;
}

void cache_free(cache_t *c) {
    cache_entry_t *cur = c->head;
    while (cur) {
        cache_entry_t *n = cur->next;
        if (cur->content) free(cur->content);
        free(cur);
        cur = n;
    }
    c->head = c->tail = NULL; c->size = 0;
    for (int i=0;i<=NUM_TEXTS;i++) c->map[i]=NULL;
}

/* Insert at front (MRU) for LRU */
void cache_insert_front(cache_t *c, cache_entry_t *e) {
    e->prev = NULL;
    e->next = c->head;
    if (c->head) c->head->prev = e;
    c->head = e;
    if (!c->tail) c->tail = e;
    c->map[e->text_id] = e;
    c->size++;
}

/* Remove arbitrary entry from list */
void cache_remove(cache_t *c, cache_entry_t *e) {
    if (!e) return;
    if (e->prev) e->prev->next = e->next;
    else c->head = e->next;
    if (e->next) e->next->prev = e->prev;
    else c->tail = e->prev;
    c->map[e->text_id] = NULL;
    e->prev = e->next = NULL;
    c->size--;
}

/* Move entry to front (for LRU on access) */
void cache_move_to_front(cache_t *c, cache_entry_t *e) {
    if (c->head == e) return;
    cache_remove(c, e);
    cache_insert_front(c, e);
}

/* Evict according to algorithm */
cache_entry_t* cache_evict_fifo(cache_t *c) {
    // FIFO: evict tail (oldest inserted) if using insert at head; we will insert new at head
    if (!c->tail) return NULL;
    cache_entry_t *vict = c->tail;
    cache_remove(c, vict);
    return vict;
}

cache_entry_t* cache_evict_lru(cache_t *c) {
    // LRU: evict tail (least recently used)
    return cache_evict_fifo(c);
}

cache_entry_t* cache_evict_lfu(cache_t *c) {
    // LFU: find minimum freq entry; tie-breaker: least recently used (tailward)
    if (!c->head) return NULL;
    int minf = INT32_MAX;
    cache_entry_t *cur = c->head;
    while (cur) {
        if (cur->freq < minf) minf = cur->freq;
        cur = cur->next;
    }
    // find from tail to head the first with freq==minf (older)
    cur = c->tail;
    while (cur) {
        if (cur->freq == minf) {
            cache_remove(c, cur);
            return cur;
        }
        cur = cur->prev;
    }
    return NULL;
}

/* Access a text via a specified algorithm: returns content (malloc'd copy)
   and updates stats. If already in cache, returns pointer to cached content (duplicated),
   but we keep cache content inside cache_entry struct. */
char* cache_access(cache_t *c, int text_id, algo_t algo, stats_t *st) {
    uint64_t t0 = now_us();
    cache_entry_t *entry = c->map[text_id];
    if (entry) {
        // hit
        st->hits++;
        // emulate fast access
        usleep(CACHE_READ_US);
        // update policies
        if (algo == ALGO_LRU) {
            cache_move_to_front(c, entry);
        }
        if (algo == ALGO_LFU) {
            entry->freq++;
            cache_move_to_front(c, entry); // keep recency too
        }
        uint64_t t1 = now_us();
        st->total_time_us += (t1 - t0);
        // return a duplicate of content to caller (caller will free)
        if (entry->content) {
            char *dup = strdup(entry->content);
            return dup;
        } else {
            return NULL;
        }
    } else {
        // miss
        st->misses++;
        st->per_text_miss_count[text_id]++;
        // read from disk
        uint64_t disk_us = 0;
        char *content = read_text_from_disk(text_id, &disk_us);
        // insert into cache (may evict)
        if (c->size >= CACHE_SIZE) {
            cache_entry_t *vict = NULL;
            if (algo == ALGO_FIFO) vict = cache_evict_fifo(c);
            else if (algo == ALGO_LRU) vict = cache_evict_lru(c);
            else if (algo == ALGO_LFU) vict = cache_evict_lfu(c);
            if (vict) {
                if (vict->content) free(vict->content);
                free(vict);
            }
        }
        // create new entry and insert at front (head)
        cache_entry_t *ne = malloc(sizeof(cache_entry_t));
        ne->text_id = text_id;
        ne->content = content ? strdup(content) : NULL; // store a copy
        ne->freq = 1;
        ne->prev = ne->next = NULL;
        cache_insert_front(c, ne);
        uint64_t t1 = now_us();
        st->total_time_us += (t1 - t0);
        // return a duplicate to caller
        char *dup = content ? strdup(content) : NULL;
        // free original content read by read_text_from_disk (we keep cache->content copy)
        if (content) free(content);
        return dup;
    }
}

/* Print a short excerpt of content to terminal */
void show_content_excerpt(const char *content) {
    if (!content) { printf("[conteúdo vazio]\n"); return; }
    int max = 1000; // limit output
    int len = strlen(content);
    int toprint = (len < max) ? len : max;
    fwrite(content, 1, toprint, stdout);
    if (len > toprint) printf("\n... (content truncated)\n");
}

/* Reset stats */
void stats_init(stats_t *s) {
    s->hits = s->misses = 0;
    s->total_time_us = 0;
    for (int i=0;i<=NUM_TEXTS;i++) s->per_text_miss_count[i]=0;
}

/* Simulation distributions */

// uniform random 1..100
int pick_uniform() {
    return (rand() % NUM_TEXTS) + 1;
}

// Poisson-like: generate k ~ Poisson(lambda) then map to 1..100
// We'll use Knuth's algorithm with lambda = 30. Then mod 100 + 1
int pick_poisson() {
    const double lambda = 30.0;
    double L = exp(-lambda);
    int k = 0;
    double p = 1.0;
    do {
        k++;
        double u = ((double)rand() / RAND_MAX);
        p *= u;
    } while (p > L && k < 1000);
    // k-1 is the Poisson sample
    int sample = (k-1) % NUM_TEXTS;
    return sample + 1;
}

// Weighted: 43% chance pick uniformly among 30..40 (inclusive), else uniform among others
int pick_weighted_43_30_40() {
    double r = ((double)rand() / RAND_MAX);
    if (r < 0.43) {
        int range = 40 - 30 + 1;
        return 30 + (rand() % range);
    } else {
        // pick uniform from 1..100 excluding 30..40
        int val;
        while (1) {
            val = (rand() % NUM_TEXTS) + 1;
            if (val < 30 || val > 40) return val;
        }
    }
}

/* Run single simulation experiment for one algorithm and one distribution.
   user_count = SIM_USERS, each user makes SIM_REQUESTS_PER_USER requests.
   This function resets cache and stats, then returns stats. */
void run_simulation_experiment(algo_t algo, int (*picker)(), const char *dist_name, const char *algo_name) {
    printf("=== Simulação: algoritmo=%s  distribuição=%s ===\n", algo_name, dist_name);
    cache_t cache;
    cache_init(&cache);
    stats_t stats;
    stats_init(&stats);
    uint64_t global_start = now_us();
    for (int user = 0; user < SIM_USERS; ++user) {
        for (int req = 0; req < SIM_REQUESTS_PER_USER; ++req) {
            int tid = picker();
            char *content = cache_access(&cache, tid, algo, &stats);
            // "display" (we don't actually print large content in sim)
            if (content) {
                // small sleep to simulate user reading time (optional)
                // usleep(1000);
                free(content);
            }
        }
    }
    uint64_t global_end = now_us();
    double total_sec = (global_end - global_start) / 1e6;
    printf("Resultado: hits=%llu, misses=%llu, total_time=%.3f s, avg_latency_per_request=%.3f ms\n",
           (unsigned long long)stats.hits,
           (unsigned long long)stats.misses,
           total_sec,
           (double)stats.total_time_us / 1000.0 / (SIM_USERS * SIM_REQUESTS_PER_USER));
    // write per-text miss CSV for later plotting
    char csvname[128];
    snprintf(csvname, sizeof(csvname), "sim_%s_%s_per_text_misses.csv", algo_name, dist_name);
    FILE *cf = fopen(csvname, "w");
    if (cf) {
        fprintf(cf, "text_id,miss_count\n");
        for (int i = 1; i <= NUM_TEXTS; ++i) fprintf(cf, "%d,%llu\n", i, (unsigned long long)stats.per_text_miss_count[i]);
        fclose(cf);
        printf("Arquivo por-texto salvo: %s\n", csvname);
    } else {
        printf("Falha ao gravar CSV %s\n", csvname);
    }
    cache_free(&cache);
}

/* Top-level interactive loop */
void interactive_loop() {
    printf("RA2 - Leitor de Textos com Cache\n");
    printf("Digite o número do texto (1..100). 0 = sair. -1 = entrar em modo de simulação.\n");
    srand(time(NULL));
    cache_t cache_fifo, cache_lru, cache_lfu;
    // We'll allow user to choose algorithm at runtime: prompt for algorithm
    int cur_algo = -1;
    while (1) {
        if (cur_algo < 0) {
            printf("Escolha o algoritmo de cache para uso interativo:\n 0=FIFO  1=LRU  2=LFU\nDigite (0/1/2): ");
            if (scanf("%d", &cur_algo) != 1) { printf("Entrada inválida.\n"); return; }
            if (cur_algo < 0 || cur_algo > 2) { cur_algo = -1; continue; }
            // init cache
            cache_init(&cache_fifo); cache_init(&cache_lru); cache_init(&cache_lfu);
        }
        printf("\nDigite número do texto: ");
        int nid;
        if (scanf("%d", &nid) != 1) { printf("Entrada inválida.\n"); break; }
        if (nid == 0) { printf("Saindo...\n"); break; }
        if (nid == -1) {
            // run full simulation suite for choosing algorithm
            printf("\n== Entrando em modo de simulação completa ==\n");
            run_simulation_experiment(ALGO_FIFO, pick_uniform, "uniform", "FIFO");
            run_simulation_experiment(ALGO_FIFO, pick_poisson, "poisson", "FIFO");
            run_simulation_experiment(ALGO_FIFO, pick_weighted_43_30_40, "weighted30-40", "FIFO");
            run_simulation_experiment(ALGO_LRU, pick_uniform, "uniform", "LRU");
            run_simulation_experiment(ALGO_LRU, pick_poisson, "poisson", "LRU");
            run_simulation_experiment(ALGO_LRU, pick_weighted_43_30_40, "weighted30-40", "LRU");
            run_simulation_experiment(ALGO_LFU, pick_uniform, "uniform", "LFU");
            run_simulation_experiment(ALGO_LFU, pick_poisson, "poisson", "LFU");
            run_simulation_experiment(ALGO_LFU, pick_weighted_43_30_40, "weighted30-40", "LFU");
            printf("Simulações concluídas. Verifique os CSVs gerados.\n");
            cur_algo = -1; // ask algorithm again
            continue;
        }
        if (nid < 1 || nid > NUM_TEXTS) {
            printf("Número inválido. Deve ser 1..%d\n", NUM_TEXTS);
            continue;
        }
        // choose cache object depending on algorithm
        cache_t *cache = NULL;
        algo_t algo = ALGO_FIFO;
        if (cur_algo == 0) { cache = &cache_fifo; algo = ALGO_FIFO; }
        else if (cur_algo == 1) { cache = &cache_lru; algo = ALGO_LRU; }
        else if (cur_algo == 2) { cache = &cache_lfu; algo = ALGO_LFU; }

        static stats_t st_global;
        static int initialized = 0;
        if (!initialized) { stats_init(&st_global); initialized = 1; }

        char *content = cache_access(cache, nid, algo, &st_global);
        if (!content) { printf("[erro ao carregar conteúdo]\n"); }
        else {
            printf("\n--- Conteúdo do texto %d (trecho) ---\n", nid);
            show_content_excerpt(content);
            printf("\n--- Fim do trecho ---\n");
            free(content);
        }
        printf("[stats até agora] hits=%llu misses=%llu avg_latency_ms=%.3f\n",
               (unsigned long long)st_global.hits,
               (unsigned long long)st_global.misses,
               (double)st_global.total_time_us / 1000.0 / (st_global.hits + st_global.misses));
    }
    // cleanup
    cache_free(&cache_fifo); cache_free(&cache_lru); cache_free(&cache_lfu);
}

int main(int argc, char **argv) {
    printf("RA2 main - leitor com cache (C)\n");
    printf("Certifique-se de ter a pasta 'texts' com os arquivos 1.txt..100.txt.\n");
    interactive_loop();
    return 0;
}
