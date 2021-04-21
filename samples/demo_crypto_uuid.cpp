
/// uuid-demo

#include "demo_log.hpp"
#include "aru/sdk/crypto/uuid.hpp"

int main(void) {
    char s[37] = "";
    aru::sdk::uuid_generate(s);
    DEMO_LOG(INFO) << "uuid : " << s << std::endl;

    return 0;
}