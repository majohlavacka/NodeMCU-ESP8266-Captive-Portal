const char index_html[] = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Captive Portal</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      background: #f2f2f2;
      text-align: center;
      padding: 30px;
    }
    h1 {
      color: #333;
    }
    form {
      background: white;
      padding: 20px;
      margin: auto;
      width: 300px;
      border-radius: 8px;
      box-shadow: 0 0 10px #ccc;
    }
    input[type=email], input[type=password] {
      width: 90%;
      padding: 10px;
      margin: 8px 0;
      border: 1px solid #ccc;
      border-radius: 4px;
    }
    input[type=submit] {
      background-color: #4CAF50;
      color: white;
      padding: 12px 20px;
      border: none;
      border-radius: 4px;
      cursor: pointer;
      font-size: 16px;
    }
    input[type=submit]:hover {
      background-color: #45a049;
    }
  </style>
</head>
<body>
  <h1>Login</h1>
  <form action="/login" method="POST">
    <input type="email" name="email" placeholder="Email" required><br>
    <input type="password" name="password" placeholder="Password" required><br>
    <input type="submit" value="Login">
  </form>
</body>
</html>
)rawliteral";
