
/// uuid-demo

#include "demo_log.hpp"
#include "ars/sdk/crypto/uuid.hpp"

int main(void) {
    char s[37] = "";
    ars::sdk::uuid_generate(s);
    DEMO_LOG(INFO) << "uuid : " << s << std::endl;

    return 0;
}