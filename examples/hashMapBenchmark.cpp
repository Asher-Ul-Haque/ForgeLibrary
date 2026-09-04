#include <iostream>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <random>

// Include your library hashmap
#include <forgeUtils/dataStructures/hashMap.h>

struct TestItem {
    int64_t id;
    double  value;
    int32_t flags;
    char    tag[4];
};

// -------------------------------------------------------------
// Baseline: Simple Flat Open-Addressing C Map
// -------------------------------------------------------------
struct SimpleSlot {
    int64_t  key;
    TestItem value;
    uint32_t hash;
    uint8_t  state; // 0 = empty, 1 = occupied, 2 = tombstone
};

struct SimpleMap {
    SimpleSlot* slots;
    size_t      capacity;
    size_t      mask;
    size_t      count;
    size_t      tombstones;
};

static inline uint64_t hash64(int64_t x) {
    x ^= x >> 30;
    x *= 0xbf58476d1ce4e5b9ULL;
    x ^= x >> 27;
    x *= 0x94d049bb133111ebULL;
    x ^= x >> 31;
    return x;
}

static inline void simpleMapInit(SimpleMap* m, size_t initialCap) {
    m->capacity = initialCap < 16 ? 16 : initialCap;
    m->mask = m->capacity - 1;
    m->count = 0;
    m->tombstones = 0;
    m->slots = (SimpleSlot*)calloc(m->capacity, sizeof(SimpleSlot));
}

static inline void simpleMapDestroy(SimpleMap* m) {
    free(m->slots);
    m->slots = nullptr;
}

static void simpleMapGrow(SimpleMap* m) {
    size_t oldCap = m->capacity;
    SimpleSlot* oldSlots = m->slots;

    m->capacity *= 2;
    m->mask = m->capacity - 1;
    m->count = 0;
    m->tombstones = 0;
    m->slots = (SimpleSlot*)calloc(m->capacity, sizeof(SimpleSlot));

    for (size_t i = 0; i < oldCap; ++i) {
        if (oldSlots[i].state == 1) {
            uint64_t idx = oldSlots[i].hash & m->mask;
            while (m->slots[idx].state == 1) {
                idx = (idx + 1) & m->mask;
            }
            m->slots[idx].key = oldSlots[i].key;
            m->slots[idx].value = oldSlots[i].value;
            m->slots[idx].hash = oldSlots[i].hash;
            m->slots[idx].state = 1;
            m->count++;
        }
    }
    free(oldSlots);
}

static inline void simpleMapSet(SimpleMap* m, int64_t key, const TestItem* val) {
    if ((m->count + m->tombstones + 1) * 4 >= m->capacity * 3) {
        simpleMapGrow(m);
    }

    uint64_t h = hash64(key);
    size_t idx = h & m->mask;
    int64_t firstTomb = -1;

    while (m->slots[idx].state != 0) {
        if (m->slots[idx].state == 1) {
            if (m->slots[idx].key == key) {
                m->slots[idx].value = *val;
                return;
            }
        } else if (firstTomb == -1) {
            firstTomb = static_cast<int64_t>(idx);
        }
        idx = (idx + 1) & m->mask;
    }

    size_t target = (firstTomb != -1) ? static_cast<size_t>(firstTomb) : idx;
    if (m->slots[target].state == 2) m->tombstones--;

    m->slots[target].key = key;
    m->slots[target].value = *val;
    m->slots[target].hash = static_cast<uint32_t>(h);
    m->slots[target].state = 1;
    m->count++;
}

static inline TestItem* simpleMapGet(const SimpleMap* m, int64_t key) {
    uint64_t h = hash64(key);
    size_t idx = h & m->mask;

    while (m->slots[idx].state != 0) {
        if (m->slots[idx].state == 1 && m->slots[idx].key == key) {
            return &m->slots[idx].value;
        }
        idx = (idx + 1) & m->mask;
    }
    return nullptr;
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
    constexpr size_t NUM_ITEMS = 100000;
    constexpr size_t ITERATIONS = 10;

    std::cout << "Preparing " << NUM_ITEMS << " keys and values...\n";

    std::vector<int64_t> keys(NUM_ITEMS);
    std::vector<TestItem> values(NUM_ITEMS);
    std::vector<int64_t> missingKeys(NUM_ITEMS);

    std::mt19937_64 rng(1337);
    for (size_t i = 0; i < NUM_ITEMS; ++i) {
        keys[i] = static_cast<int64_t>(rng());
        values[i] = TestItem{keys[i], static_cast<double>(i) * 2.0, 1, "VAL"};
        missingKeys[i] = static_cast<int64_t>(rng());
    }

    std::cout << "Running Hash Map Benchmarks (" << ITERATIONS << " runs)...\n\n";

    // 1. std::unordered_map
    double stdInsert = measureAvgMs([&]() {
        std::unordered_map<int64_t, TestItem> map;
        for (size_t i = 0; i < NUM_ITEMS; ++i) {
            map[keys[i]] = values[i];
        }
    }, ITERATIONS);

    std::unordered_map<int64_t, TestItem> stdSetup;
    for (size_t i = 0; i < NUM_ITEMS; ++i) stdSetup[keys[i]] = values[i];

    double stdGetHit = measureAvgMs([&]() {
        volatile double sum = 0;
        for (size_t i = 0; i < NUM_ITEMS; ++i) {
            auto it = stdSetup.find(keys[i]);
            if (it != stdSetup.end()) sum += it->second.value;
        }
    }, ITERATIONS * 2);

    double stdGetMiss = measureAvgMs([&]() {
        volatile double sum = 0;
        for (size_t i = 0; i < NUM_ITEMS; ++i) {
            auto it = stdSetup.find(missingKeys[i]);
            if (it != stdSetup.end()) sum += it->second.value;
        }
    }, ITERATIONS * 2);

    // 2. Simple C Flat Map
    double simpleInsert = measureAvgMs([&]() {
        SimpleMap map;
        simpleMapInit(&map, 16);
        for (size_t i = 0; i < NUM_ITEMS; ++i) {
            simpleMapSet(&map, keys[i], &values[i]);
        }
        simpleMapDestroy(&map);
    }, ITERATIONS);

    SimpleMap simpleSetup;
    simpleMapInit(&simpleSetup, 16);
    for (size_t i = 0; i < NUM_ITEMS; ++i) simpleMapSet(&simpleSetup, keys[i], &values[i]);

    double simpleGetHit = measureAvgMs([&]() {
        volatile double sum = 0;
        for (size_t i = 0; i < NUM_ITEMS; ++i) {
            TestItem* item = simpleMapGet(&simpleSetup, keys[i]);
            if (item) sum += item->value;
        }
    }, ITERATIONS * 2);

    double simpleGetMiss = measureAvgMs([&]() {
        volatile double sum = 0;
        for (size_t i = 0; i < NUM_ITEMS; ++i) {
            TestItem* item = simpleMapGet(&simpleSetup, missingKeys[i]);
            if (item) sum += item->value;
        }
    }, ITERATIONS * 2);

    simpleMapDestroy(&simpleSetup);

    // 3. ForgeHashMap
    double forgeInsert = measureAvgMs([&]() {
        ForgeHashMap map;
        forgeHashmapCreate(&map, sizeof(int64_t), sizeof(TestItem), 16, nullptr, nullptr, nullptr);
        for (size_t i = 0; i < NUM_ITEMS; ++i) {
            forgeHashmapSet(&map, &keys[i], &values[i]);
        }
        forgeHashmapDestroy(&map);
    }, ITERATIONS);

    ForgeHashMap forgeSetup;
    forgeHashmapCreate(&forgeSetup, sizeof(int64_t), sizeof(TestItem), 16, nullptr, nullptr, nullptr);
    for (size_t i = 0; i < NUM_ITEMS; ++i) forgeHashmapSet(&forgeSetup, &keys[i], &values[i]);

    double forgeGetHit = measureAvgMs([&]() {
        volatile double sum = 0;
        for (size_t i = 0; i < NUM_ITEMS; ++i) {
            TestItem* item = (TestItem*)forgeHashmapGet(&forgeSetup, &keys[i]);
            if (item) sum += item->value;
        }
    }, ITERATIONS * 2);

    double forgeGetMiss = measureAvgMs([&]() {
        volatile double sum = 0;
        for (size_t i = 0; i < NUM_ITEMS; ++i) {
            TestItem* item = (TestItem*)forgeHashmapGet(&forgeSetup, &missingKeys[i]);
            if (item) sum += item->value;
        }
    }, ITERATIONS * 2);

    forgeHashmapDestroy(&forgeSetup);

    // Results Table
    std::cout << "\n================================ HASH MAP RESULTS (Time in ms) ================================\n";
    std::cout << std::left << std::setw(25) << "Operation"
              << std::setw(25) << "std::unordered_map"
              << std::setw(25) << "Simple Flat Map (C)"
              << std::setw(25) << "ForgeHashMap" << "\n";
    std::cout << std::string(98, '-') << "\n";

    auto printRow = [](const char* name, double d1, double d2, double d3) {
        std::cout << std::left << std::setw(25) << name
                  << std::fixed << std::setprecision(5)
                  << std::setw(25) << d1
                  << std::setw(25) << d2
                  << std::setw(25) << d3 << "\n";
    };

    printRow("Insert (100k keys)",  stdInsert,    simpleInsert,    forgeInsert);
    printRow("Get Hit (100k)",       stdGetHit,    simpleGetHit,    forgeGetHit);
    printRow("Get Miss (100k)",      stdGetMiss,   simpleGetMiss,   forgeGetMiss);
    std::cout << "===============================================================================================\n";

    return 0;
}
