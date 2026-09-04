#include <iostream>
#include <set>
#include <vector>
#include <chrono>
#include <numeric>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <random>
#include <algorithm>

// Include your library header
#include <forgeUtils/dataStructures/orderedSet.h>

struct TestItem {
    int64_t id;
    double  value;
};

struct CompareTestItem {
    bool operator()(const TestItem& a, const TestItem& b) const {
        return a.id < b.id;
    }
};

static int32_t compareTestItemC(const void* a, const void* b) {
    const TestItem* itemA = (const TestItem*)a;
    const TestItem* itemB = (const TestItem*)b;
    if (itemA->id < itemB->id) return -1;
    if (itemA->id > itemB->id) return 1;
    return 0;
}

// -------------------------------------------------------------
// Baseline: Simple Dedicated C AVL Tree
// -------------------------------------------------------------
struct SimpleAVLNode {
    TestItem key;
    int32_t  height;
    SimpleAVLNode* left;
    SimpleAVLNode* right;
};

static inline int32_t sHeight(SimpleAVLNode* n) { return n ? n->height : 0; }
static inline int32_t sMax(int32_t a, int32_t b) { return a > b ? a : b; }
static inline int32_t sBalance(SimpleAVLNode* n) { return n ? sHeight(n->left) - sHeight(n->right) : 0; }

static SimpleAVLNode* sRotateRight(SimpleAVLNode* y) {
    SimpleAVLNode* x = y->left;
    SimpleAVLNode* t2 = x->right;
    x->right = y;
    y->left = t2;
    y->height = sMax(sHeight(y->left), sHeight(y->right)) + 1;
    x->height = sMax(sHeight(x->left), sHeight(x->right)) + 1;
    return x;
}

static SimpleAVLNode* sRotateLeft(SimpleAVLNode* x) {
    SimpleAVLNode* y = x->right;
    SimpleAVLNode* t2 = y->left;
    y->left = x;
    x->right = t2;
    x->height = sMax(sHeight(x->left), sHeight(x->right)) + 1;
    y->height = sMax(sHeight(y->left), sHeight(y->right)) + 1;
    return y;
}

static SimpleAVLNode* sInsert(SimpleAVLNode* node, const TestItem* key, bool* inserted) {
    if (!node) {
        SimpleAVLNode* n = (SimpleAVLNode*)malloc(sizeof(SimpleAVLNode));
        n->key = *key;
        n->height = 1;
        n->left = n->right = nullptr;
        *inserted = true;
        return n;
    }
    if (key->id < node->key.id) {
        node->left = sInsert(node->left, key, inserted);
    } else if (key->id > node->key.id) {
        node->right = sInsert(node->right, key, inserted);
    } else {
        *inserted = false;
        return node;
    }

    node->height = 1 + sMax(sHeight(node->left), sHeight(node->right));
    int32_t b = sBalance(node);

    if (b > 1 && key->id < node->left->key.id) return sRotateRight(node);
    if (b < -1 && key->id > node->right->key.id) return sRotateLeft(node);
    if (b > 1 && key->id > node->left->key.id) {
        node->left = sRotateLeft(node->left);
        return sRotateRight(node);
    }
    if (b < -1 && key->id < node->right->key.id) {
        node->right = sRotateRight(node->right);
        return sRotateLeft(node);
    }
    return node;
}

static SimpleAVLNode* sFind(SimpleAVLNode* node, int64_t id) {
    while (node) {
        if (id == node->key.id) return node;
        node = (id < node->key.id) ? node->left : node->right;
    }
    return nullptr;
}

static void sDestroy(SimpleAVLNode* node) {
    if (!node) return;
    sDestroy(node->left);
    sDestroy(node->right);
    free(node);
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
    constexpr size_t NUM_ITEMS = 50000;
    constexpr size_t ITERATIONS = 20;

    std::cout << "Generating dataset of " << NUM_ITEMS << " randomized keys...\n";
    std::vector<TestItem> dataset(NUM_ITEMS);
    std::vector<int64_t> searchKeys(NUM_ITEMS);

    std::mt19937_64 rng(42);
    for (size_t i = 0; i < NUM_ITEMS; ++i) {
        dataset[i].id = static_cast<int64_t>(rng() % (NUM_ITEMS * 10));
        dataset[i].value = static_cast<double>(i);
        searchKeys[i] = dataset[i].id;
    }

    std::shuffle(searchKeys.begin(), searchKeys.end(), rng);

    std::cout << "Running Set Benchmarks (" << ITERATIONS << " runs)...\n\n";

    // 1. std::set (Red-Black Tree)
    double stdInsert = measureAvgMs([&]() {
        std::set<TestItem, CompareTestItem> s;
        for (size_t i = 0; i < NUM_ITEMS; ++i) {
            s.insert(dataset[i]);
        }
    }, ITERATIONS);

    std::set<TestItem, CompareTestItem> stdSetup(dataset.begin(), dataset.end());
    double stdFind = measureAvgMs([&]() {
        volatile double sum = 0;
        TestItem probe{0, 0.0};
        for (size_t i = 0; i < NUM_ITEMS; ++i) {
            probe.id = searchKeys[i];
            auto it = stdSetup.find(probe);
            if (it != stdSetup.end()) sum += it->value;
        }
    }, ITERATIONS);

    double stdTraverse = measureAvgMs([&]() {
        volatile double sum = 0;
        for (const auto& item : stdSetup) {
            sum += item.value;
        }
    }, ITERATIONS * 5);

    // 2. Simple C AVL
    double simpleInsert = measureAvgMs([&]() {
        SimpleAVLNode* root = nullptr;
        bool inserted = false;
        for (size_t i = 0; i < NUM_ITEMS; ++i) {
            root = sInsert(root, &dataset[i], &inserted);
        }
        sDestroy(root);
    }, ITERATIONS);

    SimpleAVLNode* simpleSetup = nullptr;
    bool dummyInserted = false;
    for (size_t i = 0; i < NUM_ITEMS; ++i) {
        simpleSetup = sInsert(simpleSetup, &dataset[i], &dummyInserted);
    }

    double simpleFind = measureAvgMs([&]() {
        volatile double sum = 0;
        for (size_t i = 0; i < NUM_ITEMS; ++i) {
            SimpleAVLNode* n = sFind(simpleSetup, searchKeys[i]);
            if (n) sum += n->key.value;
        }
    }, ITERATIONS);

    sDestroy(simpleSetup);

    // 3. ForgeAVLTree
    double forgeInsert = measureAvgMs([&]() {
        ForgeAVLTree tree;
        forgeOrderedSetCreate(&tree, sizeof(TestItem), compareTestItemC, nullptr);
        for (size_t i = 0; i < NUM_ITEMS; ++i) {
            forgeOrderedSetInsert(&tree, &dataset[i]);
        }
        forgeOrderedSetDestroy(&tree);
    }, ITERATIONS);

    ForgeAVLTree forgeSetup;
    forgeOrderedSetCreate(&forgeSetup, sizeof(TestItem), compareTestItemC, nullptr);
    for (size_t i = 0; i < NUM_ITEMS; ++i) {
        forgeOrderedSetInsert(&forgeSetup, &dataset[i]);
    }

    double forgeFind = measureAvgMs([&]() {
        volatile double sum = 0;
        for (size_t i = 0; i < NUM_ITEMS; ++i) {
            TestItem probe{searchKeys[i], 0.0};
            TestItem* found = (TestItem*)forgeOrderedSetFind(&forgeSetup, &probe);
            if (found) sum += found->value;
        }
    }, ITERATIONS);

    double forgeTraverse = measureAvgMs([&]() {
        volatile double sum = 0;
        forgeOrderedSetTraverseInorder(&forgeSetup, [](const void* val, void* udata) {
            *(double*)udata += ((const TestItem*)val)->value;
        }, (void*)&sum);
    }, ITERATIONS * 5);

    forgeOrderedSetDestroy(&forgeSetup);

    // Results
    std::cout << "\n================================ ORDERED SET RESULTS (Time in ms) ================================\n";
    std::cout << std::left << std::setw(28) << "Operation"
              << std::setw(24) << "std::set (Red-Black)"
              << std::setw(24) << "Simple AVL (C)"
              << std::setw(24) << "ForgeAVLTree" << "\n";
    std::cout << std::string(98, '-') << "\n";

    auto printRow = [](const char* name, double d1, double d2, double d3) {
        std::cout << std::left << std::setw(28) << name
                  << std::fixed << std::setprecision(5)
                  << std::setw(24) << d1
                  << std::setw(24) << d2
                  << std::setw(24) << d3 << "\n";
    };

    printRow("Insert (50k keys)",   stdInsert,   simpleInsert,   forgeInsert);
    printRow("Find (50k queries)",   stdFind,     simpleFind,     forgeFind);
    printRow("In-order Traversal",  stdTraverse, 0.0 /* N/A */,  forgeTraverse);
    std::cout << "==================================================================================================\n";

    return 0;
}
