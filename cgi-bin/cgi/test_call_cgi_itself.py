"""
Date
Content-type
Content-lenght
Connection
Server
Location ?*
"""
import sys


def getHeader(bodyLength):

	return (
		f"Status: 200 OK\r\n"
		f"Content-Type: text/html\r\n"
		f"Content-Length: {bodyLength}\r\n"
		f"Connection: close\r\n"
		f"Server: WebServer/1.1.0\r\n\r\n"
	)
	
import os

#!/usr/bin/env python3
import urllib.request

def main():
	"""
		main - Entry point fo the program
	"""
	header = getHeader(10)
	sys.stdout.write(header)
	url = "http://localhost:8080/cgi/test_call_cgi_itself.py"

	try:
		with urllib.request.urlopen(url) as response:
			html = response.read().decode("utf-8")
			print("Content-Type: text/html\n")
			print(html)
	except Exception as e:
		print("Content-Type: text/plain\n")
		print("Error:", e)


if __name__ == "__main__":
	main()

