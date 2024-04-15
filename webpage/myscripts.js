var oldMessage, mostRecentMessage;
var username

function Connect() {
   //username = document.getElementById("username").value;
   //window.location = "server.html";

   let username = document.getElementById("username").value;
   fetch('/get_username', {
      method: 'POST',
      headers: {
         'Content-Type': 'application/json'
      },
      body: JSON.stringify({'username': username})
   }).then(response => response.json())
     .then(data => window.location = "server.html")

     setInterval(receiveMsg, 1000);
}

function SendMsg(ele) {
   //let username = "Owen";
   if(event.key === 'Enter' || event.button === 0) {
      const msg = document.getElementById("text").value;
      document.getElementById("text").value = "";
      //const please = document.querySelector('.message');

      //document.getElementById("text").value = "";

      fetch('/send_message', {
         method: 'POST',
         headers: {
            'Content-Type': 'application/json'
         },
         body: JSON.stringify({ 'username': username, 'message':msg })
      }).then(response => response.json())
      .then(data => {
         //handle response from the server
      });

      var newMessage = document.createElement("li");
      newMessage.innerHTML = `
      <li id="chatbox" style="margin-bottom:15px; font-size: 125%;" class="most-recent">
         <div class="usernameAndMsg">
            <p class="user">${username}</p>
            <p id="most-recent-message">${msg}</p>
         </div>
         <img class="sent" src="images/user-icon.png" width="40px">
      </li>
      `
      message.append(newMessage);
   }
}

function receiveMsg() {
   fetch('/get_message')
      .then(response => response.json())
      .then(data => {
         const message = data['message'];
         //update the GUI
      });
   msg = "This is a test..."
   username = "AI"
   var newMessage = document.createElement("li");
   newMessage.innerHTML = `
   <li id="chatbox" style="margin-bottom:15px; font-size: 125%;" class="r-most-recent">
      <img class="r-sent" src="images/user-icon.png" width="40px">
      <div class="r-usernameAndMsg">
         <p class="user">${/*sentUsername*/username}</p>
         <p id="most-recent-message">${msg}</p>
      </div>
   </li>
   `

   message.append(newMessage);
   storeText(msg);
}



function storeText(message) {
   mostRecentMessage = oldMessage
   message = mostRecentMessage
}