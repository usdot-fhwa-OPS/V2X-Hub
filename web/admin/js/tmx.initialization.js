// Connection specifics
// 	ip: 		Hostname for WebSocket connection (now uses current page hostname)
// 	port: 		Port for WebSocket connection (now uses current page port)
// 	path: 		Path for WebSocket proxy (routes through Apache to v2xhub container)
// 	led: 		id of the element that contains the status indicator. 
var connections = 	[
				{
					ip: window.location.hostname,
					port: window.location.port || (window.location.protocol === 'https:' ? '443' : '80'),
					path: "/v2xhub-ws",
					led: "Logo"
				}
			];
