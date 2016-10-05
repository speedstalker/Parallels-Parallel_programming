#include <stdio.h>
#include <malloc.h>

#include <stdlib.h>
#include <errno.h>


//==============================================================================
// (un)likely defines
//------------------------------------------------------------------------------
#define likely(x)      __builtin_expect(!!(x), 1)
#define unlikely(x)    __builtin_expect(!!(x), 0)
//==============================================================================
// Error handling macroses
//------------------------------------------------------------------------------
#define IS_DEBUG 1

#if (IS_DEBUG == 1)
//{
   #define HANDLE_ERROR HANDLE_ERROR_wL
   #define HANDLE_ERROR_wL(msg)                                                    \
                   do                                                              \
                   {                                                               \
                   char err_msg[256] = {0};                                        \
                                                                                   \
                   snprintf (err_msg, 255, "%d. " msg "%c", __LINE__, '\0');       \
                   perror (err_msg);                                               \
                   exit   (EXIT_FAILURE);                                          \
                   }                                                               \
                   while (0)
//}
#else
//{
   #define HANDLE_ERROR_wL HANDLE_ERROR
   #define HANDLE_ERROR(msg) \
                  do { perror(msg); exit(EXIT_FAILURE); } while (0)
//}
#endif
//==============================================================================


#if defined(__i386__)
static __inline__ unsigned long long rdtsc(void)
        {
        unsigned long long int x;
        __asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
        return x;
        }
#elif defined(__x86_64__)
static __inline__ unsigned long long rdtsc(void)
        {
        unsigned hi, lo;
        __asm__ volatile ("rdtsc" : "=a"(lo), "=d"(hi));
        return ((unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
        }
#endif

// this defines for Intel Core i5-4250U: http://www.cpu-world.com/CPUs/Core_i5/Intel-Core%20i5-4250U%20Mobile%20processor.html
#define NUMB_OF_CORES 2         // w/o hyper-threading
#define SIZE_OF_CACHE_LINE 64   // 64 bytes

#define L1_SIZE ( 32*1024)      //  32KB per core
#define L2_SIZE (256*1024)      // 256KB per core
#define L3_SIZE (  3*1024*1024) //   3MB shared across all cores

#define L1_ASSOCIATIVITY  8     //  8 way-set associativity
#define L2_ASSOCIATIVITY  8     //  8 way-set associativity
#define L3_ASSOCIATIVITY 12     // 12 way-set associativity

#define SIZE_OF_L1_SET          (L1_SIZE / L1_ASSOCIATIVITY)    //   4KB
#define SIZE_OF_L2_SET          (L2_SIZE / L2_ASSOCIATIVITY)    //  32KB
#define SIZE_OF_L3_SET          (L3_SIZE / L3_ASSOCIATIVITY)    // 256KB


#define OFFSET                  SIZE_OF_L3_SET
#define ASSOCIATIVITY           L3_ASSOCIATIVITY

#define MULTIPLIER              2
#define NUMB_OF_ELEMS_TO_ACCESS 16

#define SIZE_OF_MEM_TO_ALLOC    (OFFSET * ASSOCIATIVITY * MULTIPLIER)
#define MAX_NUMB_OF_CHUNKS      (ASSOCIATIVITY * MULTIPLIER)


static inline  long long unsigned int  _access_and_measure (volatile int* arr_to_access, size_t idx_to_access_at);

int main ()
{
size_t i = 0, j = 0, cur_numb_of_chunks = 0;
volatile int* arr = NULL;

size_t numb_of_accesses = 0, sum_of_cycles = 0;

if (unlikely ((arr = (int*)calloc (SIZE_OF_MEM_TO_ALLOC / sizeof (int), sizeof (int))) == NULL))
        HANDLE_ERROR ("calloc: arr");

/*
for (i = 0; i < (SIZE_OF_MEM_TO_ALLOC / sizeof (int)); i += (SIZE_OF_L3_SET / sizeof (int)))
        {
        printf ("%lld\n", _access_and_measure (arr, i));
        }
*/

for (cur_numb_of_chunks = 1; cur_numb_of_chunks < MAX_NUMB_OF_CHUNKS; cur_numb_of_chunks++)
        {
        // printf ("cur_numb_of_chunks = %zu\n", cur_numb_of_chunks);
        sum_of_cycles = 0;
        numb_of_accesses = 0;

        for (i = 0; i < NUMB_OF_ELEMS_TO_ACCESS; i++)
                {
                for (j = 0; j < (cur_numb_of_chunks * OFFSET / sizeof (int)); j += (OFFSET / sizeof (int)))
                        {
                        // printf ("\t" "j + i = %zu," "\t" "cycles = %lld;\n", j + i, _access_and_measure (arr, j + i));
                        sum_of_cycles += _access_and_measure (arr, j + i);
                        numb_of_accesses++;
                        }
                }

        printf ("chunks = %zu, avg cycles = %zu\n", cur_numb_of_chunks, sum_of_cycles / numb_of_accesses);
        // printf ("\n");
        }

free ((void*)arr);
arr = NULL;

return 0;
}

static inline  long long unsigned int  _access_and_measure (volatile int* arr_to_access, size_t idx_to_access_at)
{
volatile int tmp = 0;
long long unsigned int rdtscStart = 0, rdtscEnd = 0;

rdtscStart = rdtsc ();
tmp += arr_to_access[idx_to_access_at];
rdtscEnd = rdtsc () - rdtscStart;

return rdtscEnd;
}

