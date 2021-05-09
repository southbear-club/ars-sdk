
#include "aru/sdk/schedule/co.hpp"

void fun(int a, int b) {
    printf("a = %d, b = %d\n", a, b);
    aru::sdk::yield();
    a++;
    b++;
    printf("a = %d, b = %d\n", a, b);
}

int main(int argc, const char** argv) {
    auto sch = aru::sdk::sch_ref();
    aru::sdk::new_co(fun, 1, 2);
    aru::sdk::new_co(fun, 4, 5);
    aru::sdk::new_co(fun, 7, 9);

    aru::sdk::sch_run();

    return 0;
}