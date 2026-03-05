# Environment Variables Validation Controls
# Validates required environment variables and configuration

title 'Environment Variables Validation'

control 'v2xhub-env-01' do
  impact 0.9
  title 'V2XHub MySQL connection environment variables must be set'
  desc 'Ensures V2XHub container has required MySQL connection environment variables'
  
  required_vars = {
    'MYSQL_HOST' => input('mysql_host'),
    'MYSQL_PORT' => input('mysql_port').to_s,
    'MYSQL_DATABASE' => input('mysql_database'),
    'MYSQL_USER' => input('mysql_user')
  }
  
  required_vars.each do |var, expected_value|
    describe v2xhub_helper.container_env_var('v2xhub', var) do
      it { should eq expected_value }
    end
    
    describe command("docker compose exec -T v2xhub printenv #{var}") do
      its('exit_status') { should eq 0 }
      its('stdout.strip') { should eq expected_value }
    end
  end
end

control 'v2xhub-env-02' do
  impact 1.0
  title 'MySQL password secret file must exist in V2XHub container'
  desc 'Verifies MySQL password is accessible via Docker secrets in V2XHub container'
  
  describe v2xhub_helper do
    it { should be_container_file_exists('v2xhub', '/run/secrets/mysql_password') }
  end
  
  describe command('docker compose exec -T v2xhub test -f /run/secrets/mysql_password') do
    its('exit_status') { should eq 0 }
  end
  
  # Verify the MYSQL_PASSWORD environment variable points to the secrets file
  describe v2xhub_helper.container_env_var('v2xhub', 'MYSQL_PASSWORD') do
    it { should eq '/run/secrets/mysql_password' }
  end
end

control 'v2xhub-env-03' do
  impact 0.7
  title 'V2XHub core configuration environment variables must be set'
  desc 'Ensures V2XHub container has required core configuration variables'
  
  core_vars = [
    'INFRASTRUCTURE_ID',
    'INFRASTRUCTURE_NAME',
    'V2XHUB_USERNAME',
    'V2X_PORT',
    'COMMAND_WS_PORT'
  ]
  
  core_vars.each do |var|
    describe v2xhub_helper.container_env_var('v2xhub', var) do
      it { should_not be_nil }
      it { should_not be_empty }
    end
    
    describe command("docker compose exec -T v2xhub printenv #{var}") do
      its('exit_status') { should eq 0 }
      its('stdout.strip') { should_not be_empty }
    end
  end
end

control 'v2xhub-env-04' do
  impact 0.6
  title 'V2XHub port configuration must match expected values'
  desc 'Verifies V2XHub port environment variables match expected configuration'
  
  describe v2xhub_helper.container_env_var('v2xhub', 'V2X_PORT') do
    it { should eq input('v2x_port').to_s }
  end
  
  describe v2xhub_helper.container_env_var('v2xhub', 'COMMAND_WS_PORT') do
    it { should eq input('websocket_port').to_s }
  end
end

control 'v2xhub-env-05' do
  impact 0.8
  title 'V2XHub environment variables must match Docker Compose configuration'
  desc 'Ensures environment variables in V2XHub container match compose file definitions'
  
  describe yaml('docker-compose.yml') do
    its(['services', 'v2xhub', 'environment']) { should include 'MYSQL_HOST=${MYSQL_HOST:-db}' }
    its(['services', 'v2xhub', 'environment']) { should include 'MYSQL_PORT=${MYSQL_PORT:-3306}' }
    its(['services', 'v2xhub', 'environment']) { should include 'MYSQL_DATABASE=${MYSQL_DATABASE:-IVP}' }
    its(['services', 'v2xhub', 'environment']) { should include 'MYSQL_USER=${MYSQL_USER:-IVP}' }
    its(['services', 'v2xhub', 'environment']) { should include 'MYSQL_PASSWORD=/run/secrets/mysql_password' }
  end
end

control 'v2xhub-env-06' do
  impact 0.5
  title 'Optional V2XHub configuration variables should be set'
  desc 'Verifies optional but recommended V2XHub environment variables are configured'
  
  optional_vars = [
    'SIMULATION_MODE',
    'LOCAL_IP',
    'SENSOR_JSON_FILE_PATH'
  ]
  
  optional_vars.each do |var|
    describe command("docker compose exec -T v2xhub printenv #{var}") do
      its('exit_status') { should eq 0 }
    end
  end
end

control 'v2xhub-env-07' do
  impact 0.6
  title 'PHP container WebSocket environment variables must be properly configured'
  desc 'Ensures PHP container has correct WebSocket connection environment variables'
  
  describe v2xhub_helper.container_env_var('php', 'V2XHUB_WS_HOST') do
    it { should_not be_nil }
    it { should_not be_empty }
  end
  
  describe v2xhub_helper.container_env_var('php', 'V2XHUB_WS_PORT') do
    it { should eq input('websocket_port').to_s }
  end
end

control 'v2xhub-env-08' do
  impact 0.4
  title 'Environment variable defaults should be reasonable'
  desc 'Verifies that default values for environment variables are appropriate'
  
  # Check simulation mode default
  simulation_mode = v2xhub_helper.container_env_var('v2xhub', 'SIMULATION_MODE')
  describe simulation_mode do
    it { should match(/^(true|false)$/i) }
  end
  
  # Check local IP is set
  local_ip = v2xhub_helper.container_env_var('v2xhub', 'LOCAL_IP')
  describe local_ip do
    it { should match(/^\d+\.\d+\.\d+\.\d+$/) }
  end
end
