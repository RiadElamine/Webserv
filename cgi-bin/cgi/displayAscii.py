#! /Volumes/KINGSAVE/Webserv/cgi-bin/cgi/env/bin/python3

import sys
import requests
from pyfiglet import Figlet

def create_ascii(name, font, color=None):
    fig = Figlet(font=font)
#    fig = Figlet(font='doom')
    return fig.renderText(name).encode('utf-8')
#    return fig.renderText(name).encode('ascii', errors='replace')
#    return f"<pre>\n{ascii_art}</pre>"
    
    
#def makeBody(name, font, color):
#    with open("ascii.html") as file:
#        ascii_text = create_ascii(name, font)
#        html = file.read()
#        print(html)
#
#makeBody("oussama", 'doom', "white")

ascii_bytes = create_ascii('oussama', 'doom')

# Headers as bytes
headers = (
    b"HTTP/1.1 200 OK\r\n"
    b"Connection: close\r\n"
    b"Content-Type: text/plain; charset=utf-8\r\n"
    b"Content-Length: " + str(len(ascii_bytes)).encode('ascii') + b"\r\n"
    b"Server: Webserver/1.1.0\r\n"
    b"\r\n"
)

# still need to read input file to determine font and name

# Write headers
sys.stdout.buffer.write(headers)

# Write body
sys.stdout.buffer.write(ascii_bytes)
sys.stdout.buffer.flush()
