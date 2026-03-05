# WebSocket Proxy Configuration Controls
# Validates Apache WebSocket reverse proxy configuration

title 'WebSocket Proxy Configuration'

control 'v2xhub-websocket-01' do
  impact 0.8
  title 'Apache configuration file must exist'
  desc 'Ensures Apache configuration file exists in the PHP container'
  
  describe v2xhub_helper do
    it { should be_container_file_exists('php', '/etc/apache2/sites-available/000-default.conf') }
  end
  
  describe command('docker compose exec -T php test -f /etc/apache2/sites-available/000-default.conf') do
    its('exit_status') { should eq 0 }
  end
end

control 'v2xhub-websocket-02' do
  impact 0.9
  title 'WebSocket reverse proxy configuration must be present'
  desc 'Verifies Apache has WebSocket proxy configuration for v2xhub-ws'
  
  describe v2xhub_helper do
    it { should be_container_file_contains('php', '/etc/apache2/sites-available/000-default.conf', 'ProxyPass.*v2xhub-ws.*ws://') }
  end
  
  describe command('docker compose exec -T php grep "ProxyPass.*v2xhub-ws.*ws://" /etc/apache2/sites-available/000-default.conf') do
    its('exit_status') { should eq 0 }
    its('stdout') { should match(/ProxyPass.*v2xhub-ws.*ws:\/\//) }
  end
end

control 'v2xhub-websocket-03' do
  impact 0.8
  title 'Apache proxy modules must be loaded'
  desc 'Ensures required Apache proxy modules are loaded for WebSocket support'
  
  describe v2xhub_helper do
    it { should be_apache_module_loaded('php', 'proxy_module') }
    it { should be_apache_module_loaded('php', 'proxy_wstunnel_module') }
  end
  
  describe command('docker compose exec -T php apache2ctl -M') do
    its('stdout') { should match(/proxy_module/) }
    its('stdout') { should match(/proxy_wstunnel_module/) }
    its('exit_status') { should eq 0 }
  end
end

control 'v2xhub-websocket-04' do
  impact 0.7
  title 'WebSocket host environment variable must be set'
  desc 'Verifies V2XHUB_WS_HOST environment variable is configured in PHP container'
  
  describe v2xhub_helper.container_env_var('php', 'V2XHUB_WS_HOST') do
    it { should_not be_nil }
    it { should_not be_empty }
  end
  
  describe command('docker compose exec -T php printenv V2XHUB_WS_HOST') do
    its('exit_status') { should eq 0 }
    its('stdout.strip') { should_not be_empty }
  end
end

control 'v2xhub-websocket-05' do
  impact 0.7
  title 'WebSocket port environment variable must be set'
  desc 'Verifies V2XHUB_WS_PORT environment variable is configured in PHP container'
  
  describe v2xhub_helper.container_env_var('php', 'V2XHUB_WS_PORT') do
    it { should_not be_nil }
    it { should_not be_empty }
    it { should eq input('websocket_port').to_s }
  end
  
  describe command('docker compose exec -T php printenv V2XHUB_WS_PORT') do
    its('exit_status') { should eq 0 }
    its('stdout.strip') { should eq input('websocket_port').to_s }
  end
end

control 'v2xhub-websocket-06' do
  impact 0.6
  title 'WebSocket environment variables must match Docker Compose configuration'
  desc 'Ensures WebSocket environment variables in PHP container match compose configuration'
  
  describe yaml('docker-compose.yml') do
    its(['services', 'php', 'environment']) { should include 'V2XHUB_WS_HOST=${V2XHUB_WS_HOST:-v2xhub}' }
    its(['services', 'php', 'environment']) { should include 'V2XHUB_WS_PORT=${COMMAND_WS_PORT:-19760}' }
  end
end

control 'v2xhub-websocket-07' do
  impact 0.8
  title 'WebSocket port must not be exposed externally'
  desc 'Ensures WebSocket port 19760 is not directly accessible from outside Docker network'
  
  describe port(input('websocket_port')) do
    it { should_not be_listening }
  end
  
  # Check that the port is not mapped externally via Docker
  describe command("docker compose port v2xhub #{input('websocket_port')}") do
    its('stdout.strip') { should be_empty.or match(/no port/) }
  end
end

control 'v2xhub-websocket-08' do
  impact 0.6
  title 'Apache must be configured to handle WebSocket upgrades'
  desc 'Verifies Apache configuration includes WebSocket upgrade handling'
  
  # Look for common WebSocket proxy configuration patterns
  describe command('docker compose exec -T php grep -i "upgrade" /etc/apache2/sites-available/000-default.conf') do
    its('exit_status') { should eq 0 }
  end
end

control 'v2xhub-websocket-09' do
  impact 0.5
  title 'WebSocket service must be accessible via reverse proxy path'
  desc 'Ensures WebSocket service is accessible through the configured proxy path'
  
  # This test checks if the proxy configuration includes the expected path
  describe command('docker compose exec -T php grep "v2xhub-ws" /etc/apache2/sites-available/000-default.conf') do
    its('exit_status') { should eq 0 }
    its('stdout') { should match(/v2xhub-ws/) }
  end
end
