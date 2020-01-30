// Force a compilation error if condition is true */
#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)]))

// Calculates the number of elements contained in an array
#define ARRAY_SIZE(x) (uint32_t)((sizeof (x)) / (sizeof *(x)))

// Gets the size of a struct member of the specified type
#define MEMBER_SIZE(type, member) sizeof(((type *)0)->member)
