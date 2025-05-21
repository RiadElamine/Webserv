
#include "ConfigFile_copy.hpp"





    void ConfigFile::ParseAutoindex(std::string	&word)
	{
		(void)word;
	}
    void ConfigFile::ParseMethods(std::string	&word)
	{
		(void)word;
	}
    void ConfigFile::ParseCGI(std::string	&word)
	{
		(void)word;
	}
    void ConfigFile::ParseUpload(std::string	&word)
	{
		(void)word;
	}
    void ConfigFile::ParseRedir(std::string	&word)
	{
		(void)word;
	}
    void ConfigFile::ParseLocationRoot(std::string	&word)
	{
		(void)word;
	}






    void ConfigFile::ParseErrorPages(std::string	&word)
	{
		(void)word;
	}
    void ConfigFile::ParseRoot(std::string	&word)
	{
		(void)word;
	}
    void ConfigFile::ParsePort(std::string	&word)
	{
		(void)word;
	}



























// Checks if the current token represent the start of a "server" block
void ConfigFile::verifyServerKeyword(std::string &word)
{
    size_t pos;

	pos = word.find("server", i);
	if (pos == std::string::npos)
		throw std::invalid_argument("syntax error : in server name");
	index_of_t++;
	i+=6;
}

void ConfigFile::verifyDelimiter(std::string &word, CharSymbol char_symbol)
{
	if (i >= word.length())
	{
		return;
	}
	if (word[i] != char_symbol)
		throw std::invalid_argument("syntax error : in delimiter");
	index_of_t++;
	i++;
	if (char_symbol == SEMICOLON)
	{
		index_of_t = 2;
		check_semi = false;
	}
	if (char_symbol == CLOSE_BRACKET)
	{
		index_of_t = 0;
		check_final =  false;
		check_semi = false;
	}
}

//tokenize
// [server, {, f1, }]
// f1 = 1    [location, name, {, f2 ,}]
// f2 = 1  [check_metod1, ;]
// f1 = 2  [check_method2, ;]

void ConfigFile::DispatchParser(const std::string &key)
{
	typedef void (ConfigFile::*ParserFunc)(std::string&);
	const std::string keys[] = {
		"location",
		"error_page",
		"root",
		"domain",
		"port"
	};
	ParserFunc funcs[] = {
		&ConfigFile::Parselocationblock,
		&ConfigFile::ParseErrorPages,
		&ConfigFile::ParseRoot,
		&ConfigFile::ParseDomain,
		&ConfigFile::ParsePort
	};

	for (int j = 0; j < 5; ++j)
	{
		if (keys[j] == key)
		{
			call = funcs[j];
			index_of_t++;
			i += key.length();
			return;
		}
	}
	check_final = true;
}


void ConfigFile::DispatchParserLocation(const std::string& key)
{
    typedef void (ConfigFile::*ParserFunc)(std::string&);

    std::string names[] = {
        "autoindex",
        "methods",
        "cgi",
        "upload",
        "redir",
        "location_root",
        "index"
    };

    ParserFunc funcs[] = {
        &ConfigFile::ParseAutoindex,
        &ConfigFile::ParseMethods,
        &ConfigFile::ParseCGI,
        &ConfigFile::ParseUpload,
        &ConfigFile::ParseRedir,
        &ConfigFile::ParseLocationRoot,
        &ConfigFile::ParseIndex
    };

    for (int j = 0; j < 7; ++j) {
        if (names[j] == key) {
            call = funcs[j];
			index_of_lm++;
			i += key.length();
            return;
        }
    }
	check_final_location = true;
}

void verifyLocationPath(std::string	&word)
{
	(void)word;
}


bool ConfigFile::HasSpecialDelimiter(const std::string& word)
{
    for (size_t i = 0; i < word.length(); ++i)
	{
        if (word[i] == '{' || word[i] == '}')
		{
            return (true);
        }
    }
    return (false);
}

bool ConfigFile::L_HasSpecialDelimiter(const std::string& word)
{
    for (size_t i = 0; i < word.length(); ++i)
	{
        if (word[i] == '}' || word[i] == ';')
		{
            return (true);
        }
    }
    return (false);
}

void ConfigFile::get_data(std::string &word)
{
	size_t pos;
	std::string data;

	if (i >= word.length() || check_semi == true)
	{
		return;
	}
	pos = word.find(";", i);
	data = word.substr(i, pos);
	if (data.length() == 0 || HasSpecialDelimiter(data))
		throw std::invalid_argument("syntax error : fail to get data");
	std::cout << data << std::endl;
	i += pos;
	check_semi = true;
}


void ConfigFile::GetNameOfLocation(std::string &word)
{
	size_t pos;
	std::string data;

	if (i >= word.length())
	{
		return;
	}
	pos = word.find("{", i);
	data = word.substr(i, pos);
	if (data.length() == 0 || L_HasSpecialDelimiter(data))
		throw std::invalid_argument("syntax error : fail to get path of location");
	std::cout << data << std::endl;
	i += pos;
	index_of_lm++;
}

void ConfigFile::verifyDelimiterLocation(std::string &word, CharSymbol char_symbol)
{
	if (i >= word.length())
	{
		return;
	}
	if (word[i] != char_symbol)
		throw std::invalid_argument("syntax error : delimiter in Location");
	index_of_lm++;
	i++;

	if (char_symbol == SEMICOLON)
	{
		index_of_lm = 2;
		call = &ConfigFile::Parselocationblock;
		check_final_location =  false;
	}
	if (char_symbol == CLOSE_BRACKET)
	{
		index_of_t = 2;
		index_of_lm = 0;
		check_final_location =  false;
		check_final = false;
	}
}

void ConfigFile::verifyLocationPath(std::string	&word)
{
	GetNameOfLocation(word);
}
void ConfigFile::Parselocationblock(std::string	&word)
{
	if (index_of_lm == 0)
	{
		verifyLocationPath(word);
	}
	if (index_of_lm == 1)
	{
		verifyDelimiterLocation(word, OPEN_BRACKET);
	}
	if (index_of_lm == 2)
	{
		DispatchParserLocation(&word[i]);
	}

}

void ConfigFile::ParseIndex(std::string	&word)
{
	get_data(word);
	verifyDelimiterLocation(word, SEMICOLON);
}

void ConfigFile::ParseDomain(std::string &word)
{
	get_data(word);
	verifyDelimiter(word, SEMICOLON);
}

void ConfigFile::parse(const std::string& file_path)
{
    std::fstream		file(file_path.c_str());
    std::string			line;
	std::string			word;
    std::stringstream	string;
	bool				this_server = false;

    if (!file.is_open())
		throw std::runtime_error("Cannot open config file");
	
    while (std::getline(file , line))
	{
		string << line;
		while (string >> word)
		{
			i = 0;
			while (i < word.length())
			{
				if (index_of_t == 0)
				{
					verifyServerKeyword(word);
				}
				if (index_of_t == 1)
				{
					verifyDelimiter(word, OPEN_BRACKET);
				}
				if (index_of_t == 2)
				{
					DispatchParser(&word[i]);
				}
				if (index_of_t == 3)
				{
					(this->*call)(word);
				}
				if (check_final_location)
				{
					verifyDelimiterLocation(word, CLOSE_BRACKET);
					if (index_of_lm == 3)
					{
						check_final_location =  false;
						check_final = false;
					}
				}
				if (check_final)
				{
					verifyDelimiter(word, CLOSE_BRACKET);
					if (index_of_t == 3)
					{
						check_final =  false;
						check_semi = false;
					}
				}
			}
		}
		
		string.str("");
		string.clear();
	}
	if (index_of_t != 0)
	{
		throw std::invalid_argument("syntax error : in delimiter");
	}

}
