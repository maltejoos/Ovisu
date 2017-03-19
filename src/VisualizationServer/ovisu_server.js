// global data
//triplets: x, y, theta
var all_positions = [0, 0, 0];
var max_positions = 99;
var global_pos_x = 0;
var global_pos_y = 0;
var global_pos_theta = 0;

var websocket_clients = [];

//mqtt client to recieve odometry data
var mqtt = require('mqtt')
var mqtt_client  = mqtt.connect('mqtt://192.168.2.107')

mqtt_client.on('connect', function () {
  mqtt_client.subscribe('Roomba/OdometryData')
  mqtt_client.subscribe('Roomba/OdometryBurst')
})

mqtt_client.on('message', function (topic, message) {      
  switch (topic) {
    case 'Roomba/OdometryData':
        console.log(message.toString())
        ProcessOdometryData(message.toString());
    case 'Roomba/OdometryBurst':        
        //reset global positions if burst starts
        if(parseInt(message.toString()) == 1)
        {
            console.log("Reset")
            all_positions = [0,0,0];
        }
  }
})

function ProcessOdometryData(message) 
{	
    var data = message.split(";");
    var dtrans = parseInt(data[0]);
    var dtheta = parseInt(data[1]);
    var dtheta = dtheta*Math.PI/180;//convert to rad
    
    //Integrate position: translate along half rotation, then apply full rotation.	
    var tmp_ang = global_pos_theta + dtheta/2; 
    global_pos_x += Math.cos(tmp_ang)*dtrans;
    global_pos_y += Math.sin(tmp_ang)*dtrans;
    global_pos_theta += dtheta;
    
    if(all_positions.length >= max_positions)
    {
        all_positions.shift();
        all_positions.shift();
        all_positions.shift();
    }
    
    //store position
    all_positions.push(global_pos_x);
    all_positions.push(global_pos_y);
    all_positions.push(global_pos_theta);
    
    RemoveDisconnectedClients();
    SendNewPos(global_pos_x, global_pos_y, global_pos_theta);
}

//websocket server for sending graph to visualisation client
var WebSocketServer = require('ws').Server;
var wss = new WebSocketServer({port: 4321});

//send all positions when connection is opened
wss.on('connection', function(ws) {
	ws.send(JSON.stringify(all_positions));
	websocket_clients.push(ws);
});


function RemoveDisconnectedClients()
{
	for(i=0; i < websocket_clients.length; ++i)
	{
		//client is not ready
		if(websocket_clients[i].readyState != 1)
		{			
			// delete client from array if not ready anymore
			websocket_clients.slice(i, 1);
		}
	}
}

//send a new position to all connected clients
function SendNewPos(x, y, t)
{
	var pos = [x, y, t];
	for(i=0; i < websocket_clients.length; ++i)
	{
		//client is ready
		if(websocket_clients[i].readyState == 1)
		{
			websocket_clients[i].send(JSON.stringify(pos));
		}
	}
};
