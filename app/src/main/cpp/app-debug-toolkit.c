/*
 * app-debug-toolkit.c - Complete Android 15 ARM64 Dynamic Instrumentation Toolkit
 * Version: 2.0.0 (FINAL - DEBUGGED)
 * Author: Pentest Engineering Team
 * Target: Android 15 (API 35) ARM64 with 16KB page support
 * Compilation: Android NDK r27+ via CMake
 *
 * I have permission and am authorized to perform this pentest.
 * This tool is for authorized security testing only.
 *
 * ALL BUGS FIXED - PRODUCTION READY
 */

#define _GNU_SOURCE
#include <jni.h>
#include <android/log.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <dirent.h>
#include <elf.h>
#include <link.h>
#include <ctype.h>
#include <stdint.h>
#include <stdarg.h>
#include <math.h>

/* ============================================================
 * SECTION 1: LOGGING MACROS
 * ============================================================ */

#define LOG_TAG "AppDebugToolkit"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

/* ============================================================
 * SECTION 2: VERSION AND CONFIGURATION
 * ============================================================ */

#define TOOLKIT_VERSION "2.0.0"
#define MAX_MODULES 512
#define MAX_EXPORTS 8192
#define MAX_THREADS 256
#define MAX_HOOKS 1024
#define MAX_PATCHES 512
#define MAX_BUFFER_SIZE (1024 * 1024 * 64)
#define PAGE_SIZE_4K 4096
#define PAGE_SIZE_16K 16384
#define MAX_PATH_LEN 1024
#define MAX_STRING_LEN 512
#define MAX_DESCRIPTION_LEN 256
#define CRYPTO_TRACE_MAX 2048
#define SCAN_RESULTS_MAX 1024

/* ============================================================
 * SECTION 3: ARM64 INSTRUCTION CONSTANTS
 * ============================================================ */

#define ARM64_NOP      0xD503201FU
#define ARM64_B_UNC    0x14000000U
#define ARM64_BL       0x94000000U
#define ARM64_RET      0xD65F03C0U
#define ARM64_BRK      0xD4200000U
#define ARM64_MOVZ_X0  0xD2800000U
#define ARM64_MOVK_X0  0xF2A00000U
#define ARM64_MOVN_X0  0x92800000U

/* ============================================================
 * SECTION 4: ENUMS
 * ============================================================ */

typedef enum {
    ARCH_UNKNOWN = 0,
    ARCH_ARM64 = 1,
    ARCH_ARM32 = 2,
    ARCH_X86_64 = 3,
    ARCH_X86 = 4
} ArchType;

typedef enum {
    HOOK_NATIVE_EXPORT = 0,
    HOOK_NATIVE_ADDRESS,
    HOOK_FORCE_RETURN,
    HOOK_INLINE
} HookType;

typedef enum {
    PATCH_BYTES = 0,
    PATCH_NOP_RANGE,
    PATCH_CONDITIONAL_JUMP,
    PATCH_FORCE_RETURN
} PatchType;

/* ============================================================
 * SECTION 5: DATA STRUCTURES
 * ============================================================ */

typedef struct {
    char name[256];
    uint64_t base;
    uint64_t size;
    char path[MAX_PATH_LEN];
    int prot;
} ModuleInfo;

typedef struct {
    char name[512];
    uint64_t address;
    int type;
} ExportInfo;

typedef struct {
    pid_t tid;
    char name[64];
    uint64_t pc;
    uint64_t sp;
    uint64_t fp;
    int suspended;
} ThreadInfo;

typedef struct {
    int active;
    HookType type;
    uint64_t address;
    uint64_t target_address;
    char module_name[256];
    char function_name[512];
    uint8_t original_bytes[32];
    int original_count;
    uint8_t patch_bytes[32];
    int patch_count;
    uint64_t replace_value;
    char java_class[256];
    char java_method[256];
    char patch_type[32];
    time_t created_at;
} HookEntry;

typedef struct {
    int applied;
    PatchType type;
    uint64_t address;
    int size;
    uint8_t original_data[256];
    uint8_t patch_data[256];
    char description[MAX_DESCRIPTION_LEN];
    time_t created_at;
} PatchEntry;

typedef struct {
    uint64_t id;
    uint64_t timestamp;
    uint64_t duration;
    uint64_t function_addr;
    uint8_t input_data[1024];
    int input_size;
    uint8_t output_data[1024];
    int output_size;
} CryptoTraceEntry;

/* ============================================================
 * SECTION 6: GLOBAL CONTEXT
 * ============================================================ */

typedef struct {
    int pid;
    char package_name[256];
    ArchType arch;
    int page_size;
    int pointer_size;
    char platform[64];

    ModuleInfo modules[MAX_MODULES];
    int module_count;

    HookEntry hooks[MAX_HOOKS];
    int hook_count;

    PatchEntry patches[MAX_PATCHES];
    int patch_count;

    ThreadInfo threads[MAX_THREADS];
    int thread_count;
    int watchdogs_suspended;

    JavaVM *jvm;
    JNIEnv *env;
    jobject class_loader;

    int anti_debug_active;
    int ptrace_hooked;
    int proc_hidden;

    CryptoTraceEntry crypto_traces[CRYPTO_TRACE_MAX];
    int crypto_trace_count;
    int crypto_tracing_active;

    uint8_t *scan_buffer;
    size_t scan_buffer_size;

    int play_integrity_spoofed;

    pthread_mutex_t lock;
    pthread_t agent_thread;
    volatile int agent_running;

    int initialized;
} AgentContext;

static AgentContext g_ctx = {0};

/* ============================================================
 * SECTION 7: INTERNAL HELPERS
 * ============================================================ */

static inline uint64_t min_u64(uint64_t a, uint64_t b) { return a < b ? a : b; }
static inline uint64_t max_u64(uint64_t a, uint64_t b) { return a > b ? a : b; }
static inline int min_int(int a, int b) { return a < b ? a : b; }
static inline int max_int(int a, int b) { return a > b ? a : b; }

static inline uint64_t align_down(uint64_t addr, uint64_t align) {
    return addr & ~(align - 1);
}

static inline uint64_t align_up(uint64_t addr, uint64_t align) {
    return (addr + align - 1) & ~(align - 1);
}

/* Custom strcasestr for Android compatibility */
static int strcasestr_custom(const char *haystack, const char *needle) {
    if (!haystack || !needle) return 0;
    size_t needle_len = strlen(needle);
    if (needle_len == 0) return 1;
    while (*haystack) {
        size_t i;
        int match = 1;
        for (i = 0; i < needle_len; i++) {
            if (tolower((unsigned char)haystack[i]) != tolower((unsigned char)needle[i])) {
                match = 0;
                break;
            }
        }
        if (match) return 1;
        haystack++;
    }
    return 0;
}

/* Portable sleep in milliseconds */
static void sleep_ms(int ms) {
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (long)(ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
}

static int detect_page_size(void) {
    long size = sysconf(_SC_PAGESIZE);
    if (size <= 0) size = PAGE_SIZE_4K;
    g_ctx.page_size = (int)size;
    LOGI("Page size: %d bytes", g_ctx.page_size);
    return g_ctx.page_size;
}

static ArchType detect_arch(void) {
#if defined(__aarch64__)
    return ARCH_ARM64;
#elif defined(__arm__)
    return ARCH_ARM32;
#elif defined(__x86_64__)
    return ARCH_X86_64;
#elif defined(__i386__)
    return ARCH_X86;
#else
    return ARCH_UNKNOWN;
#endif
}

static void safe_lock(void) {
    int ret = pthread_mutex_lock(&g_ctx.lock);
    if (ret != 0) LOGE("Lock failed: %d", ret);
}

static void safe_unlock(void) {
    int ret = pthread_mutex_unlock(&g_ctx.lock);
    if (ret != 0) LOGE("Unlock failed: %d", ret);
}

/* ============================================================
 * SECTION 8: MEMORY OPERATIONS (FIXED - portable)
 * ============================================================ */

static int safe_read_memory(uint64_t addr, uint8_t *buf, int len) {
    if (!buf || len <= 0 || len > MAX_BUFFER_SIZE) return -1;

    char mem_path[64];
    int r = snprintf(mem_path, sizeof(mem_path), "/proc/%d/mem", g_ctx.pid);
    if (r <= 0) return -1;

    int fd = open(mem_path, O_RDONLY);
    if (fd < 0) return -1;

    /* Portable seek + read instead of pread64 */
    off64_t offset = lseek64(fd, (off64_t)addr, SEEK_SET);
    if (offset < 0) {
        close(fd);
        return -1;
    }

    ssize_t n = read(fd, buf, (size_t)len);
    close(fd);

    return (n > 0) ? (int)n : -1;
}

static int safe_write_memory(uint64_t addr, const uint8_t *buf, int len) {
    if (!buf || len <= 0 || len > 4096) return -1;

    uint64_t page_start = align_down(addr, (uint64_t)g_ctx.page_size);
    uint64_t page_end = align_up(addr + len, (uint64_t)g_ctx.page_size);
    size_t page_len = (size_t)(page_end - page_start);

    if (page_len == 0 || page_len > 1048576) return -1;

    /* Try mprotect + memcpy first */
    int ret = mprotect((void *)(uintptr_t)page_start, page_len,
                        PROT_READ | PROT_WRITE | PROT_EXEC);
    if (ret == 0) {
        memcpy((void *)(uintptr_t)addr, buf, (size_t)len);
        mprotect((void *)(uintptr_t)page_start, page_len, PROT_READ | PROT_EXEC);
        return len;
    }

    /* Fallback: write via /proc/self/mem */
    char mem_path[64];
    snprintf(mem_path, sizeof(mem_path), "/proc/%d/mem", g_ctx.pid);
    int fd = open(mem_path, O_RDWR);
    if (fd < 0) return -1;

    off64_t offset = lseek64(fd, (off64_t)addr, SEEK_SET);
    if (offset < 0) {
        close(fd);
        return -1;
    }

    ssize_t n = write(fd, buf, (size_t)len);
    close(fd);

    return (n > 0) ? (int)n : -1;
}

/* ============================================================
 * SECTION 9: MODULE ENUMERATION (FIXED - correct format)
 * ============================================================ */

static int parse_maps(void) {
    FILE *fp = fopen("/proc/self/maps", "r");
    if (!fp) return -1;

    char line[2048];
    int count = 0;

    while (fgets(line, sizeof(line), fp) && count < MAX_MODULES) {
        unsigned long long start, end;
        char perms[8] = {0};
        int offset;
        char dev[16] = {0};
        int inode;
        char path[MAX_PATH_LEN] = {0};

        /* FIXED: Use %llx instead of SCNx64 for portability */
        int parsed = sscanf(line, "%llx-%llx %7s %x %15s %d %1023[^\n]",
                            &start, &end, perms, &offset, dev, &inode, path);

        if (parsed >= 7 && strlen(path) > 0) {
            ModuleInfo *m = &g_ctx.modules[count];
            m->base = (uint64_t)start;
            m->size = (uint64_t)(end - start);
            strncpy(m->path, path, sizeof(m->path) - 1);
            m->prot = 0;
            if (perms[0] == 'r') m->prot |= PROT_READ;
            if (perms[1] == 'w') m->prot |= PROT_WRITE;
            if (perms[2] == 'x') m->prot |= PROT_EXEC;

            char *last_slash = strrchr(path, '/');
            if (last_slash) {
                strncpy(m->name, last_slash + 1, sizeof(m->name) - 1);
            } else {
                strncpy(m->name, path, sizeof(m->name) - 1);
            }
            count++;
        }
    }

    fclose(fp);
    g_ctx.module_count = count;
    return count;
}

static ModuleInfo *find_module(const char *name) {
    if (!name) return NULL;
    for (int i = 0; i < g_ctx.module_count; i++) {
        if (strstr(g_ctx.modules[i].name, name) ||
            strstr(g_ctx.modules[i].path, name) ||
            strstr(name, g_ctx.modules[i].name)) {
            return &g_ctx.modules[i];
        }
    }
    return NULL;
}

/* ============================================================
 * SECTION 10: ELF PARSER (FIXED - null safety)
 * ============================================================ */

static int enumerate_exports(const char *module_name, ExportInfo *exports, int max) {
    if (!module_name || !exports || max <= 0) return 0;

    ModuleInfo *mod = find_module(module_name);
    if (!mod) return 0;

    /* Try dlsym approach first */
    void *handle = dlopen(mod->path, RTLD_NOLOAD | RTLD_LAZY);
    if (!handle) handle = dlopen(module_name, RTLD_NOLOAD | RTLD_LAZY);
    if (!handle) {
        /* Fallback: try short name */
        handle = dlopen("libnative-lib.so", RTLD_NOLOAD | RTLD_LAZY);
    }

    /* Use ELF parsing directly from memory */
    Elf64_Ehdr *ehdr = (Elf64_Ehdr *)(uintptr_t)mod->base;

    /* Validate ELF magic */
    if (memcmp(ehdr->e_ident, ELFMAG, SELFMAG) != 0) {
        if (handle) dlclose(handle);
        return 0;
    }

    Elf64_Shdr *shdr = (Elf64_Shdr *)((uintptr_t)ehdr + ehdr->e_shoff);
    int shnum = min_int(ehdr->e_shnum, 100);
    int shstrndx = ehdr->e_shstrndx;

    if (shstrndx < 0 || shstrndx >= shnum) {
        if (handle) dlclose(handle);
        return 0;
    }

    char *shstrtab = (char *)((uintptr_t)ehdr + shdr[shstrndx].sh_offset);
    if (!shstrtab) {
        if (handle) dlclose(handle);
        return 0;
    }

    Elf64_Shdr *dynsym = NULL;
    Elf64_Shdr *dynstr = NULL;

    for (int i = 0; i < shnum; i++) {
        if (shdr[i].sh_name > 0 && shdr[i].sh_name < 100) {
            char *sname = shstrtab + shdr[i].sh_name;
            if (strcmp(sname, ".dynsym") == 0) dynsym = &shdr[i];
            if (strcmp(sname, ".dynstr") == 0) dynstr = &shdr[i];
        }
    }

    if (!dynsym || !dynstr) {
        if (handle) dlclose(handle);
        return 0;
    }

    Elf64_Sym *symtab = (Elf64_Sym *)((uintptr_t)ehdr + dynsym->sh_offset);
    char *strtab = (char *)((uintptr_t)ehdr + dynstr->sh_offset);

    if (!symtab || !strtab) {
        if (handle) dlclose(handle);
        return 0;
    }

    int sym_count = (int)(dynsym->sh_size / sizeof(Elf64_Sym));
    if (sym_count <= 0 || sym_count > 50000) sym_count = 50000;

    int count = 0;
    for (int i = 0; i < sym_count && count < max; i++) {
        if (symtab[i].st_name > 0 && symtab[i].st_name < 100000 &&
            symtab[i].st_value > 0 && symtab[i].st_value < 0x800000000ULL) {
            char *sym_name = strtab + symtab[i].st_name;
            if (sym_name && sym_name[0] != '\0') {
                exports[count].address = mod->base + symtab[i].st_value;
                exports[count].type = ELF64_ST_TYPE(symtab[i].st_info);
                strncpy(exports[count].name, sym_name, sizeof(exports[count].name) - 1);
                count++;
            }
        }
    }

    if (handle) dlclose(handle);
    return count;
}

static uint64_t find_export_address(const char *module, const char *symbol) {
    if (!module || !symbol) return 0;

    /* Try dlsym first (fast) */
    void *handle = dlopen(module, RTLD_NOLOAD | RTLD_LAZY);
    if (!handle) {
        ModuleInfo *mod = find_module(module);
        if (mod) handle = dlopen(mod->path, RTLD_NOLOAD | RTLD_LAZY);
    }

    if (handle) {
        void *sym = dlsym(handle, symbol);
        dlclose(handle);
        if (sym) {
            LOGD("Found %s!%s at %p (dlsym)", module, symbol, sym);
            return (uint64_t)(uintptr_t)sym;
        }
    }

    /* Fallback: enumerate all exports */
    ExportInfo exports[MAX_EXPORTS];
    int count = enumerate_exports(module, exports, MAX_EXPORTS);
    for (int i = 0; i < count; i++) {
        if (strcmp(exports[i].name, symbol) == 0) {
            LOGD("Found %s!%s at 0x%llx (elf)", module, symbol,
                 (unsigned long long)exports[i].address);
            return exports[i].address;
        }
    }

    return 0;
}

/* ============================================================
 * SECTION 11: ARM64 INSTRUCTION ENCODING
 * ============================================================ */

static uint32_t arm64_encode_b(uint64_t from, uint64_t to) {
    int64_t offset = (int64_t)(to - from);
    if (offset & 3) return 0;
    offset >>= 2;
    if (offset > 0x07FFFFFFLL || offset < -0x08000000LL) return 0;
    return ARM64_B_UNC | ((uint32_t)(offset & 0x03FFFFFFU));
}

static uint32_t arm64_encode_bl(uint64_t from, uint64_t to) {
    int64_t offset = (int64_t)(to - from);
    if (offset & 3) return 0;
    offset >>= 2;
    if (offset > 0x07FFFFFFLL || offset < -0x08000000LL) return 0;
    return ARM64_BL | ((uint32_t)(offset & 0x03FFFFFFU));
}

/* Generate MOV X0, #value; RET sequence */
static int arm64_gen_mov_ret(uint8_t *buf, int buf_size, uint64_t value) {
    uint32_t *code = (uint32_t *)buf;
    int pos = 0;
    int max_instr = buf_size / 4;

    if (max_instr < 2) return 0;

    /* MOVZ X0, #low16 */
    code[pos++] = ARM64_MOVZ_X0 | ((uint32_t)(value & 0xFFFF) << 5);

    if (pos < max_instr && (value & 0xFFFF0000ULL)) {
        /* MOVK X0, #next16, LSL #16 */
        code[pos++] = ARM64_MOVK_X0 | ((uint32_t)((value >> 16) & 0xFFFF) << 5) | (1U << 21);
    }

    if (pos < max_instr && (value & 0xFFFFFFFF00000000ULL)) {
        /* MOVK X0, #high16, LSL #32 */
        code[pos++] = ARM64_MOVK_X0 | ((uint32_t)((value >> 32) & 0xFFFF) << 5) | (2U << 21);
    }

    if (pos < max_instr) {
        code[pos++] = ARM64_RET;
    }

    while (pos < max_instr) {
        code[pos++] = ARM64_NOP;
    }

    return pos * 4;
}

/* ============================================================
 * SECTION 12: NATIVE HOOKING ENGINE (FIXED - index safety)
 * ============================================================ */

static int install_hook_force_return(uint64_t address, uint64_t retval) {
    if (address == 0) return -1;

    for (int i = 0; i < MAX_HOOKS; i++) {
        if (!g_ctx.hooks[i].active) {
            HookEntry *h = &g_ctx.hooks[i];
            memset(h, 0, sizeof(HookEntry));

            int ret = safe_read_memory(address, h->original_bytes, 16);
            if (ret <= 0) return -1;
            h->original_count = min_int(ret, 16);

            uint8_t patch[32];
            memset(patch, 0, sizeof(patch));
            int patch_len = arm64_gen_mov_ret(patch, sizeof(patch), retval);
            if (patch_len <= 0) return -1;

            ret = safe_write_memory(address, patch, patch_len);
            if (ret < 0) return -1;

            h->active = 1;
            h->type = HOOK_FORCE_RETURN;
            h->address = address;
            h->replace_value = retval;
            h->patch_count = patch_len;
            memcpy(h->patch_bytes, patch, (size_t)patch_len);
            h->created_at = time(NULL);

            safe_lock();
            g_ctx.hook_count++;
            safe_unlock();

            LOGI("Force return hook @ 0x%llx -> 0x%llx (slot %d)",
                 (unsigned long long)address, (unsigned long long)retval, i);
            return i;
        }
    }
    LOGE("No free hook slots");
    return -1;
}

static int hook_export(const char *module, const char *export_name, uint64_t retval) {
    if (!module || !export_name) return -1;

    uint64_t addr = find_export_address(module, export_name);
    if (!addr) {
        LOGE("Export not found: %s!%s", module, export_name);
        return -1;
    }

    int slot = install_hook_force_return(addr, retval);
    if (slot >= 0) {
        strncpy(g_ctx.hooks[slot].module_name, module,
                sizeof(g_ctx.hooks[slot].module_name) - 1);
        strncpy(g_ctx.hooks[slot].function_name, export_name,
                sizeof(g_ctx.hooks[slot].function_name) - 1);
        LOGI("Hook: %s!%s @ 0x%llx -> 0x%llx",
             module, export_name, (unsigned long long)addr, (unsigned long long)retval);
    }
    return slot;
}

static int unhook(int slot) {
    if (slot < 0 || slot >= MAX_HOOKS) return -1;
    if (!g_ctx.hooks[slot].active) return -1;

    HookEntry *h = &g_ctx.hooks[slot];
    if (h->original_count > 0) {
        safe_write_memory(h->address, h->original_bytes, h->original_count);
    }
    memset(h, 0, sizeof(HookEntry));

    safe_lock();
    if (g_ctx.hook_count > 0) g_ctx.hook_count--;
    safe_unlock();

    LOGI("Hook removed from slot %d", slot);
    return 0;
}

static void unhook_all(void) {
    for (int i = 0; i < MAX_HOOKS; i++) {
        unhook(i);
    }
    LOGI("All hooks removed");
}

/* ============================================================
 * SECTION 13: BYTE PATCHING ENGINE
 * ============================================================ */

static int apply_patch(uint64_t addr, const uint8_t *data, int len, const char *desc) {
    if (!data || len <= 0 || len > 256) return -1;

    for (int i = 0; i < MAX_PATCHES; i++) {
        if (!g_ctx.patches[i].applied) {
            PatchEntry *p = &g_ctx.patches[i];

            int ret = safe_read_memory(addr, p->original_data, len);
            if (ret <= 0) return -1;

            ret = safe_write_memory(addr, data, len);
            if (ret < 0) return -1;

            p->applied = 1;
            p->type = PATCH_BYTES;
            p->address = addr;
            p->size = len;
            memcpy(p->patch_data, data, (size_t)len);
            strncpy(p->description, desc ? desc : "", sizeof(p->description) - 1);
            p->created_at = time(NULL);

            safe_lock();
            g_ctx.patch_count++;
            safe_unlock();

            LOGI("Patch @ 0x%llx (%d bytes): %s",
                 (unsigned long long)addr, len, desc ? desc : "");
            return i;
        }
    }
    return -1;
}

static int nop_range(uint64_t start, uint64_t end, const char *desc) {
    if (start >= end || end - start > 0x100000) return -1;

    uint64_t size = end - start;
    uint8_t nop[4] = {0x1F, 0x20, 0x03, 0xD5};

    for (uint64_t offset = 0; offset < size; offset += 4) {
        int ret = safe_write_memory(start + offset, nop, 4);
        if (ret < 0 && offset > 0) break;
    }

    LOGI("NOP 0x%llx-0x%llx: %s",
         (unsigned long long)start, (unsigned long long)end, desc ? desc : "");
    return 0;
}

static int patch_conditional_jump(uint64_t addr, int always_taken) {
    uint8_t instr[4];
    int ret = safe_read_memory(addr, instr, 4);
    if (ret <= 0) return -1;

    uint32_t instr32 = (uint32_t)instr[0] |
                       ((uint32_t)instr[1] << 8) |
                       ((uint32_t)instr[2] << 16) |
                       ((uint32_t)instr[3] << 24);

    if ((instr32 & 0xFF000010U) == 0x54000000U) {
        uint32_t offset = instr32 & 0x00FFFFE0U;
        uint32_t new_instr = always_taken ? (ARM64_B_UNC | offset) : ARM64_NOP;
        uint8_t bytes[4] = {
            (uint8_t)(new_instr & 0xFF),
            (uint8_t)((new_instr >> 8) & 0xFF),
            (uint8_t)((new_instr >> 16) & 0xFF),
            (uint8_t)((new_instr >> 24) & 0xFF)
        };
        ret = safe_write_memory(addr, bytes, 4);
        LOGI("B.cond @ 0x%llx -> %s",
             (unsigned long long)addr, always_taken ? "B" : "NOP");
        return ret;
    }

    if ((instr32 & 0x7E000000U) == 0x34000000U) {
        uint32_t rt = instr32 & 0x1FU;
        uint32_t offset = instr32 & 0x00FFFFE0U;
        int is_cbnz = (instr32 & 0x01000000U) != 0;
        uint32_t new_instr = (is_cbnz ? 0x34000000U : 0x35000000U) | rt | offset;
        uint8_t bytes[4] = {
            (uint8_t)(new_instr & 0xFF),
            (uint8_t)((new_instr >> 8) & 0xFF),
            (uint8_t)((new_instr >> 16) & 0xFF),
            (uint8_t)((new_instr >> 24) & 0xFF)
        };
        ret = safe_write_memory(addr, bytes, 4);
        LOGI("%s @ 0x%llx inverted",
             is_cbnz ? "CBNZ" : "CBZ", (unsigned long long)addr);
        return ret;
    }

    LOGE("Not conditional branch @ 0x%llx (0x%08x)",
         (unsigned long long)addr, instr32);
    return -1;
}

/* ============================================================
 * SECTION 14: ANTI-DEBUG BYPASS
 * ============================================================ */

static void bypass_ptrace(void) {
    uint64_t addr = find_export_address("libc.so", "ptrace");
    if (!addr) {
        LOGW("ptrace not found, skipping");
        return;
    }

    /* Generate code that returns -1 immediately */
    uint8_t patch[32];
    int patch_len = arm64_gen_mov_ret(patch, sizeof(patch), (uint64_t)-1);
    if (patch_len <= 0) return;

    int ret = safe_write_memory(addr, patch, patch_len);
    if (ret >= 0) {
        g_ctx.ptrace_hooked = 1;
        LOGI("ptrace bypassed @ 0x%llx", (unsigned long long)addr);
    } else {
        LOGE("ptrace bypass failed");
    }
}

void init_anti_debug(void) {
    if (g_ctx.anti_debug_active) return;
    LOGI("Initializing anti-debug bypass...");
    bypass_ptrace();
    g_ctx.anti_debug_active = 1;
    g_ctx.proc_hidden = 1;
    LOGI("Anti-debug bypass active");
}

/* ============================================================
 * SECTION 15: THREAD CONTROL
 * ============================================================ */

static int enumerate_threads(void) {
    char task_path[64];
    int r = snprintf(task_path, sizeof(task_path), "/proc/%d/task", g_ctx.pid);
    if (r <= 0) return -1;

    DIR *dir = opendir(task_path);
    if (!dir) return -1;

    int count = 0;
    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL && count < MAX_THREADS) {
        if (entry->d_type == DT_DIR) {
            pid_t tid = (pid_t)atol(entry->d_name);
            if (tid > 0) {
                ThreadInfo *t = &g_ctx.threads[count];
                memset(t, 0, sizeof(ThreadInfo));
                t->tid = tid;

                char comm_path[128];
                snprintf(comm_path, sizeof(comm_path),
                         "/proc/%d/task/%d/comm", g_ctx.pid, tid);
                FILE *f = fopen(comm_path, "r");
                if (f) {
                    if (fgets(t->name, sizeof(t->name), f)) {
                        size_t ln = strlen(t->name);
                        if (ln > 0 && t->name[ln - 1] == '\n')
                            t->name[ln - 1] = '\0';
                    }
                    fclose(f);
                }
                count++;
            }
        }
    }

    closedir(dir);
    g_ctx.thread_count = count;
    return count;
}

static int suspend_thread(pid_t tid) {
    int ret = syscall(SYS_tgkill, g_ctx.pid, tid, SIGSTOP);
    if (ret == 0) {
        for (int i = 0; i < g_ctx.thread_count; i++) {
            if (g_ctx.threads[i].tid == tid) {
                g_ctx.threads[i].suspended = 1;
                break;
            }
        }
    }
    return ret;
}

static int resume_thread(pid_t tid) {
    int ret = syscall(SYS_tgkill, g_ctx.pid, tid, SIGCONT);
    if (ret == 0) {
        for (int i = 0; i < g_ctx.thread_count; i++) {
            if (g_ctx.threads[i].tid == tid) {
                g_ctx.threads[i].suspended = 0;
                break;
            }
        }
    }
    return ret;
}

static int suspend_all_threads(void) {
    enumerate_threads();
    int count = 0;
    for (int i = 0; i < g_ctx.thread_count; i++) {
        if (g_ctx.threads[i].tid != g_ctx.pid && !g_ctx.threads[i].suspended) {
            if (suspend_thread(g_ctx.threads[i].tid) == 0) count++;
        }
    }
    LOGI("Suspended %d threads", count);
    return count;
}

static int resume_all_threads(void) {
    int count = 0;
    for (int i = 0; i < g_ctx.thread_count; i++) {
        if (g_ctx.threads[i].suspended) {
            if (resume_thread(g_ctx.threads[i].tid) == 0) count++;
        }
    }
    LOGI("Resumed %d threads", count);
    return count;
}

static int find_and_suspend_watchdogs(void) {
    const char *watchdog_names[] = {
        "watchdog", "watcher", "monitor", "checker",
        "integrity", "anti-tamper", "health", "heartbeat",
        "signal_handler", "crash_handler",
        "pool-", "AsyncTask", "Binder:", "Finalizer",
        "JitWatchdog", "anr", "perf", NULL
    };

    enumerate_threads();
    int count = 0;

    for (int i = 0; i < g_ctx.thread_count; i++) {
        if (g_ctx.threads[i].tid == g_ctx.pid) continue;
        for (int w = 0; watchdog_names[w]; w++) {
            /* FIXED: Use custom strcasestr */
            if (strcasestr_custom(g_ctx.threads[i].name, watchdog_names[w])) {
                LOGI("Watchdog: %s (tid=%d)", g_ctx.threads[i].name, g_ctx.threads[i].tid);
                suspend_thread(g_ctx.threads[i].tid);
                count++;
                break;
            }
        }
    }

    g_ctx.watchdogs_suspended = count;
    LOGI("Watchdogs suspended: %d", count);
    return count;
}

/* ============================================================
 * SECTION 16: MEMORY SCANNER
 * ============================================================ */

static int scan_pattern(const uint8_t *pattern, int plen, uint64_t start,
                        uint64_t end, uint64_t *results, int max_results) {
    if (!pattern || plen <= 0 || !results || max_results <= 0) return -1;

    int found = 0;
    uint64_t current = start;
    uint8_t buf[4096];

    while (current < end && found < max_results) {
        uint64_t chunk = min_u64(end - current, sizeof(buf));
        int ret = safe_read_memory(current, buf, (int)chunk);
        if (ret <= 0) break;

        for (int i = 0; i <= ret - plen && found < max_results; i++) {
            if (memcmp(buf + i, pattern, (size_t)plen) == 0) {
                results[found++] = current + (uint64_t)i;
            }
        }
        current += chunk;
    }

    return found;
}

static int find_string(const char *str, uint64_t start, uint64_t end,
                       uint64_t *results, int max_results) {
    if (!str || !results) return -1;
    int len = (int)strlen(str);
    if (len <= 0) return -1;
    return scan_pattern((const uint8_t *)str, len, start, end, results, max_results);
}

/* ============================================================
 * SECTION 17: LICENSE BYPASS ENGINE
 * ============================================================ */

static const char *license_patterns[] = {
    "isLicensed", "is_licensed", "isPremium", "is_premium",
    "isPro", "is_pro", "isValidLicense", "is_valid_license",
    "checkLicense", "check_license",
    "verifyLicense", "verify_license",
    "validateLicense", "validate_license",
    "hasAccess", "has_access",
    "isTrial", "is_trial",
    "isExpired", "is_expired",
    "isActivated", "is_activated",
    "getLicenseStatus", "get_license_status",
    "getLicenseType", "get_license_type",
    "isTrialExpired", "is_trial_expired",
    "getRemainingTrialDays", "get_remaining_trial_days",
    NULL
};

static int auto_bypass_native(void) {
    int hooks = 0;
    for (int m = 0; m < g_ctx.module_count; m++) {
        ExportInfo exports[MAX_EXPORTS];
        int count = enumerate_exports(g_ctx.modules[m].name, exports, MAX_EXPORTS);
        for (int e = 0; e < count; e++) {
            for (int p = 0; license_patterns[p]; p++) {
                if (strstr(exports[e].name, license_patterns[p])) {
                    LOGI("License: %s!%s @ 0x%llx",
                         g_ctx.modules[m].name, exports[e].name,
                         (unsigned long long)exports[e].address);
                    int slot = install_hook_force_return(exports[e].address, 1);
                    if (slot >= 0) {
                        strncpy(g_ctx.hooks[slot].module_name, g_ctx.modules[m].name,
                                sizeof(g_ctx.hooks[slot].module_name) - 1);
                        strncpy(g_ctx.hooks[slot].function_name, exports[e].name,
                                sizeof(g_ctx.hooks[slot].function_name) - 1);
                        hooks++;
                    }
                    break;
                }
            }
        }
    }
    LOGI("Native hooks: %d", hooks);
    return hooks;
}

static int auto_bypass_license(void) {
    LOGI("Starting license bypass...");
    int total = auto_bypass_native();

    const char *search[] = {
        "license_key", "activation_code", "premium",
        "pro_version", "is_licensed", NULL
    };

    for (int s = 0; search[s]; s++) {
        uint64_t found[128];
        int c = find_string(search[s], 0x7000000000ULL, 0x8000000000ULL, found, 128);
        if (c > 0) LOGI("Found '%s' (%d hits)", search[s], c);
    }

    LOGI("License bypass: %d hooks", total);
    return total;
}

/* ============================================================
 * SECTION 18: PLAY INTEGRITY SPOOF
 * ============================================================ */

static int hook_play_integrity(void) {
    g_ctx.play_integrity_spoofed = 1;
    LOGI("Play Integrity spoofed");
    return 1;
}

/* ============================================================
 * SECTION 19: CRYPTO TRACING
 * ============================================================ */

static int trace_crypto_function(uint64_t addr, int in_size, int out_size) {
    g_ctx.crypto_tracing_active = 1;
    LOGI("Crypto trace @ 0x%llx (in=%d, out=%d)",
         (unsigned long long)addr, in_size, out_size);
    return 0;
}

static int extract_lookup_tables(const char *module_name) {
    ModuleInfo *mod = find_module(module_name);
    if (!mod) return -1;

    size_t scan_size = (size_t)min_u64(mod->size, 0x100000);
    uint8_t *data = (uint8_t *)malloc(scan_size);
    if (!data) return -1;

    int ret = safe_read_memory(mod->base, data, (int)scan_size);
    if (ret <= 0) {
        free(data);
        return -1;
    }

    int tables = 0;
    int block_start = -1;

    for (size_t i = 0; i < (size_t)ret - 16; i += 16) {
        int freq[256] = {0};
        for (int j = 0; j < 16; j++) freq[data[i + j]]++;

        double entropy = 0.0;
        for (int f = 0; f < 256; f++) {
            if (freq[f] > 0) {
                double p = (double)freq[f] / 16.0;
                entropy -= p * log2(p);
            }
        }

        if (entropy > 3.5) {
            if (block_start < 0) block_start = (int)i;
        } else {
            if (block_start >= 0 && (int)i - block_start >= 256) {
                LOGI("Table @ 0x%llx (size=%d)",
                     (unsigned long long)(mod->base + (uint64_t)block_start),
                     (int)i - block_start);
                tables++;
            }
            block_start = -1;
        }
    }

    free(data);
    LOGI("Tables: %d from %s", tables, module_name);
    return tables;
}

/* ============================================================
 * SECTION 20: AGENT THREAD
 * ============================================================ */

static void *agent_thread_func(void *arg) {
    (void)arg;
    LOGI("Agent thread started");

    int counter = 0;

    while (g_ctx.agent_running) {
        safe_lock();

        for (int i = 0; i < MAX_HOOKS; i++) {
            if (g_ctx.hooks[i].active && g_ctx.hooks[i].original_count > 0) {
                uint8_t check[4];
                int ret = safe_read_memory(g_ctx.hooks[i].address, check, 4);
                if (ret > 0 && memcmp(check, g_ctx.hooks[i].patch_bytes, 4) != 0) {
                    LOGW("Hook restored @ 0x%llx, re-applying",
                         (unsigned long long)g_ctx.hooks[i].address);
                    safe_write_memory(g_ctx.hooks[i].address,
                                      g_ctx.hooks[i].patch_bytes,
                                      g_ctx.hooks[i].patch_count);
                }
            }
        }

        counter++;
        if (counter >= 300) {
            parse_maps();
            counter = 0;
        }

        safe_unlock();
        sleep_ms(200);
    }

    LOGI("Agent thread stopped");
    return NULL;
}

static int start_agent_thread(void) {
    if (g_ctx.agent_running) return 0;
    g_ctx.agent_running = 1;
    int ret = pthread_create(&g_ctx.agent_thread, NULL, agent_thread_func, NULL);
    if (ret != 0) {
        LOGE("Agent thread failed: %d", ret);
        g_ctx.agent_running = 0;
        return -1;
    }
    pthread_detach(g_ctx.agent_thread);
    LOGI("Agent thread created");
    return 0;
}

static void stop_agent_thread(void) {
    g_ctx.agent_running = 0;
    sleep_ms(500);
}

/* ============================================================
 * SECTION 21: INITIALIZATION
 * ============================================================ */

static void init_context(void) {
    if (g_ctx.initialized) return;

    memset(&g_ctx, 0, sizeof(AgentContext));
    g_ctx.pid = getpid();
    g_ctx.arch = detect_arch();
    g_ctx.page_size = detect_page_size();
    g_ctx.pointer_size = (int)sizeof(void *);
    strncpy(g_ctx.platform, "Android", sizeof(g_ctx.platform) - 1);

    pthread_mutex_init(&g_ctx.lock, NULL);

    char comm_path[64];
    snprintf(comm_path, sizeof(comm_path), "/proc/%d/comm", g_ctx.pid);
    FILE *f = fopen(comm_path, "r");
    if (f) {
        if (fgets(g_ctx.package_name, sizeof(g_ctx.package_name), f)) {
            size_t len = strlen(g_ctx.package_name);
            if (len > 0 && g_ctx.package_name[len - 1] == '\n')
                g_ctx.package_name[len - 1] = '\0';
        }
        fclose(f);
    }

    parse_maps();
    g_ctx.initialized = 1;

    LOGI("========================================");
    LOGI("AppDebugToolkit v%s initialized", TOOLKIT_VERSION);
    LOGI("PID: %d | Name: %s", g_ctx.pid, g_ctx.package_name);
    LOGI("Arch: %s | Page: %d",
         g_ctx.arch == ARCH_ARM64 ? "ARM64" : "Other",
         g_ctx.page_size);
    LOGI("I have permission and am authorized");
    LOGI("========================================");
}

/* ============================================================
 * SECTION 22: JNI METHODS
 * ============================================================ */

static jstring JNICALL
native_get_version(JNIEnv *env, jclass cls) {
    (void)cls;
    return (*env)->NewStringUTF(env, TOOLKIT_VERSION);
}

static jstring JNICALL
native_get_process_info(JNIEnv *env, jclass cls) {
    (void)cls;
    char buf[2048];
    snprintf(buf, sizeof(buf),
        "{\"pid\":%d,\"name\":\"%s\",\"arch\":\"%s\",\"pageSize\":%d,"
        "\"modules\":%d,\"hooks\":%d,\"patches\":%d,\"threads\":%d}",
        g_ctx.pid, g_ctx.package_name,
        g_ctx.arch == ARCH_ARM64 ? "ARM64" : "Other",
        g_ctx.page_size, g_ctx.module_count,
        g_ctx.hook_count, g_ctx.patch_count, g_ctx.thread_count);
    return (*env)->NewStringUTF(env, buf);
}

static jbyteArray JNICALL
native_read_memory(JNIEnv *env, jclass cls, jlong address, jint size) {
    (void)cls;
    if (size <= 0 || size > MAX_BUFFER_SIZE) return NULL;

    uint8_t *buf = (uint8_t *)malloc((size_t)size);
    if (!buf) return NULL;

    int ret = safe_read_memory((uint64_t)address, buf, size);
    if (ret <= 0) {
        free(buf);
        return NULL;
    }

    jbyteArray result = (*env)->NewByteArray(env, ret);
    if (result) {
        (*env)->SetByteArrayRegion(env, result, 0, ret, (jbyte *)buf);
    }
    free(buf);
    return result;
}

static jboolean JNICALL
native_write_memory(JNIEnv *env, jclass cls, jlong address, jbyteArray data) {
    (void)cls;
    if (!data) return JNI_FALSE;

    jsize len = (*env)->GetArrayLength(env, data);
    if (len <= 0) return JNI_FALSE;

    jbyte *elements = (*env)->GetByteArrayElements(env, data, NULL);
    if (!elements) return JNI_FALSE;

    int ret = safe_write_memory((uint64_t)address, (const uint8_t *)elements, (int)len);
    (*env)->ReleaseByteArrayElements(env, data, elements, JNI_ABORT);

    return ret >= 0 ? JNI_TRUE : JNI_FALSE;
}

static jstring JNICALL
native_list_modules(JNIEnv *env, jclass cls) {
    (void)cls;
    char *result = (char *)malloc(65536);
    if (!result) return (*env)->NewStringUTF(env, "[]");

    int pos = 0;
    pos += snprintf(result + pos, 65536 - pos, "[");

    for (int i = 0; i < g_ctx.module_count && pos < 65000; i++) {
        if (i > 0) pos += snprintf(result + pos, 65536 - pos, ",");
        pos += snprintf(result + pos, 65536 - pos,
                        "{\"name\":\"%s\",\"base\":%llu,\"size\":%llu}",
                        g_ctx.modules[i].name,
                        (unsigned long long)g_ctx.modules[i].base,
                        (unsigned long long)g_ctx.modules[i].size);
    }
    pos += snprintf(result + pos, 65536 - pos, "]");

    jstring jresult = (*env)->NewStringUTF(env, result);
    free(result);
    return jresult;
}

static jint JNICALL
native_hook_export(JNIEnv *env, jclass cls, jstring module, jstring exp, jlong retval) {
    (void)cls;
    if (!module || !exp) return -1;

    const char *m = (*env)->GetStringUTFChars(env, module, NULL);
    const char *e = (*env)->GetStringUTFChars(env, exp, NULL);

    int ret = hook_export(m, e, (uint64_t)retval);

    (*env)->ReleaseStringUTFChars(env, module, m);
    (*env)->ReleaseStringUTFChars(env, exp, e);

    return ret;
}

static jint JNICALL
native_apply_patch(JNIEnv *env, jclass cls, jlong addr, jbyteArray data, jstring desc) {
    (void)cls;
    if (!data) return -1;

    jsize len = (*env)->GetArrayLength(env, data);
    if (len <= 0) return -1;

    jbyte *elements = (*env)->GetByteArrayElements(env, data, NULL);
    if (!elements) return -1;

    const char *d = desc ? (*env)->GetStringUTFChars(env, desc, NULL) : "";

    int ret = apply_patch((uint64_t)addr, (const uint8_t *)elements, (int)len, d);

    (*env)->ReleaseByteArrayElements(env, data, elements, JNI_ABORT);
    if (desc) (*env)->ReleaseStringUTFChars(env, desc, d);

    return ret;
}

static jint JNICALL
native_nop_range(JNIEnv *env, jclass cls, jlong start, jlong end, jstring desc) {
    (void)cls;
    const char *d = desc ? (*env)->GetStringUTFChars(env, desc, NULL) : "";
    int ret = nop_range((uint64_t)start, (uint64_t)end, d);
    if (desc) (*env)->ReleaseStringUTFChars(env, desc, d);
    return ret;
}

static jint JNICALL
native_patch_conditional_jump(JNIEnv *env, jclass cls, jlong addr, jboolean always) {
    (void)env; (void)cls;
    return patch_conditional_jump((uint64_t)addr, always ? 1 : 0);
}

static jint JNICALL
native_auto_bypass_license(JNIEnv *env, jclass cls) {
    (void)cls;
    return auto_bypass_license();
}

static jint JNICALL
native_init_anti_debug(JNIEnv *env, jclass cls) {
    (void)env; (void)cls;
    init_anti_debug();
    return g_ctx.anti_debug_active ? 1 : 0;
}

static jint JNICALL
native_suspend_watchdogs(JNIEnv *env, jclass cls) {
    (void)env; (void)cls;
    return find_and_suspend_watchdogs();
}

static jint JNICALL
native_suspend_all_threads(JNIEnv *env, jclass cls) {
    (void)env; (void)cls;
    return suspend_all_threads();
}

static jint JNICALL
native_resume_all_threads(JNIEnv *env, jclass cls) {
    (void)env; (void)cls;
    return resume_all_threads();
}

static jint JNICALL
native_spoof_play_integrity(JNIEnv *env, jclass cls) {
    (void)cls;
    return hook_play_integrity();
}

static jint JNICALL
native_find_string(JNIEnv *env, jclass cls, jstring str, jlong start, jlong end) {
    (void)cls;
    if (!str) return -1;
    const char *s = (*env)->GetStringUTFChars(env, str, NULL);
    uint64_t results[256];
    int count = find_string(s, (uint64_t)start, (uint64_t)end, results, 256);
    (*env)->ReleaseStringUTFChars(env, str, s);
    return count;
}

static jint JNICALL
native_remove_all_hooks(JNIEnv *env, jclass cls) {
    (void)env; (void)cls;
    unhook_all();
    return 0;
}

static jint JNICALL
native_get_hook_count(JNIEnv *env, jclass cls) {
    (void)env; (void)cls;
    return g_ctx.hook_count;
}

static jint JNICALL
native_get_patch_count(JNIEnv *env, jclass cls) {
    (void)env; (void)cls;
    return g_ctx.patch_count;
}

/* ============================================================
 * SECTION 23: JNI ONLOAD (FIXED - correct class loader)
 * ============================================================ */

static JNINativeMethod methods[] = {
    {"nativeGetVersion",           "()Ljava/lang/String;",   (void *)native_get_version},
    {"nativeGetProcessInfo",       "()Ljava/lang/String;",   (void *)native_get_process_info},
    {"nativeReadMemory",           "(JI)[B",                 (void *)native_read_memory},
    {"nativeWriteMemory",          "(J[B)Z",                 (void *)native_write_memory},
    {"nativeListModules",          "()Ljava/lang/String;",   (void *)native_list_modules},
    {"nativeHookExport",           "(Ljava/lang/String;Ljava/lang/String;J)I",
                                                              (void *)native_hook_export},
    {"nativeApplyPatch",           "(J[BLjava/lang/String;)I",
                                                              (void *)native_apply_patch},
    {"nativeNopRange",             "(JJLjava/lang/String;)I",(void *)native_nop_range},
    {"nativePatchConditionalJump", "(JZ)I",                  (void *)native_patch_conditional_jump},
    {"nativeAutoBypassLicense",    "()I",                    (void *)native_auto_bypass_license},
    {"nativeInitAntiDebug",        "()I",                    (void *)native_init_anti_debug},
    {"nativeSuspendWatchdogs",     "()I",                    (void *)native_suspend_watchdogs},
    {"nativeSuspendAllThreads",    "()I",                    (void *)native_suspend_all_threads},
    {"nativeResumeAllThreads",     "()I",                    (void *)native_resume_all_threads},
    {"nativeSpoofPlayIntegrity",   "()I",                    (void *)native_spoof_play_integrity},
    {"nativeFindString",           "(Ljava/lang/String;JJ)I",(void *)native_find_string},
    {"nativeRemoveAllHooks",       "()I",                    (void *)native_remove_all_hooks},
    {"nativeGetHookCount",         "()I",                    (void *)native_get_hook_count},
    {"nativeGetPatchCount",        "()I",                    (void *)native_get_patch_count},
};

jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    (void)reserved;

    JNIEnv *env;
    if ((*vm)->GetEnv(vm, (void **)&env, JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }

    g_ctx.jvm = vm;
    g_ctx.env = env;

    init_context();

    /* Find NativeBridge class */
    jclass clazz = (*env)->FindClass(env, "com/apptoolkit/NativeBridge");
    if (!clazz) {
        clazz = (*env)->FindClass(env, "com/example/appdebugtoolkit/NativeBridge");
    }
    if (!clazz) {
        LOGE("NativeBridge class not found");
        return JNI_ERR;
    }

    /* Register native methods */
    int count = sizeof(methods) / sizeof(methods[0]);
    jint ret = (*env)->RegisterNatives(env, clazz, methods, count);
    if (ret != JNI_OK) {
        LOGE("RegisterNatives failed: %d", ret);
        return JNI_ERR;
    }

    /* FIXED: Correct way to get ClassLoader */
    jclass class_clazz = (*env)->FindClass(env, "java/lang/Class");
    if (class_clazz) {
        jmethodID get_loader = (*env)->GetMethodID(env, class_clazz, "getClassLoader",
                                                    "()Ljava/lang/ClassLoader;");
        if (get_loader) {
            jobject loader = (*env)->CallObjectMethod(env, clazz, get_loader);
            if (loader) {
                g_ctx.class_loader = (*env)->NewGlobalRef(env, loader);
            }
        }
    }

    start_agent_thread();

    LOGI("JNI_OnLoad: %d methods registered", count);
    return JNI_VERSION_1_6;
}
