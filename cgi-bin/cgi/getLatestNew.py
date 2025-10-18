from modules import Scraper, ScienceScraper
from bs4 import BeautifulSoup
from modules.Info import TEMPLATE, HEADERS, PATH
# import cgi

# cgitb.enable()
# cgitb.enable(display=0, logdir="/tmp")


def fill_section(soup, template, contents, section, category, categorical):
        news_row = soup.select_one(section)
        for content in contents:
            new_article = BeautifulSoup(template.format(
                    src=content[0],
                    category=category,
                    Categorical=categorical,
                    a_href=content[1],
                    header=content[2],
                    description=content[3],
                    read_time=content[4]
                ), "lxml")
            news_row.append(new_article)

def fill_finance_content(headers, soup, template):
    finance_scraper = Scraper.scraper(headers, "finance", 'https://www.wsj.com/finance')
    finance_scraper.make_response()
    contents = finance_scraper.scrape_web()
    fill_section(soup, template, contents, "section.finance .news-row", "Finance", "Financial")

def fill_politics_content(headers, soup, template):
    politics_scraper = Scraper.scraper(headers, "politics", 'https://www.wsj.com/politics')
    politics_scraper.make_response()
    contents = politics_scraper.scrape_web()
    fill_section(soup, template, contents, "section.politics .news-row", "Politics", "Politicals")

def fill_Tech_content(headers, soup, template):
    Tech_scraper = Scraper.scraper(headers, "tech", 'https://www.wsj.com/tech')
    Tech_scraper.make_response()
    contents = Tech_scraper.scrape_web()
    fill_section(soup, template, contents, "section.technology .news-row", "Tech", "Technology")

def fill_Science_content(soup, template):
    scienceScraper = ScienceScraper.ScienceScraper()
    scienceScraper.make_response()
    contents = scienceScraper.scrape_web()
    fill_section(soup, template, contents, "section.science .news-row", "Science", "Science")

if __name__ == "__main__":
    try:
        soup = None
        with open(f"{PATH}/templates/News.html", "r", encoding='utf-8') as read_cont:
            soup = BeautifulSoup(read_cont.read(), 'lxml')

        fill_finance_content(HEADERS, soup, TEMPLATE)
        fill_politics_content(HEADERS, soup, TEMPLATE)
        fill_Tech_content(HEADERS, soup, TEMPLATE)
        fill_Science_content(soup, TEMPLATE)

        # with open("test.html", "w", encoding='utf-8') as f:
        #     f.write(soup.decode())
        print("Content-Type: text/html", end='\r\n')
        print(end="\r\n")
        print(soup.prettify())

    except Exception as e:
        print("Status: 500 Internal Server Error", end='\r\n')
        print("Content-Type: text/html", end='\r\n')
        print(end='\r\n')
        print(f"<html><body><h1>500 Internal Server Error</h1><p>{e}</p></body></html>")
