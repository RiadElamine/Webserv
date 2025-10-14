import requests
import html
from bs4 import BeautifulSoup



url = 'https://www.wsj.com/finance?mod=nav_top_section'

response = requests.get(url)
soup = BeautifulSoup(response.text, "html.parser")
print(soup)

