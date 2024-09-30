# SampleBSMPlugin

This is an example plugin used for demostrating how to develop new **V2X Hub** plugins. Any developed plugin should include accompanying documentation in the form of a `README.md` with the sections outlined here.

## Documentation Template

### Introduction

A brief introduction to the Plugin and the functionality it introduces. Well developed plugins are modular and function independently.

### Related Plugins

A list of plugins related to the documented plugin. Often to enable Connected Automated Vehicle (CAV) use-cases, requires multiple **V2X-Hub**  plugins running conccurently. This section should described other plugins required for end-user functionality and their role.

### Configuration/Deployment

This section should provide documentation about how to configure and deploy the  plugin. This should include descriptions of plugin configuration parameters as well as step by step instructions for enabling the plugin.

### Design (Recommended)

This is an optional, but recommended section that can be used to outline the high-level design of your plugin. This should include insights into data-flow, communication sequence and relevant data transforms/translations.

### Messages (Recommended)

This is an optional, but recommended section that is intended to described the TMX Messages used by the plugin to communicate with other plugins in V2X-Hub

### Testing (Recommended)

This final section is optional, but recommended and is intended to described methods used to confirm the deployed plugin is functioning correctly. This can include testing scripts used to send mock data through the plugin and/or UI status fields to check to confirm working functionality.
