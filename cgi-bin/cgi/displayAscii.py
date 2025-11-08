from pyfiglet import Figlet
import os
import sys
from urllib.parse import parse_qs

def create_ascii(name, font, color=None):
    fig = Figlet(font=font)
    return fig.renderText(name).encode('utf-8')

# Get content length from environment
content_length = os.environ.get("CONTENT_LENGTH", "0")

# Parse incoming data
if content_length and content_length != "0":
    # Read POST data from stdin
    post_data = sys.stdin.read(int(content_length))
    
    # Parse the data (assumes URL-encoded format like: name=oussama&font=doom&color=white)
    params = parse_qs(post_data)
    
    # Extract parameters (parse_qs returns lists, so we get first item)
    name = params.get('text', [''])[0]
    font = params.get('font', ['doom'])[0]  # default to 'doom'
    color = params.get('color', [None])[0]
    
    # Create ASCII art
    ascii_bytes = create_ascii(name, font, color)
    
    # Headers as bytes
    headers = (
        b"Status: 200 OK\r\n"
        b"Connection: close\r\n"
        b"Content-Type: text/plain; charset=utf-8\r\n"
        b"Content-Length: " + str(len(ascii_bytes)).encode('ascii') + b"\r\n"
        b"Server: Webserver/1.1.0\r\n"
        b"\r\n"
    )
    
    # Write response
    sys.stdout.buffer.write(headers)
    sys.stdout.buffer.write(ascii_bytes)
else:
    # No data received
    error_msg = b"Error: No data received"
    headers = (
        b"Status: 400 Bad Request\r\n"
        b"Connection: close\r\n"
        b"Content-Type: text/plain; charset=utf-8\r\n"
        b"Content-Length: " + str(len(error_msg)).encode('ascii') + b"\r\n"
        b"Server: Webserver/1.1.0\r\n"
        b"\r\n"
    )
    sys.stdout.buffer.write(headers)
    sys.stdout.buffer.write(error_msg)