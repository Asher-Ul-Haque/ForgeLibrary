/**
 * @file : threadPool.h 
 * @brief : Thread pool impelemntation for unix in c
 */

#if !defined(_WIN32)

#pragma once 

#include <forgeUtils/memory/linearAlloc.h>
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/// @brief : Task function callback signature
typedef void (*ForgeTaskFunc)(void* ARG);

/// @brief : Task struct 
typedef struct ForgeTask 
{
  ForgeTaskFunc func; ///< Task function 
  void*         arg;  ///< Function arguments
} ForgeTask;

/// @brief : A thread pool
typedef struct forgeThreadPool 
{
  pthread_t*              threads;        ///< Array of worker thread handles
  size_t                  threadCount;    ///< Total number of worker threads
  
  ForgeTask*              taskQueue;      ///< Circular task queue
  size_t                  queueCapacity;  ///< Total capacity of task queue
  size_t                  queueHead;      ///< Queue write head
  size_t                  queueTail;      ///< Queue read tail
  size_t                  queueCount;     ///< Current pending task count
  
  pthread_mutex_t         lock;           ///< Lock protecting queue state
  pthread_cond_t          hasWork;        ///< Condition variable signaled when task is pushed
  pthread_cond_t          workingDone;    ///< Condition variable signaled when all tasks complete
  
  size_t                  activeWorkers;  ///< Workers currently executing a task
  bool                    shutdown;       ///< Flag set when pool destruction is requested
  ForgeLinearAllocator*   allocator;      ///< Optional linear allocator
} ForgeThreadPool;

/**
 * @brief : Creates a thread pool with the specified number of worker threads.
 * 
 * @param POOL : Pointer to ThreadPool struct.
 * @param THREAD_COUNT : Number of worker threads to spawn (0 defaults to hardware CPU cores).
 * @param QUEUE_CAPACITY : Maximum capacity for pending tasks in queue (0 defaults to 1024).
 * @param ALLOCATOR : Optional linear allocator or NULL for global memory tracker.
 * @return : true if created successfully, false otherwise.
 */
bool forgeThreadpoolCreate(
  ForgeThreadPool*        POOL,
  size_t                  THREAD_COUNT,
  size_t                  QUEUE_CAPACITY,
  ForgeLinearAllocator*   ALLOCATOR);

/**
 * @brief : Submits a work task to the thread pool queue.
 * 
 * @param POOL :  Pointer to ThreadPool.
 * @param FUNC : Task function to execute.
 * @param ARG :  User data argument passed to func.
 * @return : true if task was enqueued, false if queue is full or pool shutting down.
 */
bool forgeThreadpoolAddTask(
  ForgeThreadPool*  POOL, 
  ForgeTaskFunc     FUNC, 
  void*             ARG);

/**
 * @brief : Blocks caller thread until all currently queued and active tasks complete.
 * @param POOL : Pointer to the thread pool
 */
void forgeThreadpoolWait(ForgeThreadPool* POOL);

/**
 * @brief : Destroys the thread pool, waiting for tasks to complete and terminating threads.
 * @param POOL : Pointer to the pool to be destroyed
 */
void forgeThreadpoolDestroy(ForgeThreadPool* POOL);

/**
 * @brief : Returns total number of tasks currently waiting or executing.
 * @param POOL : The thread pool 
 * @return : number of pending tasks
 */
size_t forgeThreadpoolPendingTasks(ForgeThreadPool* POOL);

#ifdef __cplusplus
}
#endif
#endif
