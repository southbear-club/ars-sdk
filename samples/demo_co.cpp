
#include "ars/sdk/schedule/co.hpp"

void fun(int a, int b) {
    printf("a = %d, b = %d\n", a, b);
    ars::sdk::yield();
    a++;
    b++;
    printf("a = %d, b = %d\n", a, b);
}

int main(int argc, const char** argv) {
    auto sch = ars::sdk::sch_ref();
    ars::sdk::new_co(fun, 1, 2);
    ars::sdk::new_co(fun, 4, 5);
    ars::sdk::new_co(fun, 7, 9);

    ars::sdk::sch_run();

    return 0;
}