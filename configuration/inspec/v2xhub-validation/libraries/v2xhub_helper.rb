# V2X-Hub Helper Library for InSpec Tests
# Provides common utility methods for V2X-Hub validation

class V2XHubHelper < Inspec.resource(1)
  name 'v2xhub_helper'
  desc 'Helper methods for V2X-Hub validation tests'

  # Check if Docker Compose is accessible
  def docker_compose_accessible?
    cmd = inspec.command('docker compose ps')
    cmd.exit_status == 0
  end

  # Test network connectivity between containers
  def container_can_reach?(from_container, to_host, to_port, timeout = 5)
    cmd = inspec.command("docker compose exec -T #{from_container} timeout #{timeout} bash -c '</dev/tcp/#{to_host}/#{to_port}'")
    cmd.exit_status == 0
  end

  # Check if a port is exposed externally via Docker Compose
  def has_docker_compose_exposed_port?(service, port, portocol = nil)
    # Assumes portocol is TCP unless specified
    cmd = inspec.command("docker compose port #{service} #{port}#{portocol ? " --protocol #{portocol}" : ''}")

    return false if cmd.exit_status != 0
    
    output = cmd.stdout.strip
    # Check if port is mapped and not just ":0"
    !output.empty? && output != "invalid IP:0" && output != ":0" && !output.include?("no port")
  end

  #  get published port
  def get_container_published_ports(container, port = nil)
    cmd = inspec.command("docker port #{container} #{port}")

    return [] unless cmd.exit_status == 0
    cmd.stdout.strip.split("\n")
  end

  # Get environment variable from container
  def container_env_var(container, var_name)
    cmd = inspec.command("docker compose exec -T #{container} printenv #{var_name}")
    return nil unless cmd.exit_status == 0
    cmd.stdout.strip
  end

  # Check if file exists in container
  def container_file_exists?(container, file_path)
    cmd = inspec.command("docker compose exec -T #{container} test -f #{file_path}")
    cmd.exit_status == 0
  end

  # Check if Docker network exists
  def docker_network_exists?(network_name)
    cmd = inspec.command("docker network inspect #{network_name}")
    cmd.exit_status == 0
  end

  # Check if Docker network is internal
  def docker_network_internal?(network_name)
    cmd = inspec.command("docker network inspect #{network_name} --format '{{.Internal}}'")
    return false unless cmd.exit_status == 0
    cmd.stdout.strip == 'true'
  end

  # Check if a Docker network uses the expected driver
  def has_docker_network_and_driver?(network_name, expected_driver)
    cmd = inspec.command("docker network inspect #{network_name} --format '{{.Driver}}'")
    return false unless cmd.exit_status == 0
    cmd.stdout.strip == expected_driver
  end
  # Wait for database to be ready (with retry logic)
  def database_ready?(max_attempts = 30)
    attempts = 0
    while attempts < max_attempts
      cmd = inspec.command('docker compose exec -T db mysqladmin ping -h localhost --silent')
      return true if cmd.exit_status == 0
      
      attempts += 1
      sleep(2) if attempts < max_attempts
    end
    false
  end

  # Get the networks a container is attached to
  def container_networks(container_name)
    cmd = inspec.command("docker inspect --format '{{range $k,$v := .NetworkSettings.Networks}}{{$k}} {{end}}' #{container_name}")
    return [] unless cmd.exit_status == 0
    cmd.stdout.strip.split
  end
end