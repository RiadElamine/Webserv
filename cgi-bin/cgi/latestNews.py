import requests
import html
from bs4 import BeautifulSoup
import re

class ScrapeFinance:
    def __init__(self, headers):
        self.url = 'https://www.wsj.com/finance'
        self.content = []
        self.response = None
        self.content = None
        self.headers = headers
        self.unused_content = r'\s*By\s+[A-Z][\w.\s,&-]+?\d+\s*min read'

    def make_response(self):
        self.response = requests.get(self.url, headers=self.headers)
        if self.response.status_code != 200:
            raise Exception(f"Can't send request to {self.url}\nstatus code: {self.response.status_code}")
        self.content = BeautifulSoup(self.response.text, 'lxml')

    def scrape_web(self):
        main = self.content.find("main")
        main_event = main.find("div", {"data-testid": "finance-front-lead-article"})
        src_image = main_event.img['src']
        header_a = main_event.find("a", {"data-testid": "flexcard-headline"})
        link = header_a["href"]
        description = main_event.text
        header = header_a.text
        clean_description = re.sub( self.unused_content, '', description).strip()
        

        # front_articles = main.find_all("div", {"data-testid": "finance-front-article"})
        # for article in front_articles:
        #     p = article.find("p", {"data-testid": "flexcard-text"})
        #     print(article.text)
        #     print(p)
        #     print()
        return (src_image, link, header, clean_description)

    def fill_content(self):
        template = """<article class="news-card">
                    <div class="news-image-wrapper">
                        <img src="{src}" alt="{category} news" class="news-image">
                        <span class="news-badge">{category}</span>
                    </div>
                    <div class="news-content">
                        <h3 class="news-title">
                            <a href="{a_href}">{header}</a>
                        </h3>
                        <p class="news-description">
                            {description}
                        </p>
                        <div class="news-meta">
                            <span class="news-source">Financial Times</span>
                            <span class="news-date">{time}</span>
                        </div>
                    </div>
                </article>"""
        with open("News.html", "r", encoding='utf-8') as read_cont:
            soup = BeautifulSoup(read_cont.read(), 'lxml')
            # finance_section = soup.find("section", class_="finance")
            news_row = soup.select_one("section.finance .news-row")
            content = self.scrape_web()
            new_article = BeautifulSoup(template.format(
                    src=content[0],
                    category="Finance",
                    a_href=content[1],
                    header=content[2],
                    description=content[3],
                    time="2 hours ago"
                ), "lxml")
            news_row.append(new_article)
            with open("test.html", "w", encoding='utf-8') as f:
                f.write(soup.decode())

            

    def print_content(self):
        print(self.content)


# url = 'https://www.wsj.com/finance?mod=nav_top_section'

# response = requests.get(url)
# soup = BeautifulSoup(response.text, "html.parser")
# print(soup)

if __name__ == "__main__":
    cookie = "datadome=fHepgNjln_c9L2vgYOKY81a26lgRu5IDpPxLA8RAeXE3S62XRbUYIhNSIjgzJgyIlH5dX8qam16gyxbhQsTJlOCGp_JregUgh1ZgBl0VV9zU0oEx6YUrc8tcyez7rpCD; ab_uuid=7c026cae-2661-4997-aada-39a49a873cfb; connect.sid=s%3Aal0CZkyOs4PvRoCBxxUArjsWBf1hGZUI.DYNmCsLuX8lORkwhO0t9mTjjbHZZd9dLJPI5fxd0Z2A; _pctx=%7Bu%7DN4IgrgzgpgThIC4B2YA2qA05owMoBcBDfSREQpAeyRCwgEt8oBJAEzIE4AmHgZgEYAHADZ%2BvQQFYuAFhEcADPJABfIA; ca_r=www.wsj.com; _pcid=%7B%22browserId%22%3A%22mgs6pk3jtwxlvqpo%22%7D; __pat=-14400000; xbc=%7Bkpcd%7DChBtZ3M2cGszanR3eGx2cXBvEgpLS2JncXBCbHB1GjxQVlhVOVNqRjBEdXFzc3RPclV6MDJUaDhjR0N1Q29YWGlzMER0VTRKaWpCTmFRQnZHZmJZNHhQdnZoQlYgAA; cX_P=mgs6pk3jtwxlvqpo; LANG=en_US; LANG_CHANGED=en_US; __pvi=eyJpZCI6InYtbWdzNnBrM3I2cHh5ZnYxaiIsImRvbWFpbiI6Ii53c2ouY29tIiwidGltZSI6MTc2MDU0NTQ4Njg5N30%3D; __tbc=%7Bkpcd%7DChBtZ3M2cGszanR3eGx2cXBvEgpLS2JncXBCbHB1GjxQVlhVOVNqRjBEdXFzc3RPclV6MDJUaDhjR0N1Q29YWGlzMER0VTRKaWpCTmFRQnZHZmJZNHhQdnZoQlYgAA; _pcus=eyJ1c2VyU2VnbWVudHMiOnsiQ09NUE9TRVIxWCI6eyJzZWdtZW50cyI6WyJhYTlxNmU0czFuazgiLCJhYTlxNmU0czFua2IiLCJhYXZ3ZGdycW02d3EiLCJDU2NvcmU6Mzk5ODk5NGRiMDcyYWMwMzhlYmI2NWU2YjhjOWRkYWMyNWJkYjU5Mjo5IiwiTFRyZXR1cm46ZTA0Yzk4YmYzYmZjNGMxODY2MGNkODM3NDk1MTNmOWEyYTEwNDAyMDpub19zY29yZSIsIkNTY29yZTpkZDM4ODU2ZTkzOTRmMjEzZjUxYWRkY2MxYWY5M2I0NzBlODQ3NzkzOjkiLCJMVHM6OTFiMThhNzYwMGYyYjkyNmI2N2IyZTYwNmIwYTcwNjcxZTk0ZGI3MDpub19zY29yZSJdfX19; AMCVS_CB68E4BA55144CAA0A4C98A5%40AdobeOrg=1; AMCV_CB68E4BA55144CAA0A4C98A5%40AdobeOrg=1585540135%7CMCIDTS%7C20377%7CMCMID%7C68006565037962979840729895842803082115%7CMCAID%7CNONE%7CMCOPTOUT-1760552691s%7CNONE%7CvVersion%7C4.4.0; usr_prof_v2=eyJpYyI6M30%3D"
    user_agent = 'Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/141.0.0.0 Safari/537.36'
    headers = {
            "User-Agent": user_agent,
            "Accept": "text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8",
            "Accept-Language": "en-US,en;q=0.9",
            "Connection": "keep-alive",
            "Referer": "https://www.wsj.com/",
            "Upgrade-Insecure-Requests": "1",
            "DNT": "1",  # Do Not Track
            "Cookie": cookie
        }
    scrape = ScrapeFinance(headers)
    scrape.make_response()
    scrape.fill_content()