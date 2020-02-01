// Force a compilation error if condition is true */
#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)]))

// Calculates the number of elements contained in an array
#define ARRAY_SIZE(x) (uint32_t)((sizeof (x)) / (sizeof *(x)))

// Gets the size of a struct member of the specified type
#define MEMBER_SIZE(type, member) sizeof(((type *)0)->member)

/// Maximum length of a uint32 as a string (biggest uint32 is 10 digits)
#define MAX_UINT32_STR_LEN          11U
