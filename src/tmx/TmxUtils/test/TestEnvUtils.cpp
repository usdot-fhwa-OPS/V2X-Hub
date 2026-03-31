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
    EXPECT_THROW(tmx::utils::environment::get_environment_variable(nullptr, false), tmx::TmxException );
}

TEST(testEnvUtils, get_environment_variable_unset) {
    // Precondition for test (ASSERT)
    unsetenv(tmx::utils::environment::SIMULATION_IP);

    EXPECT_THROW(tmx::utils::environment::get_environment_variable(tmx::utils::environment::SIMULATION_IP), tmx::TmxException );
}

TEST(testEnvUtils, get_environment_variable_set) {
    std::string simulation_ip = "127.0.0.1";
    setenv(tmx::utils::environment::SIMULATION_IP, simulation_ip.c_str(), 1);
    
    EXPECT_EQ(tmx::utils::environment::get_environment_variable(tmx::utils::environment::SIMULATION_IP), simulation_ip );
}

TEST(testEnvUtils, get_environment_variable_optional_set) {
    std::string simulation_ip = "127.0.0.1";
    setenv(tmx::utils::environment::SIMULATION_IP, simulation_ip.c_str(), 1);
    
    EXPECT_EQ(tmx::utils::environment::get_environment_variable(tmx::utils::environment::SIMULATION_IP,false), simulation_ip );
}

TEST(testEnvUtils, get_environment_variable_optional_unset) {
    std::string simulation_ip = "127.0.0.1";
    unsetenv(tmx::utils::environment::SIMULATION_IP);
    
    EXPECT_TRUE(tmx::utils::environment::get_environment_variable(tmx::utils::environment::SIMULATION_IP,false).empty());
}

TEST(testEnvUtils, get_local_ip_unset) {
    unsetenv(tmx::utils::environment::LOCAL_IP);
    EXPECT_THROW(tmx::utils::environment::get_local_ip(), tmx::TmxException );
}

TEST(testEnvUtils, get_local_ip_set) {
    std::string local_ip = "0.0.0.0";
    setenv(tmx::utils::environment::LOCAL_IP, local_ip.c_str(), 1);
    EXPECT_EQ(tmx::utils::environment::get_local_ip(), local_ip);
}