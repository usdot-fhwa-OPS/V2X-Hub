# Database Connectivity and Security Controls
# Validates database connectivity, security, and configuration

title 'Database Connectivity and Security'

control 'v2xhub-database-01' do
  impact 1.0
  title 'Database must be responding to ping'
  desc 'Ensures MySQL database service is responding to health checks'
  tag database: true  

  describe command('docker compose exec -T db mysqladmin ping -h localhost --silent') do
    its('exit_status') { should eq 0 }
  end
  
  describe v2xhub_helper do
    it { should be_database_ready }
  end
end

control 'v2xhub-database-02' do
  impact 1.0
  title 'Database port must not be exposed externally'
  desc 'Ensures MySQL port 3306 is not accessible from outside the Docker network'
  tag database: true

  describe port(input('mysql_port')) do
    it { should_not be_listening }
  end
  
  describe v2xhub_helper do
    it { should_not have_docker_compose_exposed_port('db', input('mysql_port')) }
  end
  
  describe command("docker port mysql #{input('mysql_port')}") do
    its('stdout.strip') { should be_empty.or match(/no public port '#{input('mysql_port')}' published for mysql/) }
  end
end

control 'v2xhub-database-03' do
  impact 0.9
  title 'Database container must have no external published ports'
  desc 'Verifies database container has no ports exposed to the host'
  tag database: true
  
  describe command("docker port mysql") do
    its('stdout.strip') { should be_empty.or match(/no public port '#{input('mysql_port')}' published for mysql/) }
  end
end

control 'v2xhub-database-04' do
  impact 0.8
  title 'Database must have proper health check configuration'
  desc 'Ensures database service has health check configured in Docker Compose'
  tag database: true
  
  describe yaml('docker-compose.yml') do
    its(['services', 'db', 'healthcheck']) { should_not be_nil }
    its(['services', 'db', 'healthcheck', 'test']) { should include 'mysqladmin' }
    its(['services', 'db', 'healthcheck', 'test']) { should include 'ping' }
  end
end

control 'v2xhub-database-05' do
  impact 0.9
  title 'Database credentials must be managed via secrets'
  desc 'Verifies database password is managed through Docker secrets'
  tag database: true

  describe yaml('docker-compose.yml') do
    its(['services', 'db', 'environment']) { should include 'MYSQL_PASSWORD_FILE=/run/secrets/mysql_password' }
    its(['services', 'db', 'secrets']) { should include 'mysql_password' }
  end
  
  describe v2xhub_helper do
    it { should be_container_file_exists('db', '/run/secrets/mysql_password') }
  end
end

control 'v2xhub-database-06' do
  impact 0.7
  title 'Database environment variables must be properly set'
  desc 'Ensures required database environment variables are configured'
  tag database: true

  required_env_vars = {
    'MYSQL_DATABASE' => input('mysql_database'),
    'MYSQL_USER' => input('mysql_user')
  }
  
  required_env_vars.each do |var, expected_value|
    describe v2xhub_helper.container_env_var('db', var) do
      it { should eq expected_value }
    end
  end
  
  # Check that random root password is enabled
  describe v2xhub_helper.container_env_var('db', 'MYSQL_RANDOM_ROOT_PASSWORD') do
    it { should eq 'yes' }
  end
end

control 'v2xhub-database-07' do
  impact 0.8
  title 'Database must be accessible only from internal network'
  desc 'Verifies database container is only connected to internal network'
  tag database: true

  # Get networks container is attached to
  networks = command("docker inspect --format '{{range $k,$v := .NetworkSettings.Networks}}{{$k}} {{end}}' mysql").stdout.strip.split

  describe 'mysql container network attachments' do
    subject { networks }

    # Attached to correct network
    it { should include 'configuration_v2xhub_data_internal' }
    it { should_not include 'configuration_v2xhub_app_external' }
    it { should_not include 'configuration_v2xhub_web_external' }
    # Ensure host networking is not used
    it { should_not include 'host' }
  end

  describe 'mysql container should be attached to ONE network' do
    it 'should be attached to exactly one network' do
      expect(networks.length).to eq 1
    end
  end

  # For each network, assert driver is allowed bridge network
  networks.each do |net|
    describe command("docker network inspect --format '{{.Name}} {{.Driver}}' #{net}") do
      its('stdout.strip') { should match(/^#{Regexp.escape(net)}\s+(#{['bridge'].join('|')})$/) }
    end
  end  
end

control 'v2xhub-database-08' do
  impact 0.6
  title 'Database initialization scripts must be present'
  desc 'Ensures required SQL initialization files are mounted'
  tag database: true

  describe yaml('docker-compose.yml') do
    its(['services', 'db', 'volumes']) { should include './mysql/localhost.sql:/docker-entrypoint-initdb.d/localhost.sql' }
    its(['services', 'db', 'volumes']) { should include './mysql/port_drayage.sql:/docker-entrypoint-initdb.d/port_drayage.sql' }
  end
  
  describe file('mysql/localhost.sql') do
    it { should exist }
    it { should be_readable }
  end
  
  describe file('mysql/port_drayage.sql') do
    it { should exist }
    it { should be_readable }
  end
end

control 'v2xhub-database-09' do
  impact 0.5
  title 'Docker secrets must be configured and database secrets secured'
  desc 'Ensures MySQL password is managed via Docker secrets'
  tag database: true
  tag docker_compose: true

  describe yaml('docker-compose.yml') do
    its(['secrets', 'mysql_password']) { should_not be_nil }
    its(['services', 'db', 'secrets']) { should include 'mysql_password' }
    its(['services', 'v2xhub', 'secrets']) { should include 'mysql_password' }
  end
end