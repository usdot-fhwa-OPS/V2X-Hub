# TIM Plugin Documentation

## Introduction

This plugin is used to read/receive TIM (Traveler Information Message) XML data and forward it to an RSU for broadcast. TIM (Traveler Information Message) can be provided as files or via HTTP POST request. 

## Related Plugins

A list of plugins related to the TIM Plugin.

### Immediate Forward Plugin

For RSU Immediate Message Forwarding (IMF) functionality forward TIM (Traveler Information Message).

## Configuration/Deployment

This plugin has several configuration parameters. Below these are listed out as together with descriptions on how to set them.

**Interval**: The interval in milliseconds to wait between sequential TIM broadcasts 

**TimFile**: The absolute path the the TIM.xml file to read for broadcasting. NOTE: Please use the upload file feature on the Web UI to update TimFiles.

**WebServiceIP**: The IP to listen for HTTP POST requests for updated TIM XML payloads

**WebServicePort**: The port to listen for HTTP POST requests for updated TIM XML payloads


## Design

This plugin consists of a simple REST Server and the functionality to read XML TIM files and broadcast them.

### Messages

**TIM**: Travel Information Message

## Functionality Testing

Testing the functionality of this plugin requires a valid TIM.xml file and a tool to send HTTP request like `curl`

1) Upload a TIM file using the **Upload File** button in the top right hand corner of the **Plugins** tab of the V2X Hub Web UI
![alt text](<docs/Upload.png>)
2) Select  **Upload Other** and provide the `/var/www/download/TIM` as the directory. Then select the file you want to upload and hit **Submit**.
![alt text](<docs/Upload.png>)
3) Update the TIM Plugin **TimFile** configuration parameter to point to your uploaded file and enable the plugin
4) If active, your TIM Plugin Messages tab should show a count increase for the TIM messages you are broadcasting.
5) Using a tool of choice send an updated TIM to the correct IP and Port
```curl -X POST -H "Content-Type: text/xml" --data @../Downloads/tim_2024.xml http://127.0.0.1:10000/tim```
6) If also active, the TIM Plugin should be broadcasting the new provided message. This can be inspecting using debug logs or the JSONLoggerPlugin.
