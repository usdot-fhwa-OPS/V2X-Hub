# Service Status Validation Controls
# Validates that core V2X-Hub services are running properly

title 'Service Status Validation'

control 'v2xhub-services-01' do
  impact 1.0
  title 'Database service must be running'
  desc 'Ensures the MySQL database service is running and accessible'
  
  describe v2xhub_helper do
    it { should be_service_running('db') }
  end
  
  describe docker_container('mysql') do
    it { should exist }
    it { should be_running }
  end
end

control 'v2xhub-services-02' do
  impact 1.0
  title 'PHP web service must be running'
  desc 'Ensures the PHP web interface service is running'
  
  describe v2xhub_helper do
    it { should be_service_running('php') }
  end
  
  describe docker_container('php') do
    it { should exist }
    it { should be_running }
  end
end

control 'v2xhub-services-03' do
  impact 1.0
  title 'V2XHub core service must be running'
  desc 'Ensures the main V2XHub service is running'
  
  describe v2xhub_helper do
    it { should be_service_running('v2xhub') }
  end
  
  describe docker_container('v2xhub') do
    it { should exist }
    it { should be_running }
  end
end

control 'v2xhub-services-04' do
  impact 0.8
  title 'All core services must be in running state'
  desc 'Verifies that all essential services are actively running'
  
  %w[db php v2xhub].each do |service|
    describe command("docker compose ps --services --filter 'status=running'") do
      its('stdout') { should match(/^#{service}$/) }
    end
  end
end

control 'v2xhub-services-05' do
  impact 0.7
  title 'Services must have proper restart policies'
  desc 'Ensures services are configured to restart automatically'
  
  describe yaml('docker-compose.yml') do
    its(['services', 'php', 'restart']) { should eq 'always' }
    its(['services', 'v2xhub', 'restart']) { should eq 'always' }
  end
end

control 'v2xhub-services-06' do
  impact 0.6
  title 'Service dependencies must be properly configured'
  desc 'Verifies that service startup dependencies are correctly defined'
  
  describe yaml('docker-compose.yml') do
    its(['services', 'php', 'depends_on']) { should include 'db' }
    its(['services', 'php', 'depends_on']) { should include 'v2xhub' }
    its(['services', 'v2xhub', 'depends_on', 'db', 'condition']) { should eq 'service_healthy' }
  end
end
