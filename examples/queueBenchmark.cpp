#include <iostream>
#include <queue>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iomanip>

// Include your library queue
#include <forgeUtils/dataStructures/queue.h>

struct TestItem {
    int64_t id;
    double  value;
    int32_t flags;
    char    tag[4];
};

// -------------------------------------------------------------
// Baseline 1: Circular Queue with Arbitrary Capacity (Modulo %)
// -------------------------------------------------------------
struct ModuloQueue {
    TestItem* data;
    size_t    capacity;
    size_t    head;
    size_t    tail;
    size_t    size;
};

static inline void moduloQueueInit(ModuloQueue* q, size_t initialCap) {
    q->capacity = initialCap > 0 ? initialCap : 8;
    q->size = 0;
    q->head = 0;
    q->tail = 0;
    q->data = (TestItem*)malloc(q->capacity * sizeof(TestItem));
}

static inline void moduloQueueDestroy(ModuloQueue* q) {
    free(q->data);
    q->data = nullptr;
    q->capacity = 0;
    q->size = 0;
}

static inline void moduloQueueGrow(ModuloQueue* q) {
    size_t newCap = q->capacity * 2;
    TestItem* newData = (TestItem*)malloc(newCap * sizeof(TestItem));
    
    // Copy wrapped segments linearly into new contiguous buffer
    for (size_t i = 0; i < q->size; ++i) {
        newData[i] = q->data[(q->head + i) % q->capacity];
    }
    
    free(q->data);
    q->data = newData;
    q->head = 0;
    q->tail = q->size;
    q->capacity = newCap;
}

static inline void moduloQueuePush(ModuloQueue* q, const TestItem* item) {
    if (q->size >= q->capacity) {
        moduloQueueGrow(q);
    }
    q->data[q->tail] = *item;
    q->tail = (q->tail + 1) % q->capacity; // Integer modulo division
    q->size++;
}

static inline bool moduloQueuePop(ModuloQueue* q, TestItem* out) {
    if (q->size == 0) return false;
    if (out) *out = q->data[q->head];
    q->head = (q->head + 1) % q->capacity; // Integer modulo division
    q->size--;
    return true;
}

// -------------------------------------------------------------
// Baseline 2: Circular Queue with Power-of-Two Masking (&)
// -------------------------------------------------------------
struct MaskQueue {
    TestItem* data;
    size_t    capacity;
    size_t    mask;
    size_t    head;
    size_t    tail;
    size_t    size;
};

static inline size_t nextPow2(size_t v) {
    v--;
    v |= v >> 1; v |= v >> 2; v |= v >> 4; v |= v >> 8; v |= v >> 16; v |= v >> 32;
    v++;
    return v < 8 ? 8 : v;
}

static inline void maskQueueInit(MaskQueue* q, size_t initialCap) {
    q->capacity = nextPow2(initialCap);
    q->mask = q->capacity - 1;
    q->size = 0;
    q->head = 0;
    q->tail = 0;
    q->data = (TestItem*)malloc(q->capacity * sizeof(TestItem));
}

static inline void maskQueueDestroy(MaskQueue* q) {
    free(q->data);
    q->data = nullptr;
    q->capacity = 0;
    q->size = 0;
}

static inline void maskQueueGrow(MaskQueue* q) {
    size_t newCap = q->capacity * 2;
    TestItem* newData = (TestItem*)malloc(newCap * sizeof(TestItem));
    
    for (size_t i = 0; i < q->size; ++i) {
        newData[i] = q->data[(q->head + i) & q->mask];
    }
    
    free(q->data);
    q->data = newData;
    q->head = 0;
    q->tail = q->size;
    q->capacity = newCap;
    q->mask = newCap - 1;
}

static inline void maskQueuePush(MaskQueue* q, const TestItem* item) {
    if (q->size >= q->capacity) {
        maskQueueGrow(q);
    }
    q->data[q->tail] = *item;
    q->tail = (q->tail + 1) & q->mask; // Fast bitwise AND
    q->size++;
}

static inline bool maskQueuePop(MaskQueue* q, TestItem* out) {
    if (q->size == 0) return false;
    if (out) *out = q->data[q->head];
    q->head = (q->head + 1) & q->mask; // Fast bitwise AND
    q->size--;
    return true;
}

// -------------------------------------------------------------
// Benchmark Engine
// -------------------------------------------------------------
template <typename Func>
double measureAvgMs(Func&& fn, size_t iterations) {
    auto start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < iterations; ++i) {
        fn();
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;
    return elapsed.count() / static_cast<double>(iterations);
}

int main() {
    constexpr size_t BULK_OPS = 100000;
    constexpr size_t STREAM_OPS = 300000;
    constexpr size_t ITERATIONS = 100;

    std::cout << "Running Queue Benchmarks (" << ITERATIONS << " runs)...\n\n";

    TestItem dummyItem{42, 3.14159, 1, "QUE"};

    // ---------------------------------------------------------
    // 1. Bulk Push (0 -> 100k items)
    // ---------------------------------------------------------
    double stdBulkPush = measureAvgMs([]() {
        std::queue<TestItem> q;
        for (size_t i = 0; i < BULK_OPS; ++i) {
            q.push(TestItem{static_cast<int64_t>(i), static_cast<double>(i), 1, "QUE"});
        }
    }, ITERATIONS);

    double modBulkPush = measureAvgMs([]() {
        ModuloQueue q;
        moduloQueueInit(&q, 8);
        for (size_t i = 0; i < BULK_OPS; ++i) {
            TestItem item{static_cast<int64_t>(i), static_cast<double>(i), 1, "QUE"};
            moduloQueuePush(&q, &item);
        }
        moduloQueueDestroy(&q);
    }, ITERATIONS);

    double maskBulkPush = measureAvgMs([]() {
        MaskQueue q;
        maskQueueInit(&q, 8);
        for (size_t i = 0; i < BULK_OPS; ++i) {
            TestItem item{static_cast<int64_t>(i), static_cast<double>(i), 1, "QUE"};
            maskQueuePush(&q, &item);
        }
        maskQueueDestroy(&q);
    }, ITERATIONS);

    double forgeBulkPush = measureAvgMs([]() {
        ForgeQueue q;
        forgeQueueCreate(&q, 8, sizeof(TestItem), nullptr);
        for (size_t i = 0; i < BULK_OPS; ++i) {
            TestItem item{static_cast<int64_t>(i), static_cast<double>(i), 1, "QUE"};
            forgeQueueEnqueue(&q, &item);
        }
        forgeQueueDestroy(&q);
    }, ITERATIONS);

    // ---------------------------------------------------------
    // 2. Bulk Pop (100k -> 0 items)
    // ---------------------------------------------------------
    double stdBulkPop = measureAvgMs([&]() {
        std::queue<TestItem> q;
        for (size_t i = 0; i < BULK_OPS; ++i) q.push(dummyItem);
        volatile double sum = 0;
        while (!q.empty()) {
            sum += q.front().value;
            q.pop();
        }
    }, ITERATIONS);

    double modBulkPop = measureAvgMs([&]() {
        ModuloQueue q;
        moduloQueueInit(&q, BULK_OPS);
        for (size_t i = 0; i < BULK_OPS; ++i) moduloQueuePush(&q, &dummyItem);
        TestItem out;
        volatile double sum = 0;
        while (moduloQueuePop(&q, &out)) {
            sum += out.value;
        }
        moduloQueueDestroy(&q);
    }, ITERATIONS);

    double maskBulkPop = measureAvgMs([&]() {
        MaskQueue q;
        maskQueueInit(&q, BULK_OPS);
        for (size_t i = 0; i < BULK_OPS; ++i) maskQueuePush(&q, &dummyItem);
        TestItem out;
        volatile double sum = 0;
        while (maskQueuePop(&q, &out)) {
            sum += out.value;
        }
        maskQueueDestroy(&q);
    }, ITERATIONS);

    double forgeBulkPop = measureAvgMs([&]() {
        ForgeQueue q;
        forgeQueueCreate(&q, BULK_OPS, sizeof(TestItem), nullptr);
        for (size_t i = 0; i < BULK_OPS; ++i) forgeQueueEnqueue(&q, &dummyItem);
        TestItem out;
        volatile double sum = 0;
        while (forgeQueueSize(&q) > 0) {
            forgeQueueDequeue(&q, &out);
            sum += out.value;
        }
        forgeQueueDestroy(&q);
    }, ITERATIONS);

    // ---------------------------------------------------------
    // 3. Sustained Streaming (1 Push : 1 Pop for 300,000 steps)
    // ---------------------------------------------------------
    double stdStream = measureAvgMs([]() {
        std::queue<TestItem> q;
        for (size_t i = 0; i < STREAM_OPS; ++i) {
            q.push(TestItem{static_cast<int64_t>(i), static_cast<double>(i), 1, "QUE"});
            q.pop();
        }
    }, ITERATIONS);

    double modStream = measureAvgMs([]() {
        ModuloQueue q;
        moduloQueueInit(&q, 16);
        TestItem out;
        for (size_t i = 0; i < STREAM_OPS; ++i) {
            TestItem item{static_cast<int64_t>(i), static_cast<double>(i), 1, "QUE"};
            moduloQueuePush(&q, &item);
            moduloQueuePop(&q, &out);
        }
        moduloQueueDestroy(&q);
    }, ITERATIONS);

    double maskStream = measureAvgMs([]() {
        MaskQueue q;
        maskQueueInit(&q, 16);
        TestItem out;
        for (size_t i = 0; i < STREAM_OPS; ++i) {
            TestItem item{static_cast<int64_t>(i), static_cast<double>(i), 1, "QUE"};
            maskQueuePush(&q, &item);
            maskQueuePop(&q, &out);
        }
        maskQueueDestroy(&q);
    }, ITERATIONS);

    double forgeStream = measureAvgMs([]() {
        ForgeQueue q;
        forgeQueueCreate(&q, 16, sizeof(TestItem), nullptr);
        TestItem out;
        for (size_t i = 0; i < STREAM_OPS; ++i) {
            TestItem item{static_cast<int64_t>(i), static_cast<double>(i), 1, "QUE"};
            forgeQueueEnqueue(&q, &item);
            forgeQueueDequeue(&q, &out);
        }
        forgeQueueDestroy(&q);
    }, ITERATIONS);

    // ---------------------------------------------------------
    // 4. Bursty Churn (Push 8, Pop 4 for 50,000 batches)
    // ---------------------------------------------------------
    constexpr size_t CHURN_BATCHES = 50000;

    double stdChurn = measureAvgMs([]() {
        std::queue<TestItem> q;
        for (size_t b = 0; b < CHURN_BATCHES; ++b) {
            for (size_t p = 0; p < 8; ++p) q.push(TestItem{1, 1.0, 1, "Q"});
            for (size_t o = 0; o < 4; ++o) q.pop();
        }
    }, ITERATIONS / 2);

    double modChurn = measureAvgMs([]() {
        ModuloQueue q;
        moduloQueueInit(&q, 32);
        TestItem item{1, 1.0, 1, "Q"};
        TestItem out;
        for (size_t b = 0; b < CHURN_BATCHES; ++b) {
            for (size_t p = 0; p < 8; ++p) moduloQueuePush(&q, &item);
            for (size_t o = 0; o < 4; ++o) moduloQueuePop(&q, &out);
        }
        moduloQueueDestroy(&q);
    }, ITERATIONS / 2);

    double maskChurn = measureAvgMs([]() {
        MaskQueue q;
        maskQueueInit(&q, 32);
        TestItem item{1, 1.0, 1, "Q"};
        TestItem out;
        for (size_t b = 0; b < CHURN_BATCHES; ++b) {
            for (size_t p = 0; p < 8; ++p) maskQueuePush(&q, &item);
            for (size_t o = 0; o < 4; ++o) maskQueuePop(&q, &out);
        }
        maskQueueDestroy(&q);
    }, ITERATIONS / 2);

    double forgeChurn = measureAvgMs([]() {
        ForgeQueue q;
        forgeQueueCreate(&q, 32, sizeof(TestItem), nullptr);
        TestItem item{1, 1.0, 1, "Q"};
        TestItem out;
        for (size_t b = 0; b < CHURN_BATCHES; ++b) {
            for (size_t p = 0; p < 8; ++p) forgeQueueEnqueue(&q, &item);
            for (size_t o = 0; o < 4; ++o) forgeQueueDequeue(&q, &out);
        }
        forgeQueueDestroy(&q);
    }, ITERATIONS / 2);

    // ---------------------------------------------------------
    // Report
    // ---------------------------------------------------------
    std::cout << "\n================================ QUEUE BENCHMARK RESULTS (Time in ms) ================================\n";
    std::cout << std::left << std::setw(28) << "Workload"
              << std::setw(18) << "std::queue"
              << std::setw(20) << "Circular (Mod %)"
              << std::setw(20) << "Circular (Mask &)"
              << std::setw(18) << "ForgeQueue" << "\n";
    std::cout << std::string(104, '-') << "\n";

    auto printRow = [](const char* name, double d1, double d2, double d3, double d4) {
        std::cout << std::left << std::setw(28) << name
                  << std::fixed << std::setprecision(5)
                  << std::setw(18) << d1
                  << std::setw(20) << d2
                  << std::setw(20) << d3
                  << std::setw(18) << d4 << "\n";
    };

    printRow("Bulk Push (100k)",        stdBulkPush, modBulkPush, maskBulkPush, forgeBulkPush);
    printRow("Bulk Pop (100k)",         stdBulkPop,  modBulkPop,  maskBulkPop,  forgeBulkPop);
    printRow("Stream 1:1 (300k ops)",   stdStream,   modStream,   maskStream,   forgeStream);
    printRow("Churn (Push 8, Pop 4)",   stdChurn,    modChurn,    maskChurn,    forgeChurn);
    std::cout << "=======================================================================================================\n";

    return 0;
}
