#include "aru/components/evpp/EventLoopThread.hpp"
#include "aru/sdk/memory/mem.hpp"

using namespace aru::evpp;

static void onTimer(TimerID timerID, int n) {
    printf("tid=%ld timerID=%lu time=%lus n=%d\n", aru::sdk::gettid(), (unsigned long)timerID, (unsigned long)time(NULL), n);
}

int main(int argc, char* argv[]) {
    aru::sdk::memcheck_register();

    printf("main tid=%ld\n", aru::sdk::gettid());

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
        printf("queueInLoop tid=%ld\n", aru::sdk::gettid());
    });

    loop->runInLoop([](){
        printf("runInLoop tid=%ld\n", aru::sdk::gettid());
    });

    // wait loop_thread exit
    loop_thread.join();

    return 0;
}
