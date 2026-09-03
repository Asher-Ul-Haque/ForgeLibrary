#include <iostream>
#include <vector>
#include <chrono>
#include <numeric>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <random>

// Include your library header
#include <forgeUtils/dataStructures/dynamicArray.h>

struct TestItem {
    int64_t id;
    double  value;
    int32_t flags;
    char    tag[4];
};

// -------------------------------------------------------------
// Baseline: Simple Dedicated C Dynamic Array
// -------------------------------------------------------------
struct SimpleArray {
    TestItem* data;
    size_t    capacity;
    size_t    size;
};

static inline void simpleArrayInit(SimpleArray* arr, size_t initialCap) {
    arr->capacity = initialCap > 0 ? initialCap : 8;
    arr->size = 0;
    arr->data = (TestItem*)malloc(arr->capacity * sizeof(TestItem));
}

static inline void simpleArrayDestroy(SimpleArray* arr) {
    free(arr->data);
    arr->data = nullptr;
    arr->capacity = 0;
    arr->size = 0;
}

static inline void simpleArrayReserve(SimpleArray* arr, size_t minCap) {
    if (minCap <= arr->capacity) return;
    arr->capacity = minCap;
    arr->data = (TestItem*)realloc(arr->data, arr->capacity * sizeof(TestItem));
}

static inline void simpleArrayPush(SimpleArray* arr, const TestItem* item) {
    if (arr->size >= arr->capacity) {
        arr->capacity = arr->capacity ? arr->capacity * 2 : 8;
        arr->data = (TestItem*)realloc(arr->data, arr->capacity * sizeof(TestItem));
    }
    arr->data[arr->size++] = *item;
}

static inline void simpleArrayPop(SimpleArray* arr, TestItem* out) {
    if (arr->size == 0) return;
    arr->size--;
    if (out) *out = arr->data[arr->size];
}

static inline void simpleArrayPushRange(SimpleArray* arr, const TestItem* items, size_t count) {
    if (arr->size + count > arr->capacity) {
        size_t newCap = arr->capacity ? arr->capacity : 8;
        while (newCap < arr->size + count) newCap *= 2;
        simpleArrayReserve(arr, newCap);
    }
    memcpy(arr->data + arr->size, items, count * sizeof(TestItem));
    arr->size += count;
}

static inline void simpleArrayShrinkToFit(SimpleArray* arr) {
    size_t targetCap = arr->size > 0 ? arr->size : 8;
    if (targetCap < arr->capacity) {
        arr->data = (TestItem*)realloc(arr->data, targetCap * sizeof(TestItem));
        arr->capacity = targetCap;
    }
}

// -------------------------------------------------------------
// Benchmark Engine
// -------------------------------------------------------------
static constexpr size_t NUM_ELEMENTS = 20000;
static constexpr size_t NUM_ITERATIONS = 5000;

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
    // Generate deterministic pseudo-random indices for read/write tests
    std::vector<size_t> randomIndices(NUM_ELEMENTS);
    std::iota(randomIndices.begin(), randomIndices.end(), 0);
    std::mt19937 g(1337);
    for (size_t i = NUM_ELEMENTS - 1; i > 0; --i) {
        std::uniform_int_distribution<size_t> d(0, i);
        std::swap(randomIndices[i], randomIndices[d(g)]);
    }

    TestItem dummyItem{42, 3.14159, 7, "VAL"};

    // Batch buffer for PushRange test
    std::vector<TestItem> batchData(NUM_ELEMENTS, dummyItem);

    // ---------------------------------------------------------
    // 1. std::vector<TestItem>
    // ---------------------------------------------------------
    double stdPushDynamic = measureAvgMs([]() {
        std::vector<TestItem> vec;
        for (size_t i = 0; i < NUM_ELEMENTS; ++i) {
            vec.push_back({static_cast<int64_t>(i), static_cast<double>(i), 1, "TST"});
        }
    }, NUM_ITERATIONS);

    double stdPushReserved = measureAvgMs([]() {
        std::vector<TestItem> vec;
        vec.reserve(NUM_ELEMENTS);
        for (size_t i = 0; i < NUM_ELEMENTS; ++i) {
            vec.push_back({static_cast<int64_t>(i), static_cast<double>(i), 1, "TST"});
        }
    }, NUM_ITERATIONS);

    double stdPushRange = measureAvgMs([&]() {
        std::vector<TestItem> vec;
        vec.insert(vec.end(), batchData.begin(), batchData.end());
    }, NUM_ITERATIONS);

    std::vector<TestItem> stdSetup(NUM_ELEMENTS, dummyItem);
    
    double stdSeqRead = measureAvgMs([&]() {
        volatile double sum = 0;
        for (size_t i = 0; i < NUM_ELEMENTS; ++i) {
            sum += stdSetup[i].value;
        }
    }, NUM_ITERATIONS * 10);

    double stdRandRead = measureAvgMs([&]() {
        volatile double sum = 0;
        for (size_t i = 0; i < NUM_ELEMENTS; ++i) {
            sum += stdSetup[randomIndices[i]].value;
        }
    }, NUM_ITERATIONS * 5);

    double stdRandWrite = measureAvgMs([&]() {
        for (size_t i = 0; i < NUM_ELEMENTS; ++i) {
            stdSetup[randomIndices[i]].value = static_cast<double>(i);
        }
    }, NUM_ITERATIONS * 5);

    // Fair Pop Benchmark (re-use buffer, avoid copy overhead inside timer)
    std::vector<TestItem> stdPopBuffer = stdSetup;
    double stdPop = measureAvgMs([&]() {
        TestItem out;
        while (!stdPopBuffer.empty()) {
            out = stdPopBuffer.back();
            stdPopBuffer.pop_back();
        }
        // Refill outside of timed measurement logic via direct resize
        stdPopBuffer.resize(NUM_ELEMENTS);
    }, NUM_ITERATIONS);

    double stdShrink = measureAvgMs([&]() {
        std::vector<TestItem> vec;
        vec.reserve(NUM_ELEMENTS * 2);
        vec.resize(NUM_ELEMENTS);
        vec.shrink_to_fit();
    }, NUM_ITERATIONS);

    // ---------------------------------------------------------
    // 2. SimpleArray (Dedicated Malloc/Free C Struct)
    // ---------------------------------------------------------
    double simplePushDynamic = measureAvgMs([]() {
        SimpleArray arr;
        simpleArrayInit(&arr, 8);
        for (size_t i = 0; i < NUM_ELEMENTS; ++i) {
            TestItem item{static_cast<int64_t>(i), static_cast<double>(i), 1, "TST"};
            simpleArrayPush(&arr, &item);
        }
        simpleArrayDestroy(&arr);
    }, NUM_ITERATIONS);

    double simplePushReserved = measureAvgMs([]() {
        SimpleArray arr;
        simpleArrayInit(&arr, NUM_ELEMENTS);
        for (size_t i = 0; i < NUM_ELEMENTS; ++i) {
            TestItem item{static_cast<int64_t>(i), static_cast<double>(i), 1, "TST"};
            simpleArrayPush(&arr, &item);
        }
        simpleArrayDestroy(&arr);
    }, NUM_ITERATIONS);

    double simplePushRange = measureAvgMs([&]() {
        SimpleArray arr;
        simpleArrayInit(&arr, 8);
        simpleArrayPushRange(&arr, batchData.data(), NUM_ELEMENTS);
        simpleArrayDestroy(&arr);
    }, NUM_ITERATIONS);

    SimpleArray simpleSetup;
    simpleArrayInit(&simpleSetup, NUM_ELEMENTS);
    for (size_t i = 0; i < NUM_ELEMENTS; ++i) simpleArrayPush(&simpleSetup, &dummyItem);

    double simpleSeqRead = measureAvgMs([&]() {
        volatile double sum = 0;
        for (size_t i = 0; i < NUM_ELEMENTS; ++i) {
            sum += simpleSetup.data[i].value;
        }
    }, NUM_ITERATIONS * 10);

    double simpleRandRead = measureAvgMs([&]() {
        volatile double sum = 0;
        for (size_t i = 0; i < NUM_ELEMENTS; ++i) {
            sum += simpleSetup.data[randomIndices[i]].value;
        }
    }, NUM_ITERATIONS * 5);

    double simpleRandWrite = measureAvgMs([&]() {
        for (size_t i = 0; i < NUM_ELEMENTS; ++i) {
            simpleSetup.data[randomIndices[i]].value = static_cast<double>(i);
        }
    }, NUM_ITERATIONS * 5);

    SimpleArray simplePopBuffer;
    simpleArrayInit(&simplePopBuffer, NUM_ELEMENTS);
    memcpy(simplePopBuffer.data, simpleSetup.data, NUM_ELEMENTS * sizeof(TestItem));
    simplePopBuffer.size = NUM_ELEMENTS;

    double simplePop = measureAvgMs([&]() {
        TestItem out;
        while (simplePopBuffer.size > 0) {
            simpleArrayPop(&simplePopBuffer, &out);
        }
        simplePopBuffer.size = NUM_ELEMENTS;
    }, NUM_ITERATIONS);

    simpleArrayDestroy(&simplePopBuffer);

    double simpleShrink = measureAvgMs([&]() {
        SimpleArray arr;
        simpleArrayInit(&arr, NUM_ELEMENTS * 2);
        arr.size = NUM_ELEMENTS;
        simpleArrayShrinkToFit(&arr);
        simpleArrayDestroy(&arr);
    }, NUM_ITERATIONS);

    simpleArrayDestroy(&simpleSetup);

    // ---------------------------------------------------------
    // 3. ForgeDynamicArray (Optimized API)
    // ---------------------------------------------------------
    double forgePushDynamic = measureAvgMs([]() {
        ForgeDynamicArray arr;
        FORGE_ARRAY_INIT(&arr, 8, TestItem);
        for (size_t i = 0; i < NUM_ELEMENTS; ++i) {
            TestItem item{static_cast<int64_t>(i), static_cast<double>(i), 1, "TST"};
            forgeDynamicArrayPush(&arr, &item);
        }
        forgeDynamicArrayDestroy(&arr);
    }, NUM_ITERATIONS);

    double forgeEmplaceDynamic = measureAvgMs([]() {
        ForgeDynamicArray arr;
        FORGE_ARRAY_INIT(&arr, 8, TestItem);
        for (size_t i = 0; i < NUM_ELEMENTS; ++i) {
            TestItem* item = FORGE_ARRAY_EMPLACE(&arr, TestItem);
            item->id = static_cast<int64_t>(i);
            item->value = static_cast<double>(i);
            item->flags = 1;
            memcpy(item->tag, "TST", 4);
        }
        forgeDynamicArrayDestroy(&arr);
    }, NUM_ITERATIONS);

    double forgePushReserved = measureAvgMs([]() {
        ForgeDynamicArray arr;
        FORGE_ARRAY_INIT(&arr, NUM_ELEMENTS, TestItem);
        for (size_t i = 0; i < NUM_ELEMENTS; ++i) {
            TestItem item{static_cast<int64_t>(i), static_cast<double>(i), 1, "TST"};
            forgeDynamicArrayPush(&arr, &item);
        }
        forgeDynamicArrayDestroy(&arr);
    }, NUM_ITERATIONS);

    double forgePushRange = measureAvgMs([&]() {
        ForgeDynamicArray arr;
        FORGE_ARRAY_INIT(&arr, 8, TestItem);
        FORGE_ARRAY_PUSH_RANGE(&arr, batchData.data(), NUM_ELEMENTS);
        forgeDynamicArrayDestroy(&arr);
    }, NUM_ITERATIONS);

    ForgeDynamicArray forgeSetup;
    FORGE_ARRAY_INIT(&forgeSetup, NUM_ELEMENTS, TestItem);
    for (size_t i = 0; i < NUM_ELEMENTS; ++i) forgeDynamicArrayPush(&forgeSetup, &dummyItem);

    // Fast direct pointer access using FORGE_ARRAY_DATA
    double forgeSeqRead = measureAvgMs([&]() {
        volatile double sum = 0;
        const TestItem* data = FORGE_ARRAY_DATA(&forgeSetup, TestItem);
        for (size_t i = 0; i < NUM_ELEMENTS; ++i) {
            sum += data[i].value;
        }
    }, NUM_ITERATIONS * 10);

    double forgeRandRead = measureAvgMs([&]() {
        volatile double sum = 0;
        const TestItem* data = FORGE_ARRAY_DATA(&forgeSetup, TestItem);
        for (size_t i = 0; i < NUM_ELEMENTS; ++i) {
            sum += data[randomIndices[i]].value;
        }
    }, NUM_ITERATIONS * 5);

    double forgeRandWrite = measureAvgMs([&]() {
        TestItem* data = FORGE_ARRAY_DATA(&forgeSetup, TestItem);
        for (size_t i = 0; i < NUM_ELEMENTS; ++i) {
            data[randomIndices[i]].value = static_cast<double>(i);
        }
    }, NUM_ITERATIONS * 5);

    ForgeDynamicArray forgePopBuffer;
    FORGE_ARRAY_INIT(&forgePopBuffer, NUM_ELEMENTS, TestItem);
    memcpy(forgePopBuffer.data, forgeSetup.data, NUM_ELEMENTS * sizeof(TestItem));
    forgePopBuffer.size = NUM_ELEMENTS;

    double forgePop = measureAvgMs([&]() {
        TestItem out;
        while (forgePopBuffer.size > 0) {
            forgeDynamicArrayPop(&forgePopBuffer, &out);
        }
        forgePopBuffer.size = NUM_ELEMENTS;
    }, NUM_ITERATIONS);

    forgeDynamicArrayDestroy(&forgePopBuffer);

    double forgeShrink = measureAvgMs([&]() {
        ForgeDynamicArray arr;
        FORGE_ARRAY_INIT(&arr, NUM_ELEMENTS * 2, TestItem);
        arr.size = NUM_ELEMENTS;
        forgeDynamicArrayShrinkToFit(&arr);
        forgeDynamicArrayDestroy(&arr);
    }, NUM_ITERATIONS);

    forgeDynamicArrayDestroy(&forgeSetup);

    // ---------------------------------------------------------
    // Reporting Results
    // ---------------------------------------------------------
    std::cout << "\n============================== BENCHMARK RESULTS (Time in ms) ==============================\n";
    std::cout << std::left << std::setw(28) << "Operation"
              << std::setw(20) << "std::vector"
              << std::setw(20) << "SimpleArray (C)"
              << std::setw(20) << "ForgeLibrary" << "\n";
    std::cout << std::string(88, '-') << "\n";

    auto printRow = [](const char* op, double stdT, double simT, double frgT) {
        std::cout << std::left << std::setw(28) << op
                  << std::fixed << std::setprecision(5)
                  << std::setw(20) << stdT
                  << std::setw(20) << simT
                  << std::setw(20) << frgT << "\n";
    };

    printRow("Push (Dynamic Growth)", stdPushDynamic, simplePushDynamic, forgePushDynamic);
    printRow("Emplace (Dynamic)",     stdPushDynamic, simplePushDynamic, forgeEmplaceDynamic);
    printRow("Push (Pre-Reserved)",   stdPushReserved, simplePushReserved, forgePushReserved);
    printRow("Push Range (Batch)",    stdPushRange, simplePushRange, forgePushRange);
    printRow("Sequential Read",       stdSeqRead, simpleSeqRead, forgeSeqRead);
    printRow("Random Read",           stdRandRead, simpleRandRead, forgeRandRead);
    printRow("Random Write",          stdRandWrite, simpleRandWrite, forgeRandWrite);
    printRow("Pop Back",              stdPop, simplePop, forgePop);
    printRow("Shrink To Fit",         stdShrink, simpleShrink, forgeShrink);
    std::cout << "=============================================================================================\n";

    return 0;
}
