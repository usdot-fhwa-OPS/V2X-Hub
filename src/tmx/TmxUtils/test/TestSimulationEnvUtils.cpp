#include <simulation/SimulationEnvUtils.h>
#include <gtest/gtest.h>
#include <stdlib.h>


TEST(testSimulationEnvUtils, is_simulation_mode_unset)
{
    // Unset any environment set SIMULATION_MODE
    unsetenv(tmx::utils::sim::SIMULATION_MODE);
    EXPECT_FALSE(tmx::utils::sim::is_simulation_mode());
}

TEST(testSimulationEnvUtils, is_simulation_mode_true) {
    setenv(tmx::utils::sim::SIMULATION_MODE, "true", 1);
    EXPECT_TRUE(tmx::utils::sim::is_simulation_mode());
    setenv(tmx::utils::sim::SIMULATION_MODE, "TRUE", 1);
    EXPECT_TRUE(tmx::utils::sim::is_simulation_mode());
}

TEST(testSimulationEnvUtils, is_simulation_mode_false) {
    setenv(tmx::utils::sim::SIMULATION_MODE, "false", 1);
    EXPECT_FALSE(tmx::utils::sim::is_simulation_mode());
    setenv(tmx::utils::sim::SIMULATION_MODE, "FALSE", 1);
    EXPECT_FALSE(tmx::utils::sim::is_simulation_mode());
}

TEST(testSimulationEnvUtils, get_sim_config_non_simulation_mode) {
    // Precondition for test (ASSERT)
    ASSERT_FALSE(tmx::utils::sim::is_simulation_mode());
    EXPECT_THROW(tmx::utils::sim::get_sim_config(tmx::utils::sim::SIMULATION_IP), tmx::TmxException );

}

TEST(testSimulationEnvUtils, get_sim_config_unset) {
    // Precondition for test (ASSERT)
    setenv(tmx::utils::sim::SIMULATION_MODE, "true", 1);
    unsetenv(tmx::utils::sim::SIMULATION_IP);
    ASSERT_TRUE(tmx::utils::sim::is_simulation_mode());

    EXPECT_THROW(tmx::utils::sim::get_sim_config(tmx::utils::sim::SIMULATION_IP), tmx::TmxException );
}

TEST(testSimulationEnvUtils, get_sim_config_set) {
    // Precondition for test (ASSERT)
    std::string simulation_ip = "127.0.0.1";
    setenv(tmx::utils::sim::SIMULATION_MODE, "true", 1);
    setenv(tmx::utils::sim::SIMULATION_IP, simulation_ip.c_str(), 1);
    ASSERT_TRUE(tmx::utils::sim::is_simulation_mode());
    
    EXPECT_EQ(tmx::utils::sim::get_sim_config(tmx::utils::sim::SIMULATION_IP), simulation_ip );
}

TEST(testSimulationEnvUtils, get_sim_config_optional_set) {
    // Precondition for test (ASSERT)
    std::string simulation_ip = "127.0.0.1";
    setenv(tmx::utils::sim::SIMULATION_MODE, "true", 1);
    setenv(tmx::utils::sim::SIMULATION_IP, simulation_ip.c_str(), 1);
    ASSERT_TRUE(tmx::utils::sim::is_simulation_mode());
    
    EXPECT_EQ(tmx::utils::sim::get_sim_config(tmx::utils::sim::SIMULATION_IP,false), simulation_ip );
}

TEST(testSimulationEnvUtils, get_sim_config_optional_unset) {
    // Precondition for test (ASSERT)
    std::string simulation_ip = "127.0.0.1";
    setenv(tmx::utils::sim::SIMULATION_MODE, "true", 1);
    unsetenv(tmx::utils::sim::SIMULATION_IP);
    ASSERT_TRUE(tmx::utils::sim::is_simulation_mode());
    
    EXPECT_TRUE(tmx::utils::sim::get_sim_config(tmx::utils::sim::SIMULATION_IP,false).empty());
}