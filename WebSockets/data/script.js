/**
 * Esta sección de código establece y maneja una conexión WebSocket entre el cliente (navegador) y el servidor.
 * Además, permite enviar comandos al servidor y manejar respuestas que actualizan la interfaz de usuario.
 */

let socket;

/**
 * La función 'window.onload' se ejecuta cuando la página ha terminado de cargarse.
 * Establece la conexión WebSocket, enlaza eventos a los botones y maneja la inicialización de la página.
 */
window.onload = () => {
  // Establecer la conexión WebSocket con el servidor en la URL construida con el nombre de host actual.
  socket = new WebSocket(`ws://${window.location.hostname}/ws`);

  // Event Handler: Se ejecuta cuando la conexión WebSocket se establece correctamente.
  socket.onopen = () => {
    console.log("Conexión WebSocket abierta");
  };

  // Event Handler: Se ejecuta cuando el cliente recibe un mensaje del servidor.
  socket.onmessage = (event) => {
    const message = event.data;
    const [id, content] = message.split(":"); // Separar el mensaje en id y contenido

    // Dependiendo del id, actualizar los elementos de la interfaz de usuario correspondientes.
    if (id === "id1") {
      document.getElementById("serverResponse1").innerText = content; 
      console.log("Tiempo")
    } else if (id === "id2") {
      document.getElementById("serverResponse2").innerText = content;
      console.log("Conectado")
    } else if (id === "id3") {
      document.getElementById("serverResponse3").innerText = content; 
      console.log("Estado")
    } else {
      console.warn("Identificador desconocido:", id);
    }
  };

  // Event Handler: Se ejecuta cuando la conexión WebSocket se cierra.
  socket.onclose = () => {
    console.log("Conexión WebSocket cerrada");
    setTimeout(initWebSocket, 2000); // Reintenta abrir la conexión WebSocket después de 2 segundos.
  };

  // Event Handler: Se ejecuta cuando ocurre un error en la conexión WebSocket.
  socket.onerror = (error) => {
    console.error("Error en WebSocket:", error);
    document.getElementById("serverResponse").innerText =
      "Error: no se pudo conectar al servidor."; // Muestra un mensaje de error al usuario.
  };

  // Enlazar los eventos 'onclick' a los botones de la interfaz para enviar comandos al servidor.
  document.getElementById("btnM12").onclick = () => sendCommand("m12");
  document.getElementById("btnM13").onclick = () => sendCommand("m13");
  document.getElementById("btnM23").onclick = () => sendCommand("m23");
  document.getElementById("reset").onclick = () => sendCommand("reset");
};

/**
 * Función 'sendCommand': Envia un comando al servidor a través de la conexión WebSocket.
 * @param {string} command - El comando que se desea enviar al servidor.
 */
function sendCommand(command) {
  // Verifica que el WebSocket esté abierto antes de enviar el comando.
  if (socket.readyState === WebSocket.OPEN) {
    socket.send(command); // Envía el comando al servidor.
    console.log("Comando enviado:", command);
  } else {
    console.warn("WebSocket no está abierto. Comando no enviado:", command);
  }
}
