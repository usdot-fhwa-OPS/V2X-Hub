# Docker Compose Validation Controls
# Validates Docker Compose accessibility and service definitions

title 'Docker Compose Configuration'

control 'v2xhub-docker-01' do
  impact 1.0
  title 'Docker Compose must be accessible'
  desc 'Ensures Docker Compose is running and accessible for service management'
  
  describe command('docker compose ps') do
    its('exit_status') { should eq 0 }
  end
  
  describe v2xhub_helper do
    it { should be_docker_compose_accessible }
  end
end

control 'v2xhub-docker-02' do
  impact 0.8
  title 'Docker Compose configuration file must exist'
  desc 'Verifies the docker-compose.yml file exists and is readable'
  
  describe file('docker-compose.yml') do
    it { should exist }
    it { should be_readable }
    its('content') { should match(/services:/) }
  end
end

control 'v2xhub-docker-03' do
  impact 0.7
  title 'Required services must be defined in Docker Compose'
  desc 'Ensures core V2X-Hub services are defined in the compose configuration'
  
  describe yaml('docker-compose.yml') do
    its(['services', 'db']) { should_not be_nil }
    its(['services', 'php']) { should_not be_nil }
    its(['services', 'v2xhub']) { should_not be_nil }
  end
end

control 'v2xhub-docker-04' do
  impact 0.6
  title 'Docker networks must be properly defined'
  desc 'Verifies internal and external networks are configured in Docker Compose'
  
  describe yaml('docker-compose.yml') do
    its(['networks', 'v2xhub_internal']) { should_not be_nil }
    its(['networks', 'v2xhub_external']) { should_not be_nil }
    its(['networks', 'v2xhub_internal', 'internal']) { should eq true }
  end
end

control 'v2xhub-docker-05' do
  impact 0.5
  title 'Docker secrets must be configured'
  desc 'Ensures MySQL password is managed via Docker secrets'
  
  describe yaml('docker-compose.yml') do
    its(['secrets', 'mysql_password']) { should_not be_nil }
    its(['services', 'db', 'secrets']) { should include 'mysql_password' }
    its(['services', 'v2xhub', 'secrets']) { should include 'mysql_password' }
  end
end
