
#include "ConfigFile.hpp"

// Checks if the current token represent the start of a "server" block
void ConfigFile::verifyServerKeyword(std::string &word, bool &this_server)
{
    size_t pos;

	if (this_server == true)
		return;
	pos = word.find("server", 0);
	if (pos != 0)
		throw std::invalid_argument("syntax error : in server name");
	word = word.substr(6);
	this_server = true;
}

void ConfigFile::verifyDelimiter(std::string &word, int &curly_brac_s, bool &this_server)
{
	if (!word.length())
		return;
	if (word[0] != OPEN_BRACKET)
		throw std::invalid_argument("syntax error : in delimiter");
	std::string tmp = word;
	for (int i = 0; i < word.length(); i++)
	{
		if (curly_brac_s == 0 && word[i] == OPEN_BRACKET)
		{
			curly_brac_s++;
			tmp = word.substr(1);
		}
		else if (curly_brac_s == 1 && word[i] == CLOSE_BRACKET)
		{
			curly_brac_s--;
			this_server = false;
			word = tmp.substr(1);
			return;
		}
		if (curly_brac_s > 1)
			throw std::invalid_argument("syntax error : in delimiter");
	}
	word = tmp;
}

void ConfigFile::parse(const std::string& file_path)
{
    std::fstream		file(file_path.c_str());
    std::string			line;
	std::string			word;
    std::stringstream	string;
	bool				this_server = false;
	int					curly_brac_s = 0;

    if (!file.is_open())
		throw std::runtime_error("Cannot open config file");

    while (std::getline(file , line))
	{
		string << line;
		while (string >> word)
		{
			while (word.length())
			{
				verifyServerKeyword(word, this_server);
				verifyDelimiter(word, curly_brac_s, this_server);
			}
		}
		string.str("");
		string.clear();
	}
	// std::cout << curly_brac_s << std::endl;
	if (curly_brac_s != 0)
		throw std::invalid_argument("syntax error : --in delimiter");
}