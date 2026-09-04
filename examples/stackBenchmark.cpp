#include <iostream>
#include <stack>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iomanip>

// Include your library stack
#include <forgeUtils/dataStructures/stack.h>

struct TestItem {
    int64_t id;
    double  value;
    int32_t flags;
    char    tag[4];
};

// -------------------------------------------------------------
// Baseline: Simple Dedicated C Stack
// -------------------------------------------------------------
struct SimpleStack {
    TestItem* data;
    size_t    capacity;
    size_t    size;
};

static inline void simpleStackInit(SimpleStack* s, size_t initialCap) {
    s->capacity = initialCap > 0 ? initialCap : 8;
    s->size = 0;
    s->data = (TestItem*)malloc(s->capacity * sizeof(TestItem));
}

static inline void simpleStackDestroy(SimpleStack* s) {
    free(s->data);
    s->data = nullptr;
    s->capacity = 0;
    s->size = 0;
}

static inline void simpleStackPush(SimpleStack* s, const TestItem* item) {
    if (s->size >= s->capacity) {
        s->capacity = s->capacity ? s->capacity * 2 : 8;
        s->data = (TestItem*)realloc(s->data, s->capacity * sizeof(TestItem));
    }
    s->data[s->size++] = *item;
}

static inline bool simpleStackPop(SimpleStack* s, TestItem* out) {
    if (s->size == 0) return false;
    s->size--;
    if (out) *out = s->data[s->size];
    return true;
}

static inline TestItem* simpleStackPeek(const SimpleStack* s) {
    if (s->size == 0) return nullptr;
    return &s->data[s->size - 1];
}

// -------------------------------------------------------------
// Benchmark Engine
// -------------------------------------------------------------
static constexpr size_t NUM_OPERATIONS = 100000;
static constexpr size_t NUM_ITERATIONS = 20000;

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
    std::cout << "Running Stack Benchmarks (" << NUM_ITERATIONS 
              << " runs, " << NUM_OPERATIONS << " operations per test)...\n\n";

    TestItem dummyItem{101, 2.71828, 5, "STK"};

    // ---------------------------------------------------------
    // 1. std::stack (default deque-backed)
    // ---------------------------------------------------------
    double stdStackPush = measureAvgMs([]() {
        std::stack<TestItem> s;
        for (size_t i = 0; i < NUM_OPERATIONS; ++i) {
            s.push(TestItem{static_cast<int64_t>(i), static_cast<double>(i), 1, "STK"});
        }
    }, NUM_ITERATIONS);

    double stdStackPop = measureAvgMs([&]() {
        std::stack<TestItem> s;
        for (size_t i = 0; i < NUM_OPERATIONS; ++i) s.push(dummyItem);
        volatile double drain = 0;
        while (!s.empty()) {
            drain += s.top().value;
            s.pop();
        }
    }, NUM_ITERATIONS);

    double stdStackPeek = measureAvgMs([&]() {
        std::stack<TestItem> s;
        s.push(dummyItem);
        volatile double drain = 0;
        for (size_t i = 0; i < NUM_OPERATIONS; ++i) {
            drain += s.top().value;
        }
    }, NUM_ITERATIONS * 10);

    double stdStackInterleaved = measureAvgMs([]() {
        std::stack<TestItem> s;
        for (size_t i = 0; i < NUM_OPERATIONS; ++i) {
            s.push(TestItem{static_cast<int64_t>(i), static_cast<double>(i), 1, "STK"});
            if ((i & 1) == 0 && !s.empty()) {
                s.pop();
            }
        }
    }, NUM_ITERATIONS);

    // ---------------------------------------------------------
    // 2. std::vector as Stack (vector-backed)
    // ---------------------------------------------------------
    double stdVecPush = measureAvgMs([]() {
        std::vector<TestItem> s;
        for (size_t i = 0; i < NUM_OPERATIONS; ++i) {
            s.push_back(TestItem{static_cast<int64_t>(i), static_cast<double>(i), 1, "STK"});
        }
    }, NUM_ITERATIONS);

    double stdVecPop = measureAvgMs([&]() {
        std::vector<TestItem> s(NUM_OPERATIONS, dummyItem);
        volatile double drain = 0;
        while (!s.empty()) {
            drain += s.back().value;
            s.pop_back();
        }
    }, NUM_ITERATIONS);

    double stdVecPeek = measureAvgMs([&]() {
        std::vector<TestItem> s;
        s.push_back(dummyItem);
        volatile double drain = 0;
        for (size_t i = 0; i < NUM_OPERATIONS; ++i) {
            drain += s.back().value;
        }
    }, NUM_ITERATIONS * 10);

    double stdVecInterleaved = measureAvgMs([]() {
        std::vector<TestItem> s;
        for (size_t i = 0; i < NUM_OPERATIONS; ++i) {
            s.push_back(TestItem{static_cast<int64_t>(i), static_cast<double>(i), 1, "STK"});
            if ((i & 1) == 0 && !s.empty()) {
                s.pop_back();
            }
        }
    }, NUM_ITERATIONS);

    // ---------------------------------------------------------
    // 3. SimpleStack (Dedicated C Struct)
    // ---------------------------------------------------------
    double simplePush = measureAvgMs([]() {
        SimpleStack s;
        simpleStackInit(&s, 8);
        for (size_t i = 0; i < NUM_OPERATIONS; ++i) {
            TestItem item{static_cast<int64_t>(i), static_cast<double>(i), 1, "STK"};
            simpleStackPush(&s, &item);
        }
        simpleStackDestroy(&s);
    }, NUM_ITERATIONS);

    double simplePop = measureAvgMs([&]() {
        SimpleStack s;
        simpleStackInit(&s, NUM_OPERATIONS);
        for (size_t i = 0; i < NUM_OPERATIONS; ++i) simpleStackPush(&s, &dummyItem);
        
        TestItem out;
        volatile double drain = 0;
        while (simpleStackPop(&s, &out)) {
            drain += out.value;
        }
        simpleStackDestroy(&s);
    }, NUM_ITERATIONS);

    double simplePeek = measureAvgMs([&]() {
        SimpleStack s;
        simpleStackInit(&s, 8);
        simpleStackPush(&s, &dummyItem);
        volatile double drain = 0;
        for (size_t i = 0; i < NUM_OPERATIONS; ++i) {
            drain += simpleStackPeek(&s)->value;
        }
        simpleStackDestroy(&s);
    }, NUM_ITERATIONS * 10);

    double simpleInterleaved = measureAvgMs([]() {
        SimpleStack s;
        simpleStackInit(&s, 8);
        TestItem out;
        for (size_t i = 0; i < NUM_OPERATIONS; ++i) {
            TestItem item{static_cast<int64_t>(i), static_cast<double>(i), 1, "STK"};
            simpleStackPush(&s, &item);
            if ((i & 1) == 0) {
                simpleStackPop(&s, &out);
            }
        }
        simpleStackDestroy(&s);
    }, NUM_ITERATIONS);

    // ---------------------------------------------------------
    // 4. ForgeStack
    // ---------------------------------------------------------
    double forgePush = measureAvgMs([]() {
        ForgeStack s;
        forgeStackCreate(&s, 8, sizeof(TestItem), nullptr);
        for (size_t i = 0; i < NUM_OPERATIONS; ++i) {
            TestItem item{static_cast<int64_t>(i), static_cast<double>(i), 1, "STK"};
            forgeStackPush(&s, &item);
        }
        forgeStackDestroy(&s);
    }, NUM_ITERATIONS);

    double forgePop = measureAvgMs([&]() {
        ForgeStack s;
        forgeStackCreate(&s, NUM_OPERATIONS, sizeof(TestItem), nullptr);
        for (size_t i = 0; i < NUM_OPERATIONS; ++i) forgeStackPush(&s, &dummyItem);

        TestItem out;
        volatile double drain = 0;
        while (forgeStackPop(&s, &out)) {
            drain += out.value;
        }
        forgeStackDestroy(&s);
    }, NUM_ITERATIONS);

    double forgePeek = measureAvgMs([&]() {
        ForgeStack s;
        forgeStackCreate(&s, 8, sizeof(TestItem), nullptr);
        forgeStackPush(&s, &dummyItem);
        volatile double drain = 0;
        for (size_t i = 0; i < NUM_OPERATIONS; ++i) {
            drain += ((TestItem*)forgeStackPeek(&s))->value;
        }
        forgeStackDestroy(&s);
    }, NUM_ITERATIONS * 10);

    double forgeInterleaved = measureAvgMs([]() {
        ForgeStack s;
        forgeStackCreate(&s, 8, sizeof(TestItem), nullptr);
        TestItem out;
        for (size_t i = 0; i < NUM_OPERATIONS; ++i) {
            TestItem item{static_cast<int64_t>(i), static_cast<double>(i), 1, "STK"};
            forgeStackPush(&s, &item);
            if ((i & 1) == 0) {
                forgeStackPop(&s, &out);
            }
        }
        forgeStackDestroy(&s);
    }, NUM_ITERATIONS);

    // ---------------------------------------------------------
    // Print Results Table
    // ---------------------------------------------------------
    std::cout << "\n================================ STACK BENCHMARK RESULTS (Time in ms) ================================\n";
    std::cout << std::left << std::setw(25) << "Operation"
              << std::setw(20) << "std::stack (deque)"
              << std::setw(20) << "std::vector (stack)"
              << std::setw(20) << "SimpleStack (C)"
              << std::setw(20) << "ForgeStack" << "\n";
    std::cout << std::string(105, '-') << "\n";

    auto printRow = [](const char* op, double d1, double d2, double d3, double d4) {
        std::cout << std::left << std::setw(25) << op
                  << std::fixed << std::setprecision(5)
                  << std::setw(20) << d1
                  << std::setw(20) << d2
                  << std::setw(20) << d3
                  << std::setw(20) << d4 << "\n";
    };

    printRow("Push (100k)",        stdStackPush,        stdVecPush,        simplePush,        forgePush);
    printRow("Pop (100k)",         stdStackPop,         stdVecPop,         simplePop,         forgePop);
    printRow("Peek (100k)",        stdStackPeek,        stdVecPeek,        simplePeek,        forgePeek);
    printRow("Interleaved (100k)",   stdStackInterleaved, stdVecInterleaved, simpleInterleaved, forgeInterleaved);
    std::cout << "=======================================================================================================\n";

    return 0;
}
