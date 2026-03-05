# Network Connectivity and Isolation Controls
# Validates network connectivity between services and network isolation

title 'Network Connectivity and Isolation'

control 'v2xhub-network-01' do
  impact 1.0
  title 'V2XHub must reach database'
  desc 'Verifies v2xhub container can connect to database service on port 3306'
  
  describe v2xhub_helper do
    it { should be_container_can_reach('v2xhub', input('mysql_host'), input('mysql_port')) }
  end
  
  describe command("docker compose exec -T v2xhub timeout 5 bash -c '</dev/tcp/#{input('mysql_host')}/#{input('mysql_port')}'") do
    its('exit_status') { should eq 0 }
  end
end

control 'v2xhub-network-02' do
  impact 1.0
  title 'PHP service must reach database'
  desc 'Verifies php container can connect to database service on port 3306'
  
  describe v2xhub_helper do
    it { should be_container_can_reach('php', input('mysql_host'), input('mysql_port')) }
  end
  
  describe command("docker compose exec -T php timeout 5 bash -c '</dev/tcp/#{input('mysql_host')}/#{input('mysql_port')}'") do
    its('exit_status') { should eq 0 }
  end
end

control 'v2xhub-network-03' do
  impact 0.8
  title 'PHP service should reach V2XHub WebSocket'
  desc 'Verifies php container can connect to v2xhub WebSocket service'
  
  describe v2xhub_helper do
    it { should be_container_can_reach('php', 'v2xhub', input('websocket_port')) }
  end
  
  describe command("docker compose exec -T php timeout 5 bash -c '</dev/tcp/v2xhub/#{input('websocket_port')}'") do
    its('exit_status') { should eq 0 }
  end
end

control 'v2xhub-network-04' do
  impact 1.0
  title 'Internal network must exist and be properly configured'
  desc 'Ensures v2xhub_internal network exists and is configured as internal'
  
  describe v2xhub_helper do
    it { should be_docker_network_exists('configuration_v2xhub_internal') }
    it { should be_docker_network_internal('configuration_v2xhub_internal') }
  end
  
  describe command('docker network inspect configuration_v2xhub_internal --format "{{.Internal}}"') do
    its('stdout.strip') { should eq 'true' }
    its('exit_status') { should eq 0 }
  end
end

control 'v2xhub-network-05' do
  impact 0.9
  title 'External network must exist'
  desc 'Ensures v2xhub_external network exists for external communication'
  
  describe v2xhub_helper do
    it { should be_docker_network_exists('configuration_v2xhub_external') }
  end
  
  describe command('docker network inspect configuration_v2xhub_external') do
    its('exit_status') { should eq 0 }
  end
end

control 'v2xhub-network-06' do
  impact 0.8
  title 'Network subnets must be properly configured'
  desc 'Verifies internal and external networks have correct subnet configurations'
  
  describe command('docker network inspect configuration_v2xhub_internal --format "{{range .IPAM.Config}}{{.Subnet}}{{end}}"') do
    its('stdout.strip') { should eq '172.20.0.0/16' }
    its('exit_status') { should eq 0 }
  end
  
  describe command('docker network inspect configuration_v2xhub_external --format "{{range .IPAM.Config}}{{.Subnet}}{{end}}"') do
    its('stdout.strip') { should eq '172.21.0.0/16' }
    its('exit_status') { should eq 0 }
  end
end

control 'v2xhub-network-07' do
  impact 0.7
  title 'Services must be connected to appropriate networks'
  desc 'Verifies services are connected to the correct Docker networks'
  
  # Database should only be on internal network
  describe command('docker inspect mysql --format "{{range $net, $conf := .NetworkSettings.Networks}}{{$net}} {{end}}"') do
    its('stdout') { should match(/configuration_v2xhub_internal/) }
    its('stdout') { should_not match(/configuration_v2xhub_external/) }
  end
  
  # V2XHub should be on both networks
  describe command('docker inspect v2xhub --format "{{range $net, $conf := .NetworkSettings.Networks}}{{$net}} {{end}}"') do
    its('stdout') { should match(/configuration_v2xhub_internal/) }
    its('stdout') { should match(/configuration_v2xhub_external/) }
  end
  
  # PHP should be on external network
  describe command('docker inspect php --format "{{range $net, $conf := .NetworkSettings.Networks}}{{$net}} {{end}}"') do
    its('stdout') { should match(/configuration_v2xhub_external/) }
  end
end
