
#if USE_ALGO_STEP_LOGGING

typedef union
{
    signed int i32_value;
    unsigned int ui32_value;
    uint64_t ui64_value;
    void *ptr;
} InflateAlgoStepType;

typedef struct
{
    int type;
    InflateAlgoStepType value;
    InflateAlgoStepType value2;
    InflateAlgoStepType value3;
    const char *name;
} InflateAlgoStep;

#define ALGO_CONTAINER_SIZE 4
#define ALGO_STEPS_SIZE 1000

InflateAlgoStep algo_steps[ALGO_CONTAINER_SIZE][ALGO_STEPS_SIZE];
int algo_steps_sizes[ALGO_CONTAINER_SIZE];
int algo_container = 0;
int algo_step = 0;

void algo_clear()
{
    memset(algo_steps, 0, sizeof(algo_steps));
    memset(algo_steps_sizes, 0, sizeof(algo_steps_sizes));
}

void algo_change_container(int c)
{
    if (algo_container >= 0 && algo_container < ALGO_CONTAINER_SIZE)
    {
        algo_steps_sizes[algo_container] = algo_step;
    }

    algo_container = c;
    algo_step = 0;
}

InflateAlgoStep *algo_cur_steps()
{
    if (algo_container >= 0 && algo_container < ALGO_CONTAINER_SIZE)
    {
        return algo_steps[algo_container];
    }
    return 0;
}

void add_algo_step_i32_value(int type, int value, int value2, const char *name)
{
    InflateAlgoStep *steps = algo_cur_steps();
    if (steps != 0)
    {
        if (algo_step < ALGO_STEPS_SIZE)
        {
            steps += algo_step;
            steps->type = type;
            steps->name = name;
            steps->value.i32_value = value;
            steps->value2.i32_value = value2;
            ++algo_step;
        }
    }
}

void add_algo_step_ui32_value(int type, unsigned int value, unsigned int value2, const char *name)
{
    InflateAlgoStep *steps = algo_cur_steps();
    if (steps != 0)
    {
        if (algo_step < ALGO_STEPS_SIZE)
        {
            steps += algo_step;
            steps->type = type;
            steps->name = name;
            steps->value.ui32_value = value;
            steps->value2.ui32_value = value2;
            ++algo_step;
        }
    }
}

void add_algo_step_ui32_ptr_value(int type, unsigned int value, void* ptr, const char *name)
{
    InflateAlgoStep *steps = algo_cur_steps();
    if (steps != 0)
    {
        if (algo_step < ALGO_STEPS_SIZE)
        {
            steps += algo_step;
            steps->type = type;
            steps->name = name;
            steps->value.ptr = ptr;
            steps->value2.ui32_value = value;
            ++algo_step;
        }
    }
}

void add_algo_step_ptr(int type, void *ptr, const char *name)
{
    InflateAlgoStep *steps = algo_cur_steps();
    if (steps != 0)
    {
        if (algo_step < ALGO_STEPS_SIZE)
        {
            steps += algo_step;
            steps->type = type;
            steps->name = name;
            steps->value.ptr = ptr;
            ++algo_step;
        }
    }
}
void add_algo_step_no_value(int type)
{
    InflateAlgoStep *steps = algo_cur_steps();
    if (steps != 0)
    {
        if (algo_step < ALGO_STEPS_SIZE)
        {
            steps += algo_step;
            steps->type = type;
            steps->value.i32_value = 0;
            ++algo_step;
        }
    }
}

#define INFLATE_ALGO_CLEAR_STEPS algo_clear()

#define INFLATE_ALGO_STEP(T) add_algo_step_no_value(T)
#define INFLATE_ALGO_STEP_I32_VALUE(T, V, N) add_algo_step_i32_value(T, V, 0, N)
#define INFLATE_ALGO_STEP_I32_I32_VALUE(T, V1, V2, N) add_algo_step_i32_value(T, V1, V2, N)
#define INFLATE_ALGO_STEP_UI32_VALUE(T, V, N) add_algo_step_ui32_value(T, V, 0, N)
#define INFLATE_ALGO_STEP_UI32_UI32_VALUE(T, V1, V2, N) add_algo_step_ui32_value(T, V1, V2, N)
#define INFLATE_ALGO_STEP_UI32_PTR_VALUE(T, V, PTR, N) add_algo_step_ui32_ptr_value(T, V, PTR, N)
#define INFLATE_ALGO_STEP_PTR(T, P, N) add_algo_step_ptr(T, P, N)
#else
#define INFLATE_ALGO_CLEAR_STEPS \
    do                           \
    {                            \
    } while (0)

#define INFLATE_ALGO_STEP(T) \
    do                       \
    {                        \
    } while (0)

#define INFLATE_ALGO_STEP_I32_VALUE(T, V, N) \
    do                                       \
    {                                        \
    } while (0)

#define INFLATE_ALGO_STEP_I32_I32_VALUE(T, V1, V2, N) \
    do                                                \
    {                                                 \
    } while (0)

#define INFLATE_ALGO_STEP_UI32_VALUE(T, V, N) \
    do                                        \
    {                                         \
    } while (0)

#define INFLATE_ALGO_STEP_UI32_UI32_VALUE(T, V1, V2, N) \
    do                                                  \
    {                                                   \
    } while (0)

#define INFLATE_ALGO_STEP_PTR(T, P, N) \
    do                              \
    {                               \
    } while (0)
#endif

#ifndef ALGO_STEPS_DEFINED
#define ALGO_STEPS_DEFINED 1

#define ALGO_STEP_FORMAT_INIT(S, F) {S, F}

#define ALGO_PT_PTR 0x10
#define ALGO_PT_INT32 0x20
#define ALGO_PT_UINT32 0x30
#define ALGO_PT_UINT64 0x40

struct type_and_format
{
    int type_and_args;
    const char *format;
};

enum AlgoStep
{
    ALGO_STEP_UNKNOWN = 0,
    ALGO_STEP_START_EXEC,
    ALGO_STEP_NULLIFY,
    //
    ALGO_STEP_START_IN_PTR,
    ALGO_STEP_START_OUT_PTR,
    ALGO_STEP_START_IN_SIZE,
    ALGO_STEP_START_OUT_SIZE,
    //
    ALGO_STEP_FIXED_TREE,
    ALGO_STEP_BUILD_FIXED_TREE,
    ALGO_STEP_GET_BITS,
    ALGO_STEP_GET_BITS_RESULT,
    //
    ALGO_STEP_RAW,
    ALGO_STEP_DYN_TREE,
    //
    ALGO_STEP_BT_USED_AVAIL,
    ALGO_STEP_BT_OFFSETS,
    ALGO_STEP_BT_CODES,
    ALGO_STEP_BT_CODES2,
    ALGO_STEP_BT_CODES3,
    ALGO_STEP_BT_CODES4,
    ALGO_STEP_BT_CODES5,
    //
    ALGO_STEP_BD_WRITE_RAW,
    //
    ALGO_STEP_DECODE_TREES_READ_CODE_LENGTHS,
    ALGO_STEP_DECODE_TREES_GET_BITS,
    ALGO_STEP_DT_SYM,
    ALGO_STEP_DT_SYM2,
    ALGO_STEP_DT_ITER,
    ALGO_STEP_DT_ITER_END,
    //
    ALGO_STEP_CHANGE_TO,
    ALGO_STEP_ERROR,
    ALGO_STEP_EXIT,
    ALGO_STEP_MAX
};

struct type_and_format algo_step_messages[ALGO_STEP_MAX + 1] = {
    ALGO_STEP_FORMAT_INIT(ALGO_PT_PTR | 1, "Unknown: %p"),
    ALGO_STEP_FORMAT_INIT(0, "EXEC!"),
    ALGO_STEP_FORMAT_INIT(ALGO_PT_UINT32 | 1, "Nullify: %d"),
    //
    ALGO_STEP_FORMAT_INIT(ALGO_PT_PTR | 1, "IN-PTR: %p"),
    ALGO_STEP_FORMAT_INIT(ALGO_PT_PTR | 1, "OUT-PTR: %p"),
    ALGO_STEP_FORMAT_INIT(ALGO_PT_UINT32 | 1, "IN-SIZE: %d"),
    ALGO_STEP_FORMAT_INIT(ALGO_PT_UINT32 | 1, "OUT-SIZE: %d"),
    //
    ALGO_STEP_FORMAT_INIT(0, "FIXED-TREE!"),
    ALGO_STEP_FORMAT_INIT(0, "BUILD-FIXED-TREE!"),
    ALGO_STEP_FORMAT_INIT(ALGO_PT_UINT32 | 1, "GET-BITS: %d"),
    ALGO_STEP_FORMAT_INIT(ALGO_PT_UINT32 | 1, "GET-BITS:res: %d"),
    //
    ALGO_STEP_FORMAT_INIT(0, "RAW!"),
    ALGO_STEP_FORMAT_INIT(0, "DYN-TREE!"),
    //
    ALGO_STEP_FORMAT_INIT(ALGO_PT_INT32 | 2, "BT:USED ? AVAIL: %d > %d"),
    ALGO_STEP_FORMAT_INIT(ALGO_PT_INT32 | 2, "BT:OFFS:%2d:%d"),
    ALGO_STEP_FORMAT_INIT(ALGO_PT_UINT32 | 2, "BT:CODES:%2d:%d"),
    ALGO_STEP_FORMAT_INIT(ALGO_PT_UINT32 | 2, "BT:CODES2:%2d:%d"),
    ALGO_STEP_FORMAT_INIT(ALGO_PT_UINT32 | 2, "BT:CODES3:%2d:%d"),
    ALGO_STEP_FORMAT_INIT(ALGO_PT_UINT32 | 2, "BT:CODES4:%2d:%d"),
    ALGO_STEP_FORMAT_INIT(ALGO_PT_UINT32 | 2, "BT:CODES5:num_codes=%2d|max_sym=%2d"),
    //
    ALGO_STEP_FORMAT_INIT(ALGO_PT_PTR | 2, "BD:RAW:%p: %d"),
    //
    ALGO_STEP_FORMAT_INIT(ALGO_PT_INT32 | 1, "DT: Read code lengths |%d|"),
    ALGO_STEP_FORMAT_INIT(ALGO_PT_INT32 | 2, "DT: Get bits: %d => %d"),
    ALGO_STEP_FORMAT_INIT(ALGO_PT_INT32 | 2, "DT: Sym:%2d | Idx:%2d"),
    ALGO_STEP_FORMAT_INIT(ALGO_PT_INT32 | 2, "DT: Sym:%2d | Len:%2d"),
    ALGO_STEP_FORMAT_INIT(ALGO_PT_INT32 | 1, "DT:ITER: num:%2d"),
    ALGO_STEP_FORMAT_INIT(ALGO_PT_INT32 | 1, "DT:ITER-END: num:%2d"),
    //
    ALGO_STEP_FORMAT_INIT(ALGO_PT_INT32 | 1, "NEW-STATE: %d"),
    ALGO_STEP_FORMAT_INIT(ALGO_PT_INT32 | 1, "ERROR: %d"),
    ALGO_STEP_FORMAT_INIT(ALGO_PT_INT32 | 1, "EXIT: %d"),
    ALGO_STEP_FORMAT_INIT(0, "UNKNOWN!"),
};

#if USE_ALGO_STEP_LOGGING
void algo_print_steps(int start, int max, void *test_buffer)
{
    TestStringBuffer *sb = 0;
    if (test_buffer != 0)
    {
        sb = (TestStringBuffer *)test_buffer;
    }
    if (max >= ALGO_CONTAINER_SIZE)
    {
        printf("ERROR: algo_print_steps too large. Value:%d\n", max);
        return;
    }
    if (algo_container < ALGO_CONTAINER_SIZE)
    {
        algo_steps_sizes[algo_container] = algo_step;
    }
    int max_step = 0;
    int i = start;
    for (; i <= max; i++)
    {
        if (max_step < algo_steps_sizes[i])
        {
            max_step = algo_steps_sizes[i];
        }
    }

#define ASB_LEN 128
    char buf[ASB_LEN];
    char buf2[ASB_LEN];
    memset(buf, 0, ASB_LEN);
    memset(buf2, 0, ASB_LEN);

    int temp_len = snprintf(buf2, ASB_LEN, "#CONTAINERS: %d-%d | #MAX-STEPS: %d\n", start, max, max_step);

#define ASB                                            \
    do                                                 \
    {                                                  \
        if (sb != 0)                                   \
        {                                              \
            addToTestStringBuffer(sb, buf2, temp_len); \
        }                                              \
        else                                           \
        {                                              \
            printf("%s", buf2);                        \
        }                                              \
    } while (0)

#define HDR_FMT "|%34d|"
#define LINE_FMT "%33s|"

    int cIdx = start;
    for (; cIdx <= max; cIdx++)
    {
        memset(buf2, 0, ASB_LEN);

        temp_len = snprintf(buf2, ASB_LEN, HDR_FMT, cIdx);
        ASB;
    }
    memset(buf2, 0, ASB_LEN);
    buf2[0] = '\n';
    temp_len = 1;
    ASB;

    for (i = 0; i < max_step+2; i++)
    {
        cIdx = start;
        for (; cIdx <= max; cIdx++)
        {
            InflateAlgoStep *step = algo_steps[cIdx];
            step += i;
            int type = step->type;
            memset(buf, 0, ASB_LEN);
            memset(buf2, 0, ASB_LEN);
            buf2[0] = '|';
            buf2[1] = ' ';
            char *pB = buf2 + 2;

            if (type >= 0 && type < ALGO_STEP_MAX)
            {
                struct type_and_format tf = algo_step_messages[type];
                int t = tf.type_and_args;
                int num_args = (t & 0xF);

                if (0 != num_args)
                {
                    t &= 0xF0;

                    const char *fmt = tf.format;

                    int has_name = (strstr(fmt, "%s") != 0) ? 1 : 0;

                    char name_buffer[50];

                    size_t name_len = 0;
                    if (step->name != 0)
                    {
                        name_len = strlen(step->name);
                    }

                    if (has_name == 0 && name_len > 0 && name_len < 20 && strlen(fmt) < 30)
                    {
                        memset(name_buffer, 0, 50);
                        snprintf(name_buffer, 50, "(%s)%s", step->name, fmt);
                        fmt = name_buffer;
                    }

                    switch (t)
                    {
                    case ALGO_PT_PTR:
                        if (has_name != 0)
                        {
                            snprintf(buf, ASB_LEN, fmt, step->name, step->value.ptr, step->value2.ui32_value);
                        }
                        else
                        {
                            snprintf(buf, ASB_LEN, fmt, step->value.ptr, step->value2.ui32_value);
                        }

                        break;
                    case ALGO_PT_INT32:
                        if (has_name != 0)
                        {
                            snprintf(buf, ASB_LEN, fmt, step->name, step->value.i32_value, step->value2.i32_value);
                        }
                        else
                        {
                            snprintf(buf, ASB_LEN, fmt, step->value.i32_value, step->value2.i32_value);
                        }

                        break;
                    case ALGO_PT_UINT32:
                        if (has_name != 0)
                        {
                            snprintf(buf, ASB_LEN, fmt, step->name, step->value.ui32_value, step->value2.ui32_value);
                        }
                        else
                        {
                            snprintf(buf, ASB_LEN, fmt, step->value.ui32_value, step->value2.ui32_value);
                        }
                        break;
                    case ALGO_PT_UINT64:
                    default:
                        snprintf(buf, ASB_LEN, "%s |%s | Val1:%ld | Val2:%ld", fmt, step->name, step->value.ui64_value, step->value2.ui64_value);
                        break;
                    }

                    temp_len = snprintf(pB, ASB_LEN - 2, LINE_FMT, buf);
                }
                else
                {
                    temp_len = snprintf(pB, ASB_LEN - 2, LINE_FMT, tf.format);
                }
            }
            else
            {
                snprintf(buf, ASB_LEN, "Unknown step: %d (ptr=%p, i32=%d)", type, step->value.ptr, step->value.i32_value);
                temp_len = snprintf(pB, ASB_LEN - 2, LINE_FMT, buf);
            }

            temp_len += 2;
            ASB;
        }
        buf2[0] = '\n';
        temp_len = 1;
        ASB;
        // printf("\n");
    }
#undef ASB
}
#endif

#endif