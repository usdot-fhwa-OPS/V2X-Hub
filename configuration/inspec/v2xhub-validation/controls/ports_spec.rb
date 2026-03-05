# Port Exposure and Security Controls
# Validates port exposure configuration and security

title 'Port Exposure and Security'

control 'v2xhub-ports-01' do
  impact 1.0
  title 'Core web ports must be exposed'
  desc 'Ensures HTTP and HTTPS ports are accessible for web interface'
  
  describe v2xhub_helper do
    it { should be_port_exposed_externally('php', input('web_http_port')) }
    it { should be_port_exposed_externally('php', input('web_https_port')) }
  end
  
  describe command("docker compose port php #{input('web_http_port')}") do
    its('exit_status') { should eq 0 }
    its('stdout.strip') { should_not be_empty }
    its('stdout.strip') { should_not eq ':0' }
  end
  
  describe command("docker compose port php #{input('web_https_port')}") do
    its('exit_status') { should eq 0 }
    its('stdout.strip') { should_not be_empty }
    its('stdout.strip') { should_not eq ':0' }
  end
end

control 'v2xhub-ports-02' do
  impact 1.0
  title 'V2X message port must be exposed'
  desc 'Ensures V2X message port 8686 is accessible for V2X communication'
  
  describe v2xhub_helper do
    it { should be_port_exposed_externally('v2xhub', input('v2x_port')) }
  end
  
  describe command("docker compose port v2xhub #{input('v2x_port')}") do
    its('exit_status') { should eq 0 }
    its('stdout.strip') { should_not be_empty }
    its('stdout.strip') { should_not eq ':0' }
  end
end

control 'v2xhub-ports-03' do
  impact 1.0
  title 'Database port must NOT be exposed externally'
  desc 'Ensures MySQL port 3306 is not accessible from outside Docker network (critical security)'
  
  describe port(input('mysql_port')) do
    it { should_not be_listening }
  end
  
  describe v2xhub_helper do
    it { should_not be_port_exposed_externally('db', input('mysql_port')) }
  end
  
  describe command("docker compose port db #{input('mysql_port')}") do
    its('stdout.strip') { should be_empty.or match(/no port/) }
  end
end

control 'v2xhub-ports-04' do
  impact 1.0
  title 'WebSocket port must NOT be exposed externally'
  desc 'Ensures WebSocket port 19760 is not directly accessible (security - should use reverse proxy)'
  
  describe port(input('websocket_port')) do
    it { should_not be_listening }
  end
  
  describe v2xhub_helper do
    it { should_not be_port_exposed_externally('v2xhub', input('websocket_port')) }
  end
  
  describe command("docker compose port v2xhub #{input('websocket_port')}") do
    its('stdout.strip') { should be_empty.or match(/no port/) }
  end
end

control 'v2xhub-ports-05' do
  impact 0.8
  title 'Core ports must be properly configured in Docker Compose'
  desc 'Verifies port mappings are correctly defined in docker-compose.yml'
  
  describe yaml('docker-compose.yml') do
    its(['services', 'php', 'ports']) { should include '${WEB_HTTP_PORT:-80}:80' }
    its(['services', 'php', 'ports']) { should include '${WEB_HTTPS_PORT:-443}:443' }
    its(['services', 'v2xhub', 'ports']) { should include '${V2X_PORT:-8686}:8686' }
  end
end

control 'v2xhub-ports-06' do
  impact 0.9
  title 'Database container must have no port mappings'
  desc 'Ensures database service has no external port exposure configured'
  
  describe yaml('docker-compose.yml') do
    its(['services', 'db', 'ports']) { should be_nil }
  end
  
  describe docker_container('mysql') do
    its('ports') { should be_empty }
  end
end

control 'v2xhub-ports-07' do
  impact 0.7
  title 'V2XHub extensive port ranges should be properly configured'
  desc 'Verifies V2XHub service has appropriate port ranges for various plugins'
  
  # Check that V2XHub has multiple port ranges configured
  describe yaml('docker-compose.yml') do
    v2xhub_ports = subject['services']['v2xhub']['ports']
    
    # Should have V2X port
    describe v2xhub_ports do
      it { should include '${V2X_PORT:-8686}:8686' }
    end
    
    # Should have WebSocket port (even if not exposed externally)
    describe v2xhub_ports do
      it { should include '${COMMAND_WS_PORT:-19760}:19760' }
    end
    
    # Should have some plugin-specific port ranges
    describe v2xhub_ports.length do
      it { should be > 5 }  # Should have multiple port mappings
    end
  end
end

control 'v2xhub-ports-08' do
  impact 0.6
  title 'Port exposure should follow security best practices'
  desc 'Ensures only necessary ports are exposed and follow security guidelines'
  
  # Check that sensitive ports are not exposed on all interfaces
  describe command('netstat -tuln 2>/dev/null | grep ":3306 "') do
    its('stdout') { should be_empty }
  end
  
  # Verify web ports are accessible
  [input('web_http_port'), input('web_https_port')].each do |port|
    describe command("netstat -tuln 2>/dev/null | grep \":#{port} \"") do
      its('exit_status') { should eq 0 }
    end
  end
end

control 'v2xhub-ports-09' do
  impact 0.5
  title 'Port ranges should be organized by functionality'
  desc 'Verifies that V2XHub port ranges are logically organized'
  
  describe yaml('docker-compose.yml') do
    v2xhub_ports = subject['services']['v2xhub']['ports']
    
    # Should have SNMP ports (161)
    describe v2xhub_ports.join(' ') do
      it { should match(/161.*udp/) }
    end
    
    # Should have simulation ports (5757, 6767, 7575)
    describe v2xhub_ports.join(' ') do
      it { should match(/5750-5760/) }
      it { should match(/6760-6770/) }
      it { should match(/7570-7580/) }
    end
    
    # Should have ODE ports (44xxx, 46xxx, 47xxx)
    describe v2xhub_ports.join(' ') do
      it { should match(/44900-44930.*udp/) }
      it { should match(/46790-46810.*udp/) }
      it { should match(/47890-47910.*udp/) }
    end
  end
end

control 'v2xhub-ports-10' do
  impact 0.4
  title 'Port documentation should be present in compose file'
  desc 'Ensures port mappings are documented with comments in docker-compose.yml'
  
  describe file('docker-compose.yml') do
    its('content') { should match(/# V2X Messages/) }
    its('content') { should match(/# CommandPlugin WebSocket/) }
    its('content') { should match(/# HTTP.*redirects to HTTPS/) }
    its('content') { should match(/# HTTPS Web UI/) }
  end
end
