// The program CMake builds with RBLXCC instead of add_executable.
//
// It is deliberately plain: two translation units, one function declared here and
// defined in totals.cpp, linked the same way a C compiler driver links. The
// language it uses is what src/Cxx*.cpp understands today, which is why this
// example exists next to the real targets rather than inside one of them.

int printf(char* format, ...);

int totals(int limit);

int main() {
    int sum = totals(4);
    printf("total=%d\n", sum);
    return sum == 10 ? 0 : 1;
}
