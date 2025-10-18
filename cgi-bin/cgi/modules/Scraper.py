import requests
import html
from bs4 import BeautifulSoup

class scraper:
    def __init__(self, headers, category, url):
        self.url = url
        self.content = []
        self.response = None
        self.content = None
        self.headers = headers
        self.category = category

    def make_response(self):
        self.response = requests.get(self.url, headers=self.headers)
        if self.response.status_code != 200:
            raise Exception(f"Can't send request to {self.url}\nstatus code: {self.response.status_code}")
        self.content = BeautifulSoup(self.response.text, 'lxml')

    def scrape_web(self):
        main = self.content.find("main")
        main_event = main.find("div", {"data-testid": f"{self.category}-front-lead-article"})
        src_image = main_event.img['src']
        header_a = main_event.find("a", {"data-testid": "flexcard-headline"})
        link = header_a["href"]
        header = header_a.text
        description = main_event.find('p', {"data-testid": "flexcard-text"})
        read_time = main_event.find('p', {"data-testid": "flexcard-readtime"})
        
        content = [(src_image, link, header, description.text, read_time.text)]
        front_articles = main.find_all("div", {"data-testid": f"{self.category}-front-article"})
        for article in front_articles:
            a = article.find("a", {"slotname": "media"})
            header = article.find('a', {"data-testid": "flexcard-headline"})
            description = article.find('p', {"data-testid": "flexcard-text"})
            read_time = article.find('p', {"data-testid": "flexcard-readtime"})
            content.append((a.img['src'], a["href"], header.text, description.text, read_time))
        return content
