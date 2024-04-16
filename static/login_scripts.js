var username;

function Connect() {
   username = document.getElementById('username').value;
   localStorage.setItem('username', username);

   fetch('/login', {
      method: 'POST',
      headers: {
         'Content-Type': 'application/json'
      },
      body: JSON.stringify({'username': username})
   }).then(response => response.json())
   .then(data => {
     console.log(data);
     if (data.status === 'success') {
       window.location.href = '/server';
     }
   })
   .catch(error => console.error('Error:', error));
}