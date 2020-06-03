#include <gtest/gtest.h>
extern "C" {
#include "semaphore.h"
#include "test_shim.h"
}

class CHayes : public ::testing::Environment {
    void TearDown() override {
        printf("tear down\n");
        sem_unlink(CHAYES_URC_SEMAPHORE_NAME);
    }
    void SetUp() override {}
};
int main(int argc, char** argv) {
    CHayes env;
    //::testing::AddGlobalTestEnvironment(&env);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

