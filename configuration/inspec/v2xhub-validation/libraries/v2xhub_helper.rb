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

  # Get running Docker Compose services
  def running_services
    cmd = inspec.command('docker compose ps --services --filter "status=running"')
    return [] unless cmd.exit_status == 0
    cmd.stdout.split("\n").map(&:strip).reject(&:empty?)
  end

  # Check if a service is running
  def service_running?(service_name)
    running_services.include?(service_name)
  end

  # Test network connectivity between containers
  def container_can_reach?(from_container, to_host, to_port, timeout = 5)
    cmd = inspec.command("docker compose exec -T #{from_container} timeout #{timeout} bash -c '</dev/tcp/#{to_host}/#{to_port}'")
    cmd.exit_status == 0
  end

  # Check if a port is exposed externally via Docker Compose
  def port_exposed_externally?(service, port)
    cmd = inspec.command("docker compose port #{service} #{port}")
    return false if cmd.exit_status != 0
    
    output = cmd.stdout.strip
    # Check if port is mapped and not just ":0"
    !output.empty? && output != ":0" && !output.include?("no port")
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

  # Get container health status
  def container_health_status(service)
    cmd = inspec.command("docker compose ps --format 'table {{.Service}}\t{{.Status}}' | grep '^#{service}'")
    return 'unknown' unless cmd.exit_status == 0
    
    status_line = cmd.stdout.strip
    return 'unknown' if status_line.empty?
    
    # Extract status from the line (second column)
    parts = status_line.split(/\s+/)
    return 'unknown' if parts.length < 2
    
    status = parts[1]
    case status
    when /healthy/
      'healthy'
    when /unhealthy/
      'unhealthy'
    when /starting/
      'starting'
    else
      status
    end
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

  # Check Apache module is loaded
  def apache_module_loaded?(container, module_name)
    cmd = inspec.command("docker compose exec -T #{container} apache2ctl -M")
    return false unless cmd.exit_status == 0
    cmd.stdout.include?(module_name)
  end

  # Search for pattern in container file
  def container_file_contains?(container, file_path, pattern)
    cmd = inspec.command("docker compose exec -T #{container} grep -q '#{pattern}' #{file_path}")
    cmd.exit_status == 0
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
end
