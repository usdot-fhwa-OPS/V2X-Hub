#include <environment/EnvUtils.h>
#include <gtest/gtest.h>
#include <stdlib.h>


TEST(testEnvUtils, is_simulation_mode_unset)
{
    // Unset any environment set SIMULATION_MODE
    unsetenv(tmx::utils::environment::SIMULATION_MODE);
    EXPECT_FALSE(tmx::utils::environment::is_simulation_mode());
}

TEST(testEnvUtils, is_simulation_mode_true) {
    setenv(tmx::utils::environment::SIMULATION_MODE, "true", 1);
    EXPECT_TRUE(tmx::utils::environment::is_simulation_mode());
    setenv(tmx::utils::environment::SIMULATION_MODE, "TRUE", 1);
    EXPECT_TRUE(tmx::utils::environment::is_simulation_mode());
}

TEST(testEnvUtils, is_simulation_mode_false) {
    setenv(tmx::utils::environment::SIMULATION_MODE, "false", 1);
    EXPECT_FALSE(tmx::utils::environment::is_simulation_mode());
    setenv(tmx::utils::environment::SIMULATION_MODE, "FALSE", 1);
    EXPECT_FALSE(tmx::utils::environment::is_simulation_mode());
}

TEST(testEnvUtils, get_environment_variable_nullptr) {
    // Precondition for test (ASSERT)
    EXPECT_THROW(tmx::utils::environment::get_environment_variable(nullptr, false), tmx::TmxException );

}

TEST(testEnvUtils, get_environment_variable_unset) {
    // Precondition for test (ASSERT)
    setenv(tmx::utils::environment::SIMULATION_MODE, "true", 1);
    unsetenv(tmx::utils::environment::SIMULATION_IP);
    ASSERT_TRUE(tmx::utils::environment::is_simulation_mode());

    EXPECT_THROW(tmx::utils::environment::get_environment_variable(tmx::utils::environment::SIMULATION_IP), tmx::TmxException );
}

TEST(testEnvUtils, get_environment_variable_set) {
    // Precondition for test (ASSERT)
    std::string simulation_ip = "127.0.0.1";
    setenv(tmx::utils::environment::SIMULATION_MODE, "true", 1);
    setenv(tmx::utils::environment::SIMULATION_IP, simulation_ip.c_str(), 1);
    ASSERT_TRUE(tmx::utils::environment::is_simulation_mode());
    
    EXPECT_EQ(tmx::utils::environment::get_environment_variable(tmx::utils::environment::SIMULATION_IP), simulation_ip );
}

TEST(testEnvUtils, get_environment_variable_optional_set) {
    // Precondition for test (ASSERT)
    std::string simulation_ip = "127.0.0.1";
    setenv(tmx::utils::environment::SIMULATION_MODE, "true", 1);
    setenv(tmx::utils::environment::SIMULATION_IP, simulation_ip.c_str(), 1);
    ASSERT_TRUE(tmx::utils::environment::is_simulation_mode());
    
    EXPECT_EQ(tmx::utils::environment::get_environment_variable(tmx::utils::environment::SIMULATION_IP,false), simulation_ip );
}

TEST(testEnvUtils, get_environment_variable_optional_unset) {
    // Precondition for test (ASSERT)
    std::string simulation_ip = "127.0.0.1";
    setenv(tmx::utils::environment::SIMULATION_MODE, "true", 1);
    unsetenv(tmx::utils::environment::SIMULATION_IP);
    ASSERT_TRUE(tmx::utils::environment::is_simulation_mode());
    
    EXPECT_TRUE(tmx::utils::environment::get_environment_variable(tmx::utils::environment::SIMULATION_IP,false).empty());
}