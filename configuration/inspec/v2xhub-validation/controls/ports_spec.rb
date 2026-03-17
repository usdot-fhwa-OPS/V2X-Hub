# Port Exposure and Security Controls
# Validates port exposure configuration and security

title 'Port Exposure and Security'

control 'v2xhub-ports-00' do
  impact 1.0
  title 'Sanity check docker compose must be accessible'
  desc 'Ensures Docker Compose is running and accessible for service management'
  tag docker_compose: true
  tag network: true

  describe v2xhub_helper do
    it { should be_docker_compose_accessible }
  end
end

control 'v2xhub-ports-01' do
  impact 1.0
  title 'Verify php specific ports must be exposed externally'
  desc 'Ensures php has only specified HTTP and HTTPS ports exposed'  
  tag network: true

  # Check ports exposed on host network
  describe port(input('web_http_port')) do
    its('protocols') { should include 'tcp' }
    its('addresses') { should include '0.0.0.0' }
  end
  describe port(input('web_https_port')) do
    its('protocols') { should include 'tcp' }
    its('addresses') { should include '0.0.0.0' }
  end
  
  # Check ports published by docker compose
  describe v2xhub_helper do
    it { should have_docker_compose_exposed_port('php', input('web_http_port')) }
    it { should have_docker_compose_exposed_port('php', input('web_https_port')) } 
  end

  # Check ports published on container
  ports = v2xhub_helper.get_container_published_ports('php')  
  describe 'php container published port' do
    subject { ports }
    it { should include '80/tcp -> 0.0.0.0:80' }
    it { should include '80/tcp -> [::]:80' }
    it { should include '443/tcp -> 0.0.0.0:443' }
    it { should include '443/tcp -> [::]:443' }
    it 'should have exactly FOUR entries' do
      expect(ports.length).to eq 4
    end
  end
end

control 'v2xhub-ports-02' do
  impact 1.0
  title 'Verify v2xhub specific ports must be exposed externally'
  desc 'Ensures v2xhub has only specified ports exposed'
  tag network: true
  
  # Check ports exposed on host network
  describe port(input('web_http_port')) do
    its('protocols') { should include 'tcp' }
    its('addresses') { should include '0.0.0.0' }
  end
  describe port(input('web_https_port')) do
    its('protocols') { should include 'tcp' }
    its('addresses') { should include '0.0.0.0' }
  end
  
  # Check ports published by docker compose
  describe v2xhub_helper do
    it { should have_docker_compose_exposed_port('v2xhub', input('websocket_port')) }
    it { should_not have_docker_compose_exposed_port('v2xhub', input('v2x_port')) }
    it { should_not have_docker_compose_exposed_port('v2xhub', input('ivp_default_port')) }

    it { should have_docker_compose_exposed_port( 'v2xhub', 4545, 'udp' ) }
    it { should have_docker_compose_exposed_port( 'v2xhub', 5050, 'udp' ) }
    it { should have_docker_compose_exposed_port( 'v2xhub', 5757, 'udp' ) }
    it { should have_docker_compose_exposed_port( 'v2xhub', 6053, 'udp' ) }
    it { should have_docker_compose_exposed_port( 'v2xhub', 6767, 'udp' ) }
    it { should have_docker_compose_exposed_port( 'v2xhub', 7575, 'udp' ) }
    it { should have_docker_compose_exposed_port( 'v2xhub', 10000 ) }
    it { should have_docker_compose_exposed_port( 'v2xhub', 22222 ) }
    # MessageReceiver (Range 26789 ±10)
    it { should have_docker_compose_exposed_port( 'v2xhub', 26780, 'udp' ) }
    it { should have_docker_compose_exposed_port( 'v2xhub', 26781, 'udp' ) }
    it { should have_docker_compose_exposed_port( 'v2xhub', 26782, 'udp' ) }
    it { should have_docker_compose_exposed_port( 'v2xhub', 26783, 'udp' ) }
    it { should have_docker_compose_exposed_port( 'v2xhub', 26784, 'udp' ) }
    it { should have_docker_compose_exposed_port( 'v2xhub', 26785, 'udp' ) }
    it { should have_docker_compose_exposed_port( 'v2xhub', 26786, 'udp' ) }
    it { should have_docker_compose_exposed_port( 'v2xhub', 26787, 'udp' ) }
    it { should have_docker_compose_exposed_port( 'v2xhub', 26788, 'udp' ) }
    it { should have_docker_compose_exposed_port( 'v2xhub', 26789, 'udp' ) }
    it { should have_docker_compose_exposed_port( 'v2xhub', 26790, 'udp' ) }
    it { should have_docker_compose_exposed_port( 'v2xhub', 26791, 'udp' ) }
    it { should have_docker_compose_exposed_port( 'v2xhub', 26792, 'udp' ) }
    it { should have_docker_compose_exposed_port( 'v2xhub', 26793, 'udp' ) }
    it { should have_docker_compose_exposed_port( 'v2xhub', 26794, 'udp' ) }
    it { should have_docker_compose_exposed_port( 'v2xhub', 26795, 'udp' ) }
    it { should have_docker_compose_exposed_port( 'v2xhub', 26796, 'udp' ) }
    it { should have_docker_compose_exposed_port( 'v2xhub', 26797, 'udp' ) }
    it { should have_docker_compose_exposed_port( 'v2xhub', 26798, 'udp' ) }
    it { should have_docker_compose_exposed_port( 'v2xhub', 26799, 'udp' ) }
    it { should have_docker_compose_exposed_port( 'v2xhub', 26800, 'udp' ) }
  end

  # Check ports published on container
  ports = v2xhub_helper.get_container_published_ports('v2xhub')  
  describe 'v2xhub container published port' do
    subject { ports }
    it { should include '4545/udp -> 0.0.0.0:4545' }
    it { should include '4545/udp -> [::]:4545' }
    it { should include '5050/udp -> 0.0.0.0:5050' }
    it { should include '5050/udp -> [::]:5050' }
    it { should include '5757/udp -> 0.0.0.0:5757' }
    it { should include '5757/udp -> [::]:5757' }
    it { should include '6053/udp -> 0.0.0.0:6053' }
    it { should include '6053/udp -> [::]:6053' }
    it { should include '6767/udp -> 0.0.0.0:6767' }
    it { should include '6767/udp -> [::]:6767' }
    it { should include '7575/udp -> 0.0.0.0:7575' }
    it { should include '7575/udp -> [::]:7575' }
    it { should include '10000/tcp -> 0.0.0.0:10000' }
    it { should include '10000/tcp -> [::]:10000' }
    it { should include '19760/tcp -> 0.0.0.0:19760' }
    it { should include '19760/tcp -> [::]:19760' }
    it { should include '22222/tcp -> 0.0.0.0:22222' }
    it { should include '22222/tcp -> [::]:22222' }
    it { should include '26780/udp -> 0.0.0.0:26780' }
    it { should include '26780/udp -> [::]:26780' }
    it { should include '26781/udp -> 0.0.0.0:26781' }
    it { should include '26781/udp -> [::]:26781' }
    it { should include '26782/udp -> 0.0.0.0:26782' }
    it { should include '26782/udp -> [::]:26782' }
    it { should include '26783/udp -> 0.0.0.0:26783' }
    it { should include '26783/udp -> [::]:26783' }
    it { should include '26784/udp -> 0.0.0.0:26784' }
    it { should include '26784/udp -> [::]:26784' }
    it { should include '26785/udp -> 0.0.0.0:26785' }
    it { should include '26785/udp -> [::]:26785' }
    it { should include '26786/udp -> 0.0.0.0:26786' }
    it { should include '26786/udp -> [::]:26786' }
    it { should include '26787/udp -> 0.0.0.0:26787' }
    it { should include '26787/udp -> [::]:26787' }
    it { should include '26788/udp -> 0.0.0.0:26788' }
    it { should include '26788/udp -> [::]:26788' }
    it { should include '26789/udp -> 0.0.0.0:26789' }
    it { should include '26789/udp -> [::]:26789' }
    it { should include '26790/udp -> 0.0.0.0:26790' }
    it { should include '26790/udp -> [::]:26790' }
    it { should include '26791/udp -> 0.0.0.0:26791' }
    it { should include '26791/udp -> [::]:26791' }
    it { should include '26792/udp -> 0.0.0.0:26792' }
    it { should include '26792/udp -> [::]:26792' }
    it { should include '26793/udp -> 0.0.0.0:26793' }
    it { should include '26793/udp -> [::]:26793' }
    it { should include '26794/udp -> 0.0.0.0:26794' }
    it { should include '26794/udp -> [::]:26794' }
    it { should include '26795/udp -> 0.0.0.0:26795' }
    it { should include '26795/udp -> [::]:26795' }
    it { should include '26796/udp -> 0.0.0.0:26796' }
    it { should include '26796/udp -> [::]:26796' }
    it { should include '26797/udp -> 0.0.0.0:26797' }
    it { should include '26797/udp -> [::]:26797' }
    it { should include '26798/udp -> 0.0.0.0:26798' }
    it { should include '26798/udp -> [::]:26798' }
    it { should include '26799/udp -> 0.0.0.0:26799' }
    it { should include '26799/udp -> [::]:26799' }
    it { should include '26800/udp -> 0.0.0.0:26800' }
    it { should include '26800/udp -> [::]:26800' }
    it 'should have exactly 60 entries' do
      expect(ports.length).to eq 60
    end
  end
end

control 'v2xhub-ports-03' do
  impact 1.0
  title 'Database port must NOT be exposed externally'
  desc 'Ensures MySQL port 3306 is not accessible from outside Docker network (critical security)'
  tag network: true
  tag database: true
  
  # Check ports exposed on host network
  describe port(input('mysql_port')) do
    it { should_not be_listening }
  end
  
  # Check ports published by docker compose
  describe v2xhub_helper do
    it { should_not have_docker_compose_exposed_port('db', input('mysql_port')) }
  end

  # Check ports published on container
  ports = v2xhub_helper.get_container_published_ports('mysql')  
  describe 'mysql container published port' do
    subject { ports }
    it 'should have exactly NO entries' do
      expect(ports.length).to eq 0
    end
  end
end

control 'v2xhub-ports-05' do
  impact 0.8
  title 'php container port ranges should be properly configured in docker compose'
  desc 'Verifies php service has appropriate port ranges for various plugins'  
  tag network: true

  describe yaml('docker-compose.yml') do
    its(['services', 'php', 'ports']) { should include '80:80' }
    its(['services', 'php', 'ports']) { should include '443:443' }
  end
end

control 'v2xhub-ports-06' do
  impact 0.7
  title 'V2XHub container port ranges should be properly configured in docker compose'
  desc 'Verifies V2XHub service has appropriate port ranges for various plugins'
  tag network: true

  compose = yaml('docker-compose.yml').params
  v2xhub_ports = compose.dig('services', 'v2xhub', 'ports') || []

  # Check that V2XHub has multiple port ranges configured
  describe v2xhub_ports do
    # Command Plugin
    it { should  include('19760:19760') }                # Should have WebSocket port

    # Message Receiver Plugin
    it { should include('26780-26800:26780-26800/udp') } # MessageReceiver (Range 26789 ±10)
    
    # SPaT Plugin
    it { should include('6053:6053/udp') }               # TSC sends UDP
    it { should include('5050:5050/udp') }               # SNMP polling
    
    # TIM Plugin
    it { should include('10000:10000/tcp') }             # TIM WebServicePort
    
    # CARMA Cloud Plugin
    it { should include('22222:22222/tcp') } 

    # CDASim Adapter Plugin (simulation mode)
    it { should include('6767:6767/udp') }               # CDASim registration
    it { should include('7575:7575/udp') }               # time sync
    it { should include('5757:5757/udp') }               # SIM_V2X (simulated V2X out)

    # MUST Sensor Driver Plugin
    it { should include('4545:4545/udp') }               # MUST detection receiver port

    it { should_not  include('8686:8686') }              # Should have V2X port
  end
end

control 'v2xhub-ports-07' do
  impact 0.9
  title 'Database container must have no port mappings in docker compose'
  desc 'Ensures database service has no external port exposure configured'
  tag database: true
  tag network: true

  describe yaml('docker-compose.yml') do
    its(['services', 'db', 'ports']) { should be_nil }
  end  
end