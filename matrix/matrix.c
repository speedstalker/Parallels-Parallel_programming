#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <time.h>

#include <errno.h>
#include <assert.h>

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


#define MTX_DIMENSION 1040

static inline size_t _min (size_t l, size_t r);

int main ()
{
size_t i = 0, j = 0, k = 0;
size_t i0 = 0, j0 = 0, k0 = 0, block_size = 0; // for block algo

long long **mtx1 = NULL, **mtx2 = NULL, **answ_mtx = NULL;
long long tmp = 0; // for block algo

unsigned long long rdtscStart = 0, rdtscEnd = 0;
time_t timeStart = 0, timeEnd = 0;

if (unlikely((mtx1 = (long long**)calloc (MTX_DIMENSION, sizeof (long long*))) == NULL))
        HANDLE_ERROR ("calloc: mtx1");
for (i = 0; i < MTX_DIMENSION; i++)
        {
        if (unlikely((mtx1[i] = (long long*)calloc (MTX_DIMENSION, sizeof (long long))) == NULL))
                HANDLE_ERROR ("calloc: mtx1[i]");

        for (j = 0; j < MTX_DIMENSION; j++)
                {
                mtx1[i][j] = (i + 1) * (j + 1);
                // printf ("%lld ", mtx1[i][j]);
                }
        // printf ("\n");
        }

if (unlikely((mtx2 = (long long**)calloc (MTX_DIMENSION, sizeof (long long*))) == NULL))
        HANDLE_ERROR ("calloc: mtx2");
for (i = 0; i < MTX_DIMENSION; i++)
        {
        if (unlikely((mtx2[i] = (long long*)calloc (MTX_DIMENSION, sizeof (long long))) == NULL))
                HANDLE_ERROR ("calloc: mtx2[i]");

        for (j = 0; j < MTX_DIMENSION; j++)
                {
                mtx2[i][j] = (i + 1) * (j + 1);
                // printf ("%lld ", mtx1[i][j]);
                }
        // printf ("\n");
        }

if (unlikely((answ_mtx = (long long**)calloc (MTX_DIMENSION, sizeof (long long*))) == NULL))
        HANDLE_ERROR ("calloc: answ_mtx");
for (i = 0; i < MTX_DIMENSION; i++)
        if (unlikely((answ_mtx[i] = (long long*)calloc (MTX_DIMENSION, sizeof (long long))) == NULL))
                HANDLE_ERROR ("calloc: answ_mtx[i]");

//==========
// computing of block multi
//----------
// printf ("TIME: Start of block multiplicating = %ld\r\n", timeStart = time (NULL));
// block_size = 512;

for (block_size = 16; block_size <= MTX_DIMENSION; block_size += 16)
{
printf ("%zu ", block_size);
timeStart = time (NULL);
rdtscStart = rdtsc ();
/* I will not indent 6 times
 * I will not indent 6 times
 * I will not ind...
 */
for (i0 = 0; i0 < MTX_DIMENSION; i0 += block_size)
        for (j0 = 0; j0 < MTX_DIMENSION; j0 += block_size)
                for (k0 = 0; k0 < MTX_DIMENSION; k0 += block_size)
                        {
                        for (i = i0; i < _min (i0 + block_size, MTX_DIMENSION); i++)
                                for (j = j0; j < _min (j0 + block_size, MTX_DIMENSION); j++)
                                        for (k = k0; k < _min (k0 + block_size, MTX_DIMENSION); k++)
                                                {
                                                // += used for avoiding "warning: variable ‘tmp’ set but not used"
                                                tmp += mtx1[i][k] * mtx2[k][j];
                                                // printf ("%lld ", tmp);

                                                // for real computing: need to lock mutex or do smth similar here
                                                // answ_mtx[i][j] += tmp;
                                                }
                        }

rdtscEnd = rdtsc() - rdtscStart;
timeEnd = time (NULL) - timeStart;
printf ("%lld %ld\n", rdtscEnd, timeEnd);
}

// printf ("TIME: End of block multiplication, delta = %ld\r\n", time (NULL) - timeStart);
//==========


for (i = 0; i < MTX_DIMENSION; i++)
        {
        free (answ_mtx[i]);
        answ_mtx[i] = NULL;
        }
free (answ_mtx);
answ_mtx = NULL;

for (i = 0; i < MTX_DIMENSION; i++)
        {
        free (mtx2[i]);
        mtx2[i] = NULL;
        }
free (mtx2);
mtx2 = NULL;

for (i = 0; i < MTX_DIMENSION; i++)
        {
        free (mtx1[i]);
        mtx1[i] = NULL;
        }
free (mtx1);
mtx1 = NULL;

return 0;
}


static inline size_t _min (size_t l, size_t r)
{
return (likely(l < r))? l : r;
}

