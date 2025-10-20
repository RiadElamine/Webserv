import requests
from bs4 import BeautifulSoup

class ScienceScraper():
    def __init__(self):
        self.url = 'https://www.livescience.com/'
        self.content = None
    def make_response(self):
        response = requests.get(self.url) 
        if (response.status_code != 200):
            raise Exception(f"Can't send request to {self.url}, status code: {response.status_code}")
        self.content = BeautifulSoup(response.text, 'lxml')
    
    def scrape_web(self):
        content = []
        main = self.content.find('section', {"id": "homePageCarousel"})
        for i in range(1, 7):
            article = main.find("div", {"id": f"Item{i}"})
            link = article.find('a', class_='article-link')
            figure = link.figure
            src_image = figure.picture.img
            caption = figure.figcaption.find_all('span')
            header = caption[0]
            description = caption[1]
            content.append((src_image['src'], link['href'], header.text, description.text, "None"))
        
        return content

