#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoOTA.h>

const char* ssid = "YOUR SSID";
const char* password = "YOUR PASSWORD";

ESP8266WebServer server(80);

// Настройки длин
const int MAX_USERNAME_LENGTH = 20;  // Maximum username length
const int MAX_MESSAGE_LENGTH = 200;  // Maximum message length

String currentUser = ""; // Stores the current user's name
String messages = "";    // Stores all chat messages

unsigned long lastClearTime = 0; // Time of last message clearing
const unsigned long CLEAR_INTERVAL = 30 * 60 * 1000; // 30 minutes in milliseconds

unsigned long lastMessageTime = 0; // Last message time
const unsigned long FLOOD_INTERVAL = 2 * 1000; // Flood protection interval in milliseconds

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("Connected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  ArduinoOTA.begin();

  server.on("/", HTTP_GET, handleRoot);
  server.on("/send", HTTP_POST, handleSend);
  server.on("/messages", HTTP_GET, handleMessages);
  server.on("/setuser", HTTP_POST, handleSetUser);
  server.on("/sys", HTTP_GET, handleSystemMessage);

  server.begin();
}

void loop() {
  ArduinoOTA.handle();
  server.handleClient();

  // Очистка сообщений каждые 5 минут
  unsigned long currentMillis = millis();
  if (currentMillis - lastClearTime >= CLEAR_INTERVAL) {
    messages = "";
    lastClearTime = currentMillis;
  }
}

void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>CHAT</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      background-color: #121212; /* Page background color */
      color: #e0e0e0; /* Text color */
      margin: 0;
      padding: 0;
      display: flex;
      flex-direction: column;
      height: 100vh;
    }
    h1 {
      color: #e0e0e0;
      text-align: center;
      margin: 20px 0;
    }
    #userSection {
      margin: 20px;
      text-align: center;
    }
    #chat {
      width: 100%;
      height: calc(100vh - 180px); /* Chat height minus padding and buttons */
      border: 1px solid #333;
      background-color: #1e1e1e; /* Chat background color */
      color: #e0e0e0; /* Text color */
      overflow-y: auto;
      padding: 10px;
      box-sizing: border-box;
    }
    .message {
      margin: 10px 0;
      padding: 10px;
      border-radius: 15px;
      max-width: 80%;
      word-wrap: break-word;
      position: relative;
    }
   /* CSS for chat images */
    .message img {
      max-width: 100%;
      height: auto;
      border-radius: 10px;
      margin: 10px 0;
    }
    .message.user-sent {
      background-color: #007bff; /* Background color of the message sent by the user */
      color: #ffffff; /* Color of the message text sent by the user */
      margin-left: auto;
      text-align: right;
    }
    .message.user-sent:before {
      content: '';
      position: absolute;
      top: 50%;
      right: -15px;
      border: 10px solid transparent;
      border-left-color: #007bff; /* Background color of the message sent by the user */
    }
    .message.user-received {
      background-color: #2c2c2c; /* Background color of the message received by the user */
      color: #ffffff; /* Color of the message text received by the user */
      margin-right: auto;
    }
    .message.user-received:before {
      content: '';
      position: absolute;
      top: 50%;
      left: -15px;
      border: 10px solid transparent;
      border-right-color: #2c2c2c; /* Background color of the message received by the user */
    }
    .message.system {
      background-color: #ff4d4d; /* System message background color */
      color: #ffffff; /* System message text color */
      margin-right: auto;
    }
    .message.system:before {
      content: '';
      position: absolute;
      top: 50%;
      left: -15px;
      border: 10px solid transparent;
      border-right-color: #ff4d4d; /* System message background color */
    }
    #message-container {
      display: flex;
      align-items: center;
      margin: 10px;
    }
    #message {
      flex: 1;
      padding: 10px;
      border: 1px solid #333;
      border-radius: 5px;
      background-color: #2c2c2c;
      color: #e0e0e0;
      font-size: 1.2em; /* Message input font size */
    }
    #send {
      width: 80px;
      padding: 10px;
      margin-left: 10px;
      border: none;
      border-radius: 5px;
      background-color: #007bff; /* Button background color */
      color: #ffffff;
      cursor: pointer;
      font-size: 1.2em; /* Button font size */
    }
    #send:hover {
      background-color: #0056b3; /* Button background color when hovered */
    }
    @media (max-width: 600px) {
      #message {
        font-size: 1em; /* Reduced message input font size */
      }
      #send {
        font-size: 1em; /* Reduced button font size */
      }
      #chat {
        height: calc(100vh - 150px);
      }
    }
    @media (max-width: 400px) {
      #message {
        font-size: 0.9em; /* Even smaller font size for message input */
      }
      #send {
        font-size: 0.9em; /* Even smaller button font size */
      }
      #chat {
        height: calc(100vh - 120px);
      }
    }
    .error {
      color: red; /* Error text color */
      font-size: 0.9em;
    }
  </style>
</head>
<body>
  <h1>CHAT</h1>
  <div id="userSection">
    <label for="username">Enter your name:</label>
    <input id="username" type="text" placeholder="Your name..." maxlength="20">
    <button id="setUser" onclick="setUser()">Set Name</button>
    <div id="usernameError" class="error"></div>
  </div>
  <div id="chat"></div>
  <div id="message-container">
    <input id="message" type="text" placeholder="Type a message..." maxlength="200">
    <button id="send" onclick="sendMessage()">Send</button>
  </div>
  <div id="messageError" class="error"></div>

  <script>
  let chat = document.getElementById("chat");
  let messageInput = document.getElementById("message");
  let usernameInput = document.getElementById("username");
  let usernameError = document.getElementById("usernameError");
  let messageError = document.getElementById("messageError");

  let lastMessageId = ''; // Variable to store the last message ID

  function beep() {
    var snd = new Audio("data:audio/wav;base64,UklGRlYIAABXQVZFZm10IBAAAAABAAEARKwAAIhYAQACABAAZGF0YegGAABFR0xFqEdzRPZIODzbw0S3/LvXuIi7b7k8u0S65LmDwlI9KEbcRN5EBEWURP5EFEQBRjs8zMNfuhm84ru7u128lbsBvXq62MQaOztFr0K+Qw9DPUM+Q4xCaES3OmHFxLv4vSi9sL2QvZq9J76OvMzFJTopQ4ZBv0HUQVBB8EG1QPxCJznkxli9jL/SviW/Yr/ivinApr3Zx/o3bkLjPmFBxT5/QRY+MUJ1PKJEkttfuY/EyL0Ww6C+H8NmvmPEQrzYI/NFbTqzQcw760C9OyNBgTo3Q07cOrsOxmC/wsQYwOHE0L8qxrK9OyPeQz05uz+MOgA/cjpAPzI5UkFd3eK8pMf8wFPGvcFmxoXBm8eDvzIiOkKYNzo+1ziGPbo4xD2DN8g/Ft7ZvvPI1MK2x4PD2Mc8wxvJM8FdIVFATDZLPKI3ejumN5Y7ljZmPb3fz79Zy37DXMrpw9nKEcM+zWi9gwv0QeYyDDzWNBo7rDS6O3IyOEEa9YS+TM5WxGDMTsV6zMbEls50v/YKXUAiMbE6+DLIOdEyWzqvMKY/I/WnwH/PRMatzSvH1M2ZxvPPXsGhCns+uS/vOIkxAzhqMYk4Xi+lPZj1Y8IS0dfHUc+2yHnPK8iD0SLDaAqGPFkuLzcKMF021i/yNsQt+Tur9YzEMNLpyWrQ1cqC0F/KcNKJxYwJMTtgLP41DC4kNeYtpDX2K2s6agmhxnrTtsvi0X7MF9LyyxLUKcco9vc4VCvcMwAt+jLlLGwzCCsLOJcJBchh1fvMy9POze/TVs3M1cfIsvbpNgkqCTKUKz4xaiu4MZApOTYxCf/JstbTzifVo89J1THPGdfJyuP2KTV6KG8w/SmgL90pDDAZKFw04AjZyzDYc9DG1ivR+Naz0MDYbsx29wEzWCdnLs8ooi2nKBIu6CZBMsIIks222SrSKNga0w3YFtML2arQSuj6LN0nXCteKAErFShRK9sm5y23F4DTNdk41ZjYsNXE2H3V4NkK06novyvrJRMqgiafKVUm0Ck9JTYswRY71b7a09Y42jnXcNoB14rbo9TA6dkplSRAKB4l1yfnJBAoziNlKtwV9tY23JHYo9sJ2cfb59jH3LHWX+puKMMi5iZII3gmHCOfJh4ixSjaFLXYxd0Y2mfdXNrB3f/Z/d6Z11HsgSWlIqEjnCOZIlEkUSFPJjUlfyIpHzPhvty13Vrdwt2S3dPd8t1b3fjh9B1qIqYhoCG2IVkhqSH9IBciFx334jXeUd8G3zffWd8437/fwt6b41EcBCHWHz8g6B/yH+QfjB9ZIMUbT+Tg3+LgnuDY4OLg5uA84X/g4uQPGzsfaR59HnceMx5wHtQd1h5EGsrlc+GC4jXiaOKO4mDi/uLn4ZHmUBkDHlgcch03HGcd0xufHQMbnx7F75ngl+Wp4gflHuMj5RrjyOVC4rYPnR6JGaccDBo5HO0ZPBxNGQodmfBp4hjnSeSi5q3kx+ao5Gjn3OPrDrgcKBjeGqEYdBqCGHQa5xc3G5nxH+Si6O3lLuhV5k3oWObh6KDl9g39GpsWRBkEF+QY4RbhGE4WlRlu8vjlEOqo56jpCejM6QjoY+pS5xcNJxkzFXsXoBUQF4sV/xYJFZoXoPNw5+Tr9Oid6zXp5+v86Obs+eZRBKoY+xJcFpoT5xVzEwUWjBLqFwX8Gejp7VXqTu3L6m/tuOpG7vDo5APpFmsRxhT6EVcU1BFvFPwQKhY4/BTqQu8f7Lzuiuzj7nfsse/Q6pUDBRUAEAkThBCcEmAQrBKZDzwUmfzl67/wyu1G8DDubfAh7izxqOxJAyETkw5QEQUP7hDbDgARGg5vEtX83e0W8pvvqPEB8Mjx/u928q7u0gJkEQANvg9iDWAPPA1kD5UMoBCuAsvvefNa8STzr/FR86bx9vN78FL9bQ+pC+wNAAyQDd8Lig1JC5wOgAKI8Qr18/K99Ejz5fRK83f1S/Kz/X0NSQooDIwK1wtkCtML1wnCDCUCcvNw9rP0MvYE9Vn2CfXh9i/0Af6iC9MIdwoJCSoK4QggCmUI4grWAU715Pdl9rj3rPbh97f2WvgF9l7+tQlxB7MImAdqCHEHXAgBB/cIjwEk91v5Gfgz+Wj4TvmJ+Jr5Sfgt/DAHSwbMBj0GmgYTBn8GxQW6BnMDnPmC+gH6jfo2+rX6U/r9+if6BP2HBbwELAWsBPkEhATaBEEE+ASFAlf7Bfyo+xn82vtB/Pz7f/zo+/T9yAM/A4ADIwNSA/oCLwPDAjEDmwEQ/Yb9VP2h/YH9y/2l/QH+rP3g/g0CwAHRAaABpQF2AYEBRAFuAa0Azf4E/wH/J/8u/0z/Wf93/3r/w/9bADcALQASAAEATElTVDYAAABJTkZPSUNSRAYAAAAyMDIyAABJU0ZUHAAAAExhdmY1OS4yNy4xMDAgKGxpYnNuZGZpbGUtKQBpZDMgBAEAAElEMwMAAAAAAXlURFJDAAAABQAAADIwMjJUWFhYAAAAFwAAAFNvZnR3YXJlAExhdmY1OS4yNy4xMDBUWFhYAAAAEQAAAG1ham9yX2JyYW5kAGRhc2hUWFhYAAAAEAAAAG1pbm9yX3ZlcnNpb24AMFRYWFgAAAAgAAAARW5jb2RlZCBieQBMQU1FIGluIEZMIFN0dWRpbyAyMFRYWFgAAAAbAAAAY29tcGF0aWJsZV9icmFuZHMAaXNvNm1wNDFUWFhYAAAAFgAAAEVOQ09ERVIAVW5yZWFsRW5naW5lNFRYWFgAAAAbAAAAQlBNIChiZWF0cyBwZXIgbWludXRlKQAxNDAA");  
    snd.play();
  }

  function fetchMessages() {
    fetch('/messages')
      .then(response => response.text())
      .then(data => {
        let shouldScroll = chat.scrollTop + chat.clientHeight >= chat.scrollHeight;

        // Check if there are new messages
        if (data !== chat.innerHTML) {
          chat.innerHTML = processMessages(data);

          // Find the last message ID
          let newLastMessageId = '';
          let messages = chat.getElementsByClassName('message');
          if (messages.length > 0) {
            let lastMessage = messages[messages.length - 1];
            newLastMessageId = lastMessage.id.replace('msg_', '');
          }

          // Play sound only if there are new messages
          if (lastMessageId !== newLastMessageId) {
            beep();
            lastMessageId = newLastMessageId;
          }

          if (shouldScroll) {
            chat.scrollTop = chat.scrollHeight;
          }
        }
      });
  }

  function processMessages(messages) {
    // Function for processing messages and replacing links to images
    return messages.replace(/(https?:\/\/[^\s]+?\.(jpg|jpeg|png|gif))/gi, function (match) {
      return '<img src="' + match + '" />';
    });
  }

  function sendMessage() {
    let message = messageInput.value;
    if (message.trim() === '' || !window.currentUser) return;

    if (message.length > 200) {
      messageError.textContent = 'Message is too long';
      return;
    } else {
      messageError.textContent = '';
    }

    let now = new Date();
    let timeString = now.toLocaleTimeString();

    fetch('/send', {
      method: 'POST',
      headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
      body: 'message=' + encodeURIComponent(message) + '&user=' + encodeURIComponent(window.currentUser) + '&time=' + encodeURIComponent(timeString)
    }).then(() => {
      messageInput.value = '';
      fetchMessages();
    });
  }

  function setUser() {
    let username = usernameInput.value.trim();
    if (username.length === 0) return;

    if (username.length > 20) {
      usernameError.textContent = 'Username is too long';
      return;
    } else {
      usernameError.textContent = '';
    }

    window.currentUser = username;
    document.getElementById("userSection").style.display = 'none';
    fetchMessages();
  }

  function checkEnter(event) {
    if (event.key === 'Enter') {
      sendMessage();
      event.preventDefault();
    }
  }

  messageInput.addEventListener('keypress', checkEnter);
  setInterval(fetchMessages, 3000);
</script>
</body>
</html>
)rawliteral";
  server.send(200, "text/html", html);
}


void handleSend() {
  if (server.hasArg("message") && server.hasArg("user") && server.hasArg("time")) {
    String newMessage = server.arg("message");
    String user = server.arg("user");
    String time = server.arg("time");

    // Generate a unique message identifier
    String messageId = String(millis()); // You can use a more complex mechanism for uniqueness

    // Check message length
    if (newMessage.length() > MAX_MESSAGE_LENGTH) {
      server.send(400, "text/plain", "Message is too long");
      return;
    }

    // Check the length of the username
    if (user.length() > MAX_USERNAME_LENGTH) {
      server.send(400, "text/plain", "Username is too long");
      return;
    }

    // Protection from flood attacks
    unsigned long currentMillis = millis();
    if (currentMillis - lastMessageTime < FLOOD_INTERVAL) {
      server.send(429, "text/plain", "Too many requests. Please wait before sending another message.");
      return;
    }

    // Define the message class
    String messageClass = (user == currentUser) ? "user-sent" : "user-received";

    // Add a message with username, time and ID
    messages += "<p id='msg_" + messageId + "' class='message " + messageClass + "'><b>" + user + "</b> (" + time + "): " + newMessage + "</p>";
    lastMessageTime = currentMillis;
  } else {
    server.send(400, "text/plain", "Invalid request");
  }
  server.send(200, "text/plain", "Message received");
}

void handleMessages() {
  server.send(200, "text/html", messages);
  // Messages are not cleared when requested
}

void handleSetUser() {
  if (server.hasArg("user")) {
    String user = server.arg("user");

    // Check the length of the username
    if (user.length() > MAX_USERNAME_LENGTH) {
      server.send(400, "text/plain", "Username is too long");
      return;
    }

    currentUser = user;
  }
  server.send(200, "text/plain", "User set");
}

void handleSystemMessage() {
  if (server.hasArg("message")) {
    String systemMessage = server.arg("message");

    // Check the length of the system message
    if (systemMessage.length() > 500) {
      server.send(400, "text/plain", "System message is too long");
      return;
    }

    // Add a system message to the chat
    messages += "<p class='message system'>" + systemMessage + "</p>";
    server.send(200, "text/plain", "System message added");
  } else {
    server.send(400, "text/plain", "No system message provided");
  }
}
