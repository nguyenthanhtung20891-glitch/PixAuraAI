#include <climits>
#include <cstdio>

// Test-only overflow confirms that UBSan is active independently of ASan.
int main() {
    volatile int maximum = INT_MAX;
    const int overflow = maximum + 1;
    std::printf("Unexpected undetected overflow: %d\n", overflow);
    return 0;
}
