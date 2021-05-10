#include "ars/sdk/evpp/EventLoopThread.hpp"
#include "ars/sdk/memory/mem.hpp"

using namespace ars::evpp;

static void onTimer(TimerID timerID, int n) {
    printf("tid=%ld timerID=%lu time=%lus n=%d\n", ars::sdk::gettid(), (unsigned long)timerID, (unsigned long)time(NULL), n);
}

int main(int argc, char* argv[]) {
    ars::sdk::memcheck_register();

    printf("main tid=%ld\n", ars::sdk::gettid());

    EventLoopThread loop_thread;
    EventLoopPtr loop = loop_thread.loop();

    // runEvery 1s
    loop->setInterval(1000, std::bind(onTimer, std::placeholders::_1, 100));

    // runAfter 10s
    loop->setTimeout(10000, [&loop](TimerID timerID){
        loop->stop();
    });

    loop_thread.start();

    loop->queueInLoop([](){
        printf("queueInLoop tid=%ld\n", ars::sdk::gettid());
    });

    loop->runInLoop([](){
        printf("runInLoop tid=%ld\n", ars::sdk::gettid());
    });

    // wait loop_thread exit
    loop_thread.join();

    return 0;
}
