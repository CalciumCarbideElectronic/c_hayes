#include <gtest/gtest.h>
extern "C" {
#include "semaphore.h"
#include "test_shim.h"
#include "mqueue.h"
#include "../src/chayes.h"
}

class CHayesEnv : public ::testing::Environment {
public:
    void TearDown() override {
        sem_unlink(CHAYES_URC_SEMAPHORE_NAME);
        mq_unlink(CHAYES_QUEUE_NAME);
    }
    void SetUp() override {}
};
int main(int argc, char** argv) {
    ::testing::AddGlobalTestEnvironment(new CHayesEnv  );
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

