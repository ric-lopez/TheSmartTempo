let socket;

window.onload = () => {
  // Se establece la conexión WebSocket
  socket = new WebSocket(`ws://${window.location.hostname}/ws`);

  socket.onopen = () => {
    console.log("Conexión WebSocket abierta");
  };

  socket.onmessage = (event) => {
    const message = event.data;
    const [id, content] = message.split(":"); 
  
    if (id === "id1") {
      const times = content.split(",");
      if(times.length > 2){
        let showTimes = "";
        if(times[0] == '0'){
          showTimes = "1 a 2 = "+times[1]+" segundos\n"+"2 a 3 = "+times[2]+" segundos\n"+"1 a 3 = "+times[3]+" segundos";  
        }else{
          showTimes = "3 a 2 = "+times[2]+" segundos\n"+"2 a 1 = "+times[1]+" segundos\n"+"3 a 1 = "+times[3]+" segundos";
        }
        document.getElementById("serverResponse1").style.visibility="hidden";
        document.getElementById("serverResponse3").style.visibility="visible"; 
        document.getElementById("serverResponse3").innerText = showTimes; 
      }else{
        if(content == "ERROR"){
          document.getElementById("serverResponse1").innerText = content+" DE MEDICION";     
        }else{
          document.getElementById("serverResponse1").innerText = content+" segundos";   
        }
      }
      //document.getElementById("serverResponse1").innerText = content; 
      console.log("Tiempo")
    }else if (id === "id2") {
      document.getElementById("serverResponse2").innerText = content;
      console.log("conectado")
    }else if (id === "id3") {
      document.getElementById("serverResponse3").innerText = content; 
      console.log("conectado")
    }else{
      console.warn("Identificador desconocido:", id);
    }
  };

  socket.onclose = () => {
    console.log("Conexión WebSocket cerrada");
    setTimeout(initWebSocket, 2000);
  };

  socket.onerror = (error) => {
    console.error("Error en WebSocket:", error);
    document.getElementById("serverResponse").innerText =
      "Error: no se pudo conectar al servidor.";
  };

  // Enlazar eventos de botones
  document.getElementById("btnM12").onclick = () => sendCommand("m12");
  document.getElementById("btnM13").onclick = () => sendCommand("m13");
  document.getElementById("btnM23").onclick = () => sendCommand("m23");
  document.getElementById("btnM123").onclick = () => sendCommand("m123");
  document.getElementById("reset").onclick = () => sendCommand("reset");
};

function sendCommand(command) {
  if (socket.readyState === WebSocket.OPEN) {
    socket.send(command);
    document.getElementById("serverResponse3").style.visibility="hidden"; 
    document.getElementById("serverResponse1").style.visibility="visible"; 
    console.log("Comando enviado:", command);
  } else {
    console.warn("WebSocket no está abierto. Comando no enviado:", command);
  }
}


