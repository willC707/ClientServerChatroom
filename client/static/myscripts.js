var username

window.onload = function() {
   username = localStorage.getItem('username');
}

var socket = io.connect('http://' + document.domain + ':' + location.port);
socket.on('server_response', function(msg) {
   console.log('Received message:', msg.data);

   if (msg.data.type == 'notification'){
      // Handle notification
      var newNotification = document.createElement("li");
      newNotification.style.backgroundColor = "rgb(2, 2, 41)"; // Change to desired background color
      newNotification.style.color = "white"; // Text color
      newNotification.style.border = "2px solid white"; // Border color and width
      newNotification.style.borderRadius = "10px"; // Rounded corners
      newNotification.style.padding = "2px"; // Padding
      newNotification.style.margin = "0px";
      newNotification.innerHTML = `
      <li id="notif" >
         <p style="margin: 0;">${msg.data.msg}</p>
      </li>
      `
      notifications.append(newNotification);
      
   } else if (msg.data.type == 'user_list'){
      // Handle user list
      for (let item of msg.data.users){
         var newUser = document.createElement("li");
         newUser.style.backgroundColor = "rgb(2, 2, 41)"; // Change to desired background color
         newUser.style.color = "white"; // Text color
         newUser.style.border = "2px solid white"; // Border color and width
         newUser.style.borderRadius = "10px"; // Rounded corners
         newUser.style.padding = "2px"; // Padding
         newUser.style.margin = "0px";
         newUser.innerHTML = `
         <li id="usr" >
            <p style="margin: 0;">${item}</p>
         </li>
         `
         users.append(newUser);
      }
   } else if (msg.data.type == 'group_list'){
      // Handle group list
      for (let item of msg.data.groups){
         var newGroup = document.createElement("li");
         newGroup.style.backgroundColor = "rgb(2, 2, 41)"; // Change to desired background color
         newGroup.style.color = "white"; // Text color
         newGroup.style.border = "2px solid white"; // Border color and width
         newGroup.style.borderRadius = "10px"; // Rounded corners
         newGroup.style.padding = "2px"; // Padding
         newGroup.style.margin = "0px";
         newGroup.innerHTML = `
         <li id="grp" >
            <p style="margin: 0;">${item}</p>
         </li>
         `
         groups.append(newGroup);
      }
   } else if (msg.data.type == 'board_list'){
      // Handle board list
      for (let item of msg.data.boards){
         var newBoard = document.createElement("li");
         newBoard.style.backgroundColor = "rgb(2, 2, 41)"; // Change to desired background color
         newBoard.style.color = "white"; // Text color
         newBoard.style.border = "2px solid white"; // Border color and width
         newBoard.style.borderRadius = "10px"; // Rounded corners
         newBoard.style.padding = "2px"; // Padding
         newBoard.style.margin = "0px";
         newBoard.innerHTML = `
         <li id="notif" >
            <p style="margin: 0;">${item}</p>
         </li>
         `
         boards.append(newBoard);
      }
   } else if (msg.data.type == 'new_message'){
      // Handle new message
      var newMessage = document.createElement("li");
      newMessage.innerHTML = `
      <li id="chatbox" style="margin-bottom:15px; font-size: 125%;" class="most-recent">
         <div class="usernameAndMsg">
            <p class="user">${msg.data.sender}</p>
            <p class="subject">${msg.data.subject}</p>
            <p id="most-recent-message">${msg.data.msg}</p>
         </div>
         <img class="sent" src="static/images/user-icon.png" width="40px">
      </li>
      `
      message.append(newMessage);
   } else if(msg.data.type == 'error'){
      // handle error
   }
});


function SendMsg() {
   if(event.key === 'Enter' || event.button === 0) {
      const subject = document.getElementById('subject').value;
      const msg = document.getElementById('text').value
      document.getElementById('subject').value="";
      document.getElementById('text').value="";

      fetch('/send_message', {
         method: 'POST',
         headers: {
            'Content-Type': 'application/json'

         },
         body: JSON.stringify({'subject':subject, 'message':msg})
      })
      .then(response => response.json())
      .then(data => {
            var newMessage = document.createElement("li");
            newMessage.innerHTML = `
            <li id="chatbox" style="margin-bottom:15px; font-size: 125%;" class="most-recent">
               <div class="usernameAndMsg">
                  <p class="user">${username}</p>
                  <p class="subject">${subject}</p>
                  <p id="most-recent-message">${msg}</p>
               </div>
               <img class="sent" src="static/images/user-icon.png" width="40px">
            </li>
            `
            message.append(newMessage);
      });
   }
}

function JoinBoard() {
   const board_name = document.getElementById('boardName').value;
   document.getElementById('boardName').value = "";

   fetch('/join_board',{
      method:'POST',
      headers: {
         'Content-Type': 'application/json'
      },
      body: JSON.stringify({'board_name':board_name})
   })
   .then(response=>response.json())
   .then(data=>console.log(data));

}

function Disconnect() {
   fetch('/disc_server', {
      method:'POST',
      headers: {
         'Content-Type': 'application/json'
      },
      body: JSON.stringify({'username':username})
   })
   .then(response=>response.json())
   .then(data=>console.log(data));
}

function GetBoards() {
   let listElement = document.getElementById('boards');
   listElement.innerHTML = '';
   fetch('/get_boards', {
      method:'POST',
      headers: {
         'Content-Type': 'application/json'
      },
      body: JSON.stringify({'username':username})
   })
   .then(response=>response.json())
   .then(data=>console.log(data));
}

function JoinGroup() {
   const group_name = document.getElementById('joinGroup').value;
   document.getElementById('joinGroup').value = '';
   fetch('/join_group', {
      method:'POST',
      headers: {
         'Content-Type': 'application/json'
      },
      body: JSON.stringify({'group_name': group_name })
   })
   .then(response=>response.json())
   .then(data=>console.log(data));
}

function LeaveGroup() {
   const group_name = document.getElementById('leaveGroup').value;
   document.getElementById('leaveGroup').value = '';
   fetch('/leave_group', {
      method:'POST',
      headers: {
         'Content-Type': 'application/json'
      },
      body: JSON.stringify({'group_name':group_name})
   })
   .then(response=>response.json())
   .then(data=>console.log(data));
}

function GetUsers() {
   fetch('/get_users', {
      method:'POST',
      headers: {
         'Content-Type': 'application/json'
      },
      body: JSON.stringify({'username':username})
   })
   .then(response=>response.json())
   .then(data=>console.log(data));
}

function GetGroup() {
   fetch('/get_group', {
      method:'POST',
      headers: {
         'Content-Type': 'application/json'
      },
      body: JSON.stringify({'username':username})
   })
   .then(response=>response.json())
   .then(data=>console.log(data));
}

function receiveMsg() {
   fetch('/receive_msge', {
      method:'POST',
      headers: {
         'Content-Type': 'application/json'
      },
      body: JSON.stringify({'msg_id': msg_id})
   })
   .then(response=>response.json())
   .then(data=>console.log(data));
}






// function SendMsg(ele) {
//    //let username = "Owen";
//    if(event.key === 'Enter' || event.button === 0) {
//       const msg = document.getElementById("text").value;
//       const subject = document.getElementById("subject").value;
//       document.getElementById("text").value = "";
//       document.getElementById("subject").value = "";
//       //const please = document.querySelector('.message');

//       //document.getElementById("text").value = "";

//       fetch('/send_message', {
//          method: 'POST',
//          headers: {
//             'Content-Type': 'application/json'
//          },
//          body: JSON.stringify({ 'message':msg, 'subject': subject })
//       }).then(response => response.json())
//       .then(data => {
//          //handle response from the server
//       });
      
//       var newMessage = document.createElement("li");
//       newMessage.innerHTML = `
//       <li id="chatbox" style="margin-bottom:15px; font-size: 125%;" class="most-recent">
//          <div class="usernameAndMsg">
//             <p class="user">${username}</p>
//             <p class="subject">${subject}</p>
//             <p id="most-recent-message">${msg}</p>
//          </div>
//          <img class="sent" src="static/images/user-icon.png" width="40px">
//       </li>
//       `
//       message.append(newMessage);
//    }
// }

// function receiveMsg() {
//    fetch('/get_message')
//       .then(response => response.json())
//       .then(data => {
//          const msg = data['message'];
//          const subject = data['subject'];
//          const user = data['username'];
//          const msgid = data['msgid'];
//          const date = data['date']
         
//          if(user != "none"){
//             var newMessage = document.createElement("li");
//             newMessage.innerHTML = `
//             <li id="chatbox" style="margin-bottom:15px; font-size: 125%;" class="r-most-recent">
//                <img class="r-sent" src="static/images/user-icon.png" width="40px">
//                <div class="r-usernameAndMsg">
//                   <p class="user">${user}</p>
//                   <p class="subject">${subject}</p>
//                   <p id="most-recent-message">${msg}</p>
//                </div>
//             </li>
//             `
   
//             message.append(newMessage);
//          }
//          else{
//             if(subject == "[SERVER]"){
//                var newMessage = document.createElement("li");
//                newMessage.innerHTML = `
//                <li id="chatbox" style="margin-bottom:15px; font-size: 125%;" class="r-most-recent">
//                   <img class="r-sent" src="static/images/user-icon.png" width="40px">
//                   <div class="r-usernameAndMsg">
//                      <p class="user">${subject}</p>
//                      <p id="most-recent-message">${msg}</p>
//                   </div>
//                </li>
//                `
      
//                message.append(newMessage);
//             }
//             if(subject == "USERS"){
//                var newMessage = document.createElement("li");
//                newMessage.innerHTML = `
//                <li id="chatbox" style="margin-bottom:15px; font-size: 125%;" class="r-most-recent">
//                   <img class="r-sent" src="static/images/user-icon.png" width="40px">
//                   <div class="r-usernameAndMsg">
//                      <p class="user">${subject}</p>
//                      <p id="most-recent-message">${msg}</p>
//                   </div>
//                </li>
//                `
      
//                message.append(newMessage);
//             }
//             if (subject == "BOARDS"){
//                var newMessage = document.createElement("li");
//                newMessage.innerHTML = `
//                <li id="chatbox" style="margin-bottom:15px; font-size: 125%;" class="r-most-recent">
//                   <img class="r-sent" src="static/images/user-icon.png" width="40px">
//                   <div class="r-usernameAndMsg">
//                      <p class="user">${subject}</p>
//                      <p id="most-recent-message">${msg}</p>
//                   </div>
//                </li>
//                `
      
//                message.append(newMessage);
//             }
//             if (subject == "MEMBERS"){
//                var newMessage = document.createElement("li");
//                newMessage.innerHTML = `
//                <li id="chatbox" style="margin-bottom:15px; font-size: 125%;" class="r-most-recent">
//                   <img class="r-sent" src="static/images/user-icon.png" width="40px">
//                   <div class="r-usernameAndMsg">
//                      <p class="user">${subject}</p>
//                      <p id="most-recent-message">${msg}</p>
//                   </div>
//                </li>
//                `
      
//                message.append(newMessage);
//             }
//          }
//       });
      
// }