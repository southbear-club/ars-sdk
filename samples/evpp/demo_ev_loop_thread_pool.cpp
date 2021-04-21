#include "aru/components/evpp/EventLoopThreadPool.hpp"
#include "aru/sdk/memory/mem.hpp"

using namespace aru::evpp;

static void onTimer(TimerID timerID, int n) {
    printf("tid=%ld timerID=%lu time=%lus n=%d\n", aru::sdk::gettid(), (unsigned long)timerID, (unsigned long)time(NULL), n);
}

int main(int argc, char* argv[]) {
    aru::sdk::memcheck_register();

    printf("main tid=%ld\n", aru::sdk::gettid());

    EventLoopThreadPool loop_threads(4);
    loop_threads.start(true);

    int thread_num = loop_threads.threadNum();
    for (int i = 0; i < thread_num; ++i) {
        EventLoopPtr loop = loop_threads.nextLoop();
        printf("worker[%d] tid=%ld\n", i, loop->tid());

        loop->runInLoop([loop](){
            // runEvery 1s
            loop->setInterval(1000, std::bind(onTimer, std::placeholders::_1, 100));
        });

        loop->queueInLoop([](){
            printf("queueInLoop tid=%ld\n", aru::sdk::gettid());
        });

        loop->runInLoop([](){
            printf("runInLoop tid=%ld\n", aru::sdk::gettid());
        });
    }

    // runAfter 10s
    loop_threads.loop()->setTimeout(10000, [&loop_threads](TimerID timerID){
        loop_threads.stop(false);
    });

    // wait loop_threads exit
    loop_threads.join();

    return 0;
}
