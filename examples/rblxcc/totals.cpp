// The second translation unit of the example, compiled on its own by a separate
// RBLXCC invocation and linked with main.cpp afterwards.

int totals(int limit) {
    int sum = 0;
    for (int i = 1; i <= limit; i++) sum += i;
    return sum;
}
