#include <stdint.h>
#include <system/threadPool.h>
#include <core/asserts.h>
#include <core/logger.h>
#include <memory/tracker.h>

#include <stdlib.h>
#include <unistd.h>

#define DEFAULT_queueCapacity 1024

static void* workerThreadLoop(void* ARG) 
{
  ThreadPool* pool = (ThreadPool*)ARG;

  while (1) 
  {
    pthread_mutex_lock(&pool->lock);

    // - - - Wait while queue is empty and pool is active
    while (pool->queueCount == 0 && !pool->shutdown) 
    {
      pthread_cond_wait(&pool->hasWork, &pool->lock);
    }

    if (pool->shutdown && pool->queueCount == 0) 
    {
      pthread_mutex_unlock(&pool->lock);
      pthread_exit(NULL);
    }

    // - - - Pop task from circular queue
    ForgeTask task = pool->taskQueue[pool->queueTail];
    pool->queueTail = (pool->queueTail + 1) % pool->queueCapacity;
    pool->queueCount--;
    pool->activeWorkers++;

    pthread_mutex_unlock(&pool->lock);

    // - - - Execute task outside mutex lock
    if (task.func) task.func(task.arg);

    pthread_mutex_lock(&pool->lock);
    pool->activeWorkers--;

    // - - - Signal waiting callers if all tasks completed
    if (pool->queueCount == 0 && pool->activeWorkers == 0) 
    {
      pthread_cond_broadcast(&pool->workingDone);
    }

    pthread_mutex_unlock(&pool->lock);
  }

  return NULL;
}

bool forgeThreadpoolCreate(
  ThreadPool*       POOL,
  size_t            THREAD_COUNT,
  size_t            QUEUE_CAPACITY,
  LinearAllocator*  ALLOCATOR)
{
  FORGE_ASSERT_DEBUG_MESSAGE(POOL != NULL, "[THREAD POOL] : Target pool pointer cannot be NULL");

  if (THREAD_COUNT == 0) 
  {
    uint64_t cores = sysconf(_SC_NPROCESSORS_ONLN);
    THREAD_COUNT = (cores > 0) ? (size_t)cores : 4;
  }

  if (QUEUE_CAPACITY == 0) QUEUE_CAPACITY = DEFAULT_queueCapacity;

  POOL->threadCount   = THREAD_COUNT;
  POOL->queueCapacity = QUEUE_CAPACITY;
  POOL->queueHead     = 0;
  POOL->queueTail     = 0;
  POOL->queueCount    = 0;
  POOL->activeWorkers = 0;
  POOL->shutdown      = false;
  POOL->allocator     = ALLOCATOR;

  // - - - Allocate thread handles array and task queue
  if (POOL->allocator) 
  {
    POOL->threads   = (pthread_t*)linearAllocAllocate(POOL->allocator, THREAD_COUNT * sizeof(pthread_t), DEFAULT_ALIGNMENT_BYTES);
    POOL->taskQueue = (ForgeTask*)linearAllocAllocate(POOL->allocator, QUEUE_CAPACITY * sizeof(ForgeTask), DEFAULT_ALIGNMENT_BYTES);
  } 
  else 
  {
    POOL->threads   = (pthread_t*)FORGE_MALLOC(THREAD_COUNT * sizeof(pthread_t));
    POOL->taskQueue = (ForgeTask*)FORGE_MALLOC(QUEUE_CAPACITY * sizeof(ForgeTask));
  }

  if (!POOL->threads || !POOL->taskQueue) 
  {
    FORGE_LOG_ERROR("[THREAD POOL] : Failed to allocate thread or task memory!");
    return false;
  }

  // - - - Initialize POSIX mutex & condition variables
  if (pthread_mutex_init(&POOL->lock, NULL) != 0 ||
      pthread_cond_init(&POOL->hasWork, NULL) != 0 ||
      pthread_cond_init(&POOL->workingDone, NULL) != 0) 
  {
    FORGE_LOG_ERROR("[THREAD POOL] : Failed to initialize pthread synchronization primitives!");
    return false;
  }

  // - - - Spawn worker threads
  for (size_t i = 0; i < THREAD_COUNT; ++i) 
  {
    if (pthread_create(&POOL->threads[i], NULL, workerThreadLoop, POOL) != 0) 
    {
      FORGE_LOG_ERROR("[THREAD POOL] : Failed to spawn worker thread %zu", i);
      forgeThreadpoolDestroy(POOL);
      return false;
    }
  }

  FORGE_LOG_INFO("[THREAD POOL] : Created thread pool with %zu workers and queue capacity of %zu", 
                  THREAD_COUNT, QUEUE_CAPACITY);
  return true;
}

bool forgeThreadpoolAddTask(
  ThreadPool*   POOL, 
  ForgeTaskFunc FUNC, 
  void*         ARG) 
{
  FORGE_ASSERT_DEBUG_MESSAGE(POOL != NULL, "[THREAD POOL] : Cannot add task to NULL POOL");
  FORGE_ASSERT_DEBUG_MESSAGE(FUNC != NULL, "[THREAD POOL] : Task FUNC cannot be NULL");

  pthread_mutex_lock(&POOL->lock);

  if (POOL->shutdown || POOL->queueCount == POOL->queueCapacity) 
  {
    pthread_mutex_unlock(&POOL->lock);
    FORGE_LOG_WARNING("[THREAD POOL] : Task rejected! Queue full or pool shutting down.");
    return false;
  }

  POOL->taskQueue[POOL->queueHead].func = FUNC;
  POOL->taskQueue[POOL->queueHead].arg  = ARG;
  POOL->queueHead                       = (POOL->queueHead + 1) % POOL->queueCapacity;
  POOL->queueCount++;

  // - - - Signal sleeping worker thread
  pthread_cond_signal(&POOL->hasWork);
  pthread_mutex_unlock(&POOL->lock);

  return true;
}

void forgeThreadpoolWait(ThreadPool* POOL) 
{
  FORGE_ASSERT_DEBUG_MESSAGE(POOL != NULL, "[THREAD POOL] : Cannot wait for a NULL POOL");

  pthread_mutex_lock(&POOL->lock);
  while (POOL->queueCount > 0 || POOL->activeWorkers > 0) 
  {
    pthread_cond_wait(&POOL->workingDone, &POOL->lock);
  }
  pthread_mutex_unlock(&POOL->lock);
}

void forgeThreadpoolDestroy(ThreadPool* POOL) 
{
  FORGE_ASSERT_DEBUG_MESSAGE(POOL != NULL, "[THREAD POOL] : Cannot destroy a NULL POOL");

  // - - - Wait for ongoing tasks to finish
  forgeThreadpoolWait(POOL);

  pthread_mutex_lock(&POOL->lock);
  POOL->shutdown = true;
  pthread_cond_broadcast(&POOL->hasWork);
  pthread_mutex_unlock(&POOL->lock);

  // - - - Join all worker threads
  for (size_t i = 0; i < POOL->threadCount; ++i) 
  {
    pthread_join(POOL->threads[i], NULL);
  }

  // - - - Destroy pthread primitives
  pthread_mutex_destroy(&POOL->lock);
  pthread_cond_destroy(&POOL->hasWork);
  pthread_cond_destroy(&POOL->workingDone);

  if (!POOL->allocator) 
  {
    FORGE_FREE(POOL->threads);
    FORGE_FREE(POOL->taskQueue);
  }

  POOL->threads       = NULL;
  POOL->taskQueue     = NULL;
  POOL->threadCount   = 0;
  POOL->queueCapacity = 0;
}

size_t forgeThreadpoolPendingTasks(ThreadPool* POOL) 
{
  FORGE_ASSERT_DEBUG_MESSAGE(POOL != NULL, "[THREAD POOL] : Cannot check pending tasks of a NULL POOL");

  if (!POOL) return 0;
  pthread_mutex_lock(&POOL->lock);
  size_t count = POOL->queueCount + POOL->activeWorkers;
  pthread_mutex_unlock(&POOL->lock);
  return count;
}
