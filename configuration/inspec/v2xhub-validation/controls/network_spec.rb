# Network Connectivity and Isolation Controls
# Validates network connectivity between services and network isolation

title 'Network Connectivity and Isolation'

control 'v2xhub-network-01' do
  impact 1.0
  title 'V2XHub must reach database'
  desc 'Verifies v2xhub container can connect to database service on port 3306'
  tag network: true
  tag database: true

  describe v2xhub_helper do
    it { should be_container_can_reach('v2xhub', input('mysql_host'), input('mysql_port')) }
  end
  
  describe command("docker compose exec -T v2xhub timeout 5 bash -c '</dev/tcp/#{input('mysql_host')}/#{input('mysql_port')}'") do
    its('exit_status') { should eq 0 }
  end
end

control 'v2xhub-network-02' do
  impact 1.0
  title 'PHP service must be isolated from V2XHub and database'
  desc 'Verifies php container is isolated from v2xhub and db services. Note web browsers routes WebSocket TLS connection to v2xhub service.'
  tag network: true
  tag database: true
 
  describe v2xhub_helper do
    it { should_not be_container_can_reach('php', input('mysql_host'), input('mysql_port')) }
  end
  
  describe command("docker compose exec -T php timeout 5 bash -c '</dev/tcp/#{input('mysql_host')}/#{input('mysql_port')}'") do
    its('exit_status') { should_not eq 0 }
  end

  describe v2xhub_helper do
    it { should_not be_container_can_reach('php', 'v2xhub', input('websocket_port')) }
  end
  
  describe command("docker compose exec -T php timeout 5 bash -c '</dev/tcp/v2xhub/#{input('websocket_port')}'") do
    its('exit_status') { should_not eq 0 }
  end
end

control 'v2xhub-network-03' do
  impact 1.0
  title 'The \'v2xhub_web_external\' internal network must exist and be properly configured'
  desc 'Ensures configuration_v2xhub_web_external network exists and is configured as internal'
  tag network: true

  # Validate network exists and is internal
  describe v2xhub_helper do
    it { should be_docker_network_exists('configuration_v2xhub_web_external') }
    it { should_not be_docker_network_internal('configuration_v2xhub_web_external') }
  end
  
  # Validate the network driver
  describe v2xhub_helper do
    it { should have_docker_network_and_driver('configuration_v2xhub_web_external', 'bridge') }
  end  
end

control 'v2xhub-network-04' do
  impact 1.0
  title 'The \'v2xhub_data_internal\' internal network must exist and be properly configured'
  desc 'Ensures configuration_v2xhub_data_internal network exists and is configured as internal'
  tag network: true
  tag database: true

  # Validate network exists and is internal
  describe v2xhub_helper do
    it { should be_docker_network_exists('configuration_v2xhub_data_internal') }
    it { should be_docker_network_internal('configuration_v2xhub_data_internal') }
  end
  
  # Validate the network driver
  describe v2xhub_helper do
    it { should have_docker_network_and_driver('configuration_v2xhub_data_internal', 'bridge') }
  end  
end

control 'v2xhub-network-05' do
  impact 1.0
  title 'The \'v2xhub_app_external\' internal network must exist and be properly configured'
  desc 'Ensures configuration_v2xhub_app_external network exists and is not configured as internal'
  tag network: true

  # Validate network exists and is not internal
  describe v2xhub_helper do
    it { should be_docker_network_exists('configuration_v2xhub_app_external') }
    it { should_not be_docker_network_internal('configuration_v2xhub_app_external') }
  end
  
  # Validate the network driver
  describe v2xhub_helper do
    it { should have_docker_network_and_driver('configuration_v2xhub_app_external', 'bridge') }
  end  
end

control 'v2xhub-network-06' do
  impact 0.7
  title 'php service must be connected to appropriate networks'
  desc 'Verifies php service is connected to the correct Docker networks'
  tag network: true

  # Get networks container is attached to
  networks = v2xhub_helper.container_networks('php')
  describe 'php container network attachments' do
    subject { networks }

    # Attached to correct network
    it { should include 'configuration_v2xhub_web_external' }
    it { should_not include 'configuration_v2xhub_app_external' }
    it { should_not include 'configuration_v2xhub_data_internal' }
    # Ensure host networking is not used
    it { should_not include 'host' }
  end

  describe 'php container' do
    it 'should be attached to exactly ONE networks' do
      expect(networks.length).to eq 1
    end
  end  
end

control 'v2xhub-network-07' do
  impact 0.7
  title 'v2xhub service must be connected to appropriate networks'
  desc 'Verifies v2xhub service is connected to the correct Docker networks'
  tag network: true

  # Get networks container is attached to
  networks = v2xhub_helper.container_networks('v2xhub')
  describe 'v2xhub container network attachments' do
    subject { networks }

    # Attached to correct network
    it { should_not include 'configuration_v2xhub_web_external' }
    it { should include 'configuration_v2xhub_app_external' }
    it { should include 'configuration_v2xhub_data_internal' }
    # Ensure host networking is not used
    it { should_not include 'host' }
  end

  describe 'v2xhub container' do
    it 'should be attached to exactly TWO networks' do
      expect(networks.length).to eq 2
    end
  end
end

control 'v2xhub-network-08' do
  impact 0.7
  title 'db service must be connected to appropriate networks'
  desc 'Verifies database service is connected to the correct Docker networks'
  tag network: true
  tag database: true

  # Get networks container is attached to
  networks = v2xhub_helper.container_networks('mysql')  
  describe 'mysql container network attachments' do
    subject { networks }

    # Attached to correct network
    it { should_not include 'configuration_v2xhub_app_external' }
    it { should_not include 'configuration_v2xhub_web_external' }
    it { should include 'configuration_v2xhub_data_internal' }
    # Ensure host networking is not used
    it { should_not include 'host' }
  end

  describe 'mysql container' do
    it 'should be attached to exactly ONE network' do
      expect(networks.length).to eq 1
    end
  end
end

control 'v2xhub-network-09' do
  impact 0.6
  title 'Docker networks must be properly defined'
  desc 'Verifies internal and external networks are configured in Docker Compose'
  tag network: true
  tag docker_compose: true

  describe yaml('docker-compose.yml') do
    its(['networks', 'v2xhub_web_external']) { should_not be_nil }
    its(['networks', 'v2xhub_app_external']) { should_not be_nil }
    its(['networks', 'v2xhub_data_internal']) { should_not be_nil }
    its(['networks', 'v2xhub_data_internal', 'internal']) { should eq true }
  end
end

