import requests
from pyfiglet import Figlet

def create_ascii(name):
    fig = Figlet(font='doom')
    ascii_art = fig.renderText(name)
    return f"<pre>\n{ascii_art}</pre>"
    
    
def create_html_page(name_to_render):
    return '''<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>ASCII Generator</title>
<style>
  body {
    background-color: #0a0a0f;
    color: #00ffe0;
    font-family: 'Courier New', monospace;
    display: flex;
    flex-direction: column;
    align-items: center;
    justify-content: center;
    min-height: 100vh;
    margin: 0;
    padding: 0;
    text-shadow: 0 0 6px #00ffe0, 0 0 20px #007a70;
  }

  h1 {
    font-size: 2em;
    letter-spacing: 3px;
    margin-bottom: 20px;
    border-bottom: 2px solid #00ffe0;
    padding-bottom: 5px;
  }

  .ascii-container {
    background: rgba(0, 255, 224, 0.05);
    border: 1px solid #00ffe0;
    border-radius: 12px;
    padding: 30px;
    max-width: 90%;
    overflow-x: auto;
    box-shadow: 0 0 25px #00ffe0a0, inset 0 0 20px #00ffe020;
  }

  pre {
    font-size: 1rem;
    line-height: 1.2;
    margin: 0;
    white-space: pre;
  }

  footer {
    position: fixed;
    bottom: 10px;
    font-size: 0.9rem;
    color: #009987;
  }
</style>
</head>
<body>

  <h1>Futuristic ASCII Generator</h1>
  <div class="ascii-container">\n''' + create_ascii(name_to_render) + '''</div>

  <footer>Type a name → Generate ASCII → Paste it here 🚀</footer>

</body>
</html>'''


print(create_html_page("oussama"))
