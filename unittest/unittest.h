// Hacky, but makes copy pasting into fuchsia.git simpler.
#define BEGIN_TEST_CASE(NAME)                       \
    int main() {
#define END_TEST_CASE(NAME)                         \
    }
#define RUN_TEST(TEST_FUNC)                         \
    {                                               \
        const char* error_msg = nullptr;            \
        zx_status_t status = TEST_FUNC();           \
        if (status == ZX_OK) {                      \
            printf("[ \033[0;31mERROR\033[0m  ]");  \
        } else {                                    \
            printf("[ \033[0;32mPASSES\033[0m ]");  \
        }                                           \
        printf(" " #TEST_FUNC);                     \
        if (error_msg) {                            \
            printf(": %s ", error_msg);             \
        }                                           \
        printf("\n");                               \
    }
#define BEGIN_TEST
#define END_TEST                                    \
    return true;
#define BEGIN_HELPER
#define END_HELPER                                  \
    return true;
#define ASSERT_TRUE(COND)                           \
    {                                               \
        bool result = (COND);                       \
        if (!result) return false;                  \
    }
#define ASSERT_EQ(LEFT, RIGHT)                      \
    {                                               \
        if ((LEFT) != (RIGHT)) return false;        \
    }

