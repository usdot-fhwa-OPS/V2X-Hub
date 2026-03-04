# Development Environment Setup

## Using VS Code For Development

[Visual Studio Code](https://code.visualstudio.com/) is a functional free IDE which can be used to develop many projects. Through the use of Docker images it allows developers to develop in containers where build tools can be pre-configured, precluding the need to have a special host to build on with instructions to follow for tool installation.  It can be used on Windows hosts using Docker Desktop or on Linux hosts with Docker installed.

## Setup

This project includes a DevContainer setup which allows for developing in a containerized environment that will spawn run-time depedencies for testing 

### Installation

You will need to have the following installed on your host or VM.

* Docker - Install varies by Linux distribution. Use Docker Desktop on Windows.
* Install VS Code - Look in https://code.visualstudio.com/download

### One Time Setup

* Open a VS Code window.
* Open the source folder on your host.  VS Code may notify you that there are extensions available to handle the .devcontainer folder.  If so do not do anything for now.
* Install (if not already present) the VS Code **Dev Containers** extension.
* Use ctrl-shift-P (Command Pallette) and select *Dev Containers: Open Folder in Container* and select the current project folder.
* At this point VS Code should start building a container for your V2XHUB development as well as MySQL and PHP containers that are necessary for running the system.

Once a container has been built you should not need to rebuild it again unless something changes in the dependencies or Docker setup.  If you do need to rebuild then use the *Dev Containers: Rebuild Container* command from the pallette.

One of the great features about this setup is that the devcontainer.json file can be configured with a list of extensions to be included any time the container is opener so the list can be configured with what is most commonly used with the project so new users do not need to figure that out by themselves or manually install them.


## Tasks

The configuration comes with 3 configured tasks.  These can be run under from the menu via *Terminal -> Run Task...*.  The tasks are described below.
* build : This builds the code using the build.sh script in the src directory.  The output from this can be using to navigate directly to any problems.  This task is configured as the default build task so it can be activated with the shortcut ctrl-shift-B.
* clean : Cleans out the build files if you need to start over with compiling for any reason.
* run : After a successful build you can use the run task to run your code changes and connect to the system using a browser.
* test: After a successful build you can use this task to run all the build unit tests

> [!NOTE]
> Both **test** and **build** tasks have a duplicate version which includes **coverage** to build the source code and run the unit tests to get code coverage metrics. 

## DevContainer VSCode Extensions

By default our dev container installs several VSCode extensions helpful for developing and testing V2X Hub code changes. These are documented in the `.devcontainer/devcontainer.json` under **customizations.vscode.extensions** and also listed below:
- **CPP Tools**, **CPP Tools Extension Pack**, and **CMake Tools** are extensions for C++ and CMake to make editing our source code and build files easier and more intuitive
- **Sonar Lint** is a code quality and linting tool that provides feedback about best practices and security issues or hotspots


### Sonar Lint One-Time Setup

The first time setting up Sonar Lint, there are a couple VSCode prompts to navitigate to allow you to get Sonar Lint Feedback.

## Configuring DevContainer to Trust Organizational Certificate Authorities / Internal TLS

> [!NOTE]
> Skip if not required. For developers, developing inside coporate VPN, this step is required to avoid certificate issues.

If your organization uses private Certificate Authorities (CA), TLS inspection, or a corporate VPN that intercepts HTTPS traffic, the DevContainer must trust your organization’s root and/or intermediate CA certificates.

Without these certificates installed, tools such as curl, nvm, git, or package managers may fail with TLS certificate verification errors.

### Steps
1. Obtain Required Certificates
Request from your IT/security team:

The organization’s Root CA certificate
Any Intermediate CA certificates used for perimeter or TLS inspection
You do not need the VPN server certificate or any private keys — only public CA certificates.

2. Save Certificates to the Repository
Place the certificate files (public certs only) in:

.devcontainer/perimeter-certs/
Requirements:

Files must be in PEM format
Use the .crt file extension (required for automatic installation)
Multiple certificates may be added if needed
Example:

.devcontainer/perimeter-certs/
├── Corp-Root-CA.crt
├── Corp-Perimeter-CA.crt
3. Rebuild the DevContainer
In VS Code:

Dev Containers: Rebuild Container
During container setup, the DevContainer initialization script will:

Detect any .crt files in .devcontainer/perimeter-certs/
Install them into /usr/local/share/ca-certificates/
Update the container’s system trust store
No manual installation steps are required.

Verification (Optional)
After rebuilding, verify that TLS trust works from DevContainer:

curl -I https://nodejs.org/dist/index.json
Expected result:

You should see an HTTP/1.1 200 or HTTP/2 200 response
No certificate verification errors
If this succeeds, certificate trust is correctly configured.

### Troubleshooting

If you still see certificate errors:

Ensure certificate files use the .crt extension
Confirm the files contain valid PEM-encoded certificates
Verify you included both root and intermediate CA certificates (if required)
Rebuild the container again after changes
Security Note
Only public CA certificates should be committed to the repository.
Never commit private keys.

Proceed
After certificate trust is configured, continue with V2XHub deployment as usual.