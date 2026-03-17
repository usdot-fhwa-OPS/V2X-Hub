# Container Health Status Controls
# Validates container health checks and overall system health

title 'Container Health Status'

control 'v2xhub-health-01' do
  impact 0.9
  title 'Database container must be healthy'
  desc 'Ensures MySQL database container passes health checks'
  tag health: true

  containers = docker.containers.where { names == 'mysql' }
  # Existence check 
  describe 'mysql container existence' do
    subject { containers.ids }
    it { should_not be_empty }
  end

  containers.ids.each do |id|
    # call docker inspect to retrieve detailed information about the container
    describe 'mysql container' do
      subject { docker.object(id) }
      its('State.Status') { should eq 'running' }
      its('State.Health.Status') { should eq 'healthy' }
    end
  end
end

control 'v2xhub-health-02' do
  impact 0.8
  title 'V2XHub container should be healthy if health check is configured'
  desc 'Verifies V2XHub container health status when health check is available'
  tag health: true

  containers = docker.containers.where { names == 'v2xhub' }
  # Existence check 
  describe 'v2xhub container existence' do
    subject { containers.ids }
    it { should_not be_empty }
  end

  containers.ids.each do |id|
    # call docker inspect to retrieve detailed information about the container
    describe 'v2xhub container' do
      subject { docker.object(id) }
      its('State.Status') { should eq 'running' }
    end
  end
end

control 'v2xhub-health-03' do
  impact 0.7
  title 'PHP container should be running and responsive'
  desc 'Ensures PHP web service container is running properly'
  tag health: true

  containers = docker.containers.where { names == 'php' }
  # Existence check 
  describe 'php container existence' do
    subject { containers.ids }
    it { should_not be_empty }
  end

  containers.ids.each do |id|
    # call docker inspect to retrieve detailed information about the container
    describe 'php container' do
      subject { docker.object(id) }
      its('State.Status') { should eq 'running' }
    end
  end
end

control 'v2xhub-health-04' do
  impact 0.8
  title 'Database health check must be properly configured'
  desc 'Verifies database service has appropriate health check settings'
  tag health: true
  
  describe yaml('docker-compose.yml') do
    its(['services', 'db', 'healthcheck', 'test']) { should_not be_nil }
    its(['services', 'db', 'healthcheck', 'interval']) { should eq '30s' }
    its(['services', 'db', 'healthcheck', 'timeout']) { should eq '10s' }
    its(['services', 'db', 'healthcheck', 'retries']) { should eq 3 }
    its(['services', 'db', 'healthcheck', 'start_period']) { should eq '40s' }
  end
end

control 'v2xhub-health-05' do
  impact 0.7
  title 'Container resource limits should be properly configured'
  desc 'Verifies containers have appropriate resource constraints'
  tag health: true
  
  describe yaml('docker-compose.yml') do
    # V2XHub should have memory limits
    its(['services', 'v2xhub', 'deploy', 'resources', 'limits', 'memory']) { should eq '2g' }
    its(['services', 'v2xhub', 'deploy', 'resources', 'reservations', 'memory']) { should eq '1g' }
    
    # V2XHub should have ulimits configured
    its(['services', 'v2xhub', 'ulimits', 'nofile', 'soft']) { should eq 2560 }
    its(['services', 'v2xhub', 'ulimits', 'nofile', 'hard']) { should eq 2560 }
  end
end

control 'v2xhub-health-06' do
  impact 0.5
  title 'Container uptime should indicate stability'
  desc 'Verifies containers have been running for a reasonable time without restarts'
  tag health: true
  
  %w[mysql php v2xhub].each do |container|
    describe command("docker inspect #{container} --format '{{.State.Status}}'") do
      its('stdout.strip') { should eq 'running' }
    end
    
    # Check that container hasn't been restarting frequently
    describe command("docker inspect #{container} --format '{{.RestartCount}}'") do
      its('stdout.strip.to_i') { should be < 5 }
    end
  end
end

control 'v2xhub-health-07' do
  impact 0.6
  title 'Service dependencies must be satisfied'
  desc 'Ensures service startup order and dependencies are working correctly'
  tag health: true
  
  # V2XHub should start after database is healthy
  describe yaml('docker-compose.yml') do
    its(['services', 'v2xhub', 'depends_on', 'db', 'condition']) { should eq 'service_healthy' }
  end
  
  # PHP should depend on both db and v2xhub
  describe yaml('docker-compose.yml') do
    its(['services', 'php', 'depends_on']) { should include 'db' }
    its(['services', 'php', 'depends_on']) { should include 'v2xhub' }
  end
end

control 'v2xhub-health-08' do
  impact 0.4
  title 'Container logs should not show critical errors'
  desc 'Verifies recent container logs do not contain critical error patterns'
  tag health: true
  
  %w[db php v2xhub].each do |service|
    describe command("docker compose logs --tail=50 #{service}") do
      its('exit_status') { should eq 0 }
      # Should not contain common critical error patterns
      its('stdout') { should_not match(/FATAL|CRITICAL|ERROR.*failed to start/i) }
    end
  end
end

control 'v2xhub-health-09' do
  impact 0.5
  title 'System should have adequate resources available'
  desc 'Verifies system has sufficient resources for V2X-Hub operation'
  tag health: true
  
  # Check available disk space (should have at least 1GB free)
  describe command('df -BG . | tail -1 | awk \'{print $4}\' | sed \'s/G//\'') do
    its('stdout.strip.to_i') { should be >= 1 }
  end
  
  # Check available memory (should have some free memory)
  describe command('free -m | grep "^Mem:" | awk \'{print $7}\'') do
    its('stdout.strip.to_i') { should be > 100 }
  end
end

control 'v2xhub-health-10' do
  impact 0.7
  title 'Services must have proper restart policies'
  desc 'Ensures services are configured to restart automatically'
  tag health: true

  describe yaml('docker-compose.yml') do
    its(['services', 'php', 'restart']) { should eq 'always' }
    its(['services', 'v2xhub', 'restart']) { should eq 'always' }
  end
end

control 'v2xhub-health-11' do
  impact 0.6
  title 'Service dependencies must be properly configured'
  desc 'Verifies that service startup dependencies are correctly defined'
  tag health: true

  describe yaml('docker-compose.yml') do
    its(['services', 'php', 'depends_on']) { should include 'db' }
    its(['services', 'php', 'depends_on']) { should include 'v2xhub' }
    its(['services', 'v2xhub', 'depends_on', 'db', 'condition']) { should eq 'service_healthy' }
  end
end

control 'v2xhub-health-12' do
  impact 0.7
  title 'Required services must be defined in Docker Compose'
  desc 'Ensures core V2X-Hub services are defined in the compose configuration'
  tag health: true
  tag docker_compose: true

  describe yaml('docker-compose.yml') do
    its(['services', 'db']) { should_not be_nil }
    its(['services', 'php']) { should_not be_nil }
    its(['services', 'v2xhub']) { should_not be_nil }
  end
end