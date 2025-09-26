import requests
import sys

# url = 'https://httpbin.org/filedoesntexist'
url = sys.argv[1]
def send_get_request():
    response = requests.get(url)
    for item in response.headers.items():
        print(item[0], ": ", item[1])
    
    print(response.text)

if __name__ == "__main__":
    send_get_request()