
#include "ConfigFile.hpp"


	// utils functions
	long convert_long(const std::string &data)
	{
		int			signe;
		long		res;
		long	max = std::numeric_limits<long>::max();

		res = 0;
		signe = 1;
		const char *str = data.c_str();
		while ((*str >= 9 && *str <= 13) || *str == 32)
			str++;

		while (*str)
		{
			if (!isdigit(*str))
				throw std::invalid_argument("Invalid number");
			if ((res > (max / 10)) || (res == (max / 10) && *str > '7'))
			{
				throw std::out_of_range("Overflow detected");
			}
			res = res * 10 + (*str - 48);
			str++;
		}
		return (res * signe);
	}
	//////////////////////////////////////////////////

	// inside location
    void ConfigFile::ParseAutoindex()
	{
		std::string check = get_data();
		if (!check.empty())
		{
			if (check.compare("on") == 0)
				location.autoindex = true;
			else
				location.autoindex = false;
		}
		verifyDelimiterLocation(SEMICOLON);
	}

	std::string ConfigFile::get_method()
	{
		size_t pos;
		std::string data;

		if (i >= word.length() || check_semi == 3)
		{
			return ("");
		}
		pos = word.find(";", i);
		data = word.substr(i, pos);
		if (!data.compare("get"))
		{
			location.methods.push_back("get");
		}
		else if ((!data.compare("post")))
		{
			location.methods.push_back("post");
		}
		else if ((!data.compare("delete")))
		{
			location.methods.push_back("delete");
		}
		else if ((data.length() == 0 && !check_semi))
			throw std::invalid_argument("syntax error : fail to get data");
		else
			return ("");
		std::cout << data << std::endl;
		i += pos;
		check_semi++;
		return (data);
	}

    void ConfigFile::ParseMethods()
	{
		get_method();
		verifyDelimiterLocation(SEMICOLON);
	}
	
    void ConfigFile::ParseUpload()
	{
		std::string data = get_data();
		if (!data.empty())
		{
			location.upload_store = data;
		}
		verifyDelimiterLocation(SEMICOLON);
	}

    void ConfigFile::ParseRedir()
	{
		std::string data = get_data(2);
		if (!data.empty())
		{
			if (check_semi == 1)
			{
				indexOfRedircat = convert_long(data);
				if (indexOfRedircat < 100 || indexOfRedircat > 599)
					throw std::out_of_range("value " + data + " must be between 300 and 599");
			}
			else
			{
				location.redirect[indexOfRedircat] = data;
			}
		}
		if (check_semi == 2)
			verifyDelimiterLocation(SEMICOLON);
	}
	
    void ConfigFile::ParseLocationRoot()
	{
		std::string data = get_data();
		if (!data.empty())
		{
			location.root = data;
		}
		verifyDelimiterLocation(SEMICOLON);
	}
	
	void ConfigFile::ParseCGI()
	{
		std::string data = get_data();
		if (!data.empty())
		{
			location.cgi_ext = data;
		}
		verifyDelimiterLocation(SEMICOLON);
	}	

    void ConfigFile::ParseCGIPath()
	{
		std::string data = get_data();
		if (!data.empty())
		{
			location.cgi_Path_Info = data;
		}
		verifyDelimiterLocation(SEMICOLON);
	}

	void ConfigFile::ParseIndex()
	{
		std::string data = get_data();
		if (!data.empty())
		{
			location.index = data;
		}
		verifyDelimiterLocation(SEMICOLON);
	}

	
	//////// inside server /////////
    void ConfigFile::ParseErrorPages()
	{
		std::string data = get_data(2);
		if (!data.empty())
		{
			if (check_semi == 1)
			{
				indexOfErrorPages = convert_long(data);
				if (indexOfErrorPages < 300 || indexOfErrorPages > 599)
					throw std::out_of_range("value " + data + " must be between 300 and 599");
			}
			else
			{
				server.error_pages[indexOfErrorPages] = data;
			}
		}
		if (check_semi == 2)
			verifyDelimiter(SEMICOLON);
	}

	void ConfigFile::ParseGlobalRoot()
	{
		std::string data = get_data();
		if (!data.empty())
		{
			server.global_root = data;
		}
		verifyDelimiter(SEMICOLON);
	}


    void ConfigFile::ParsePort()
	{
		std::string data = get_data();
		if (!data.empty())
		{
			server.port = convert_long(data);
			if (server.port < 1 || server.port > 65535)
				throw std::runtime_error("Port must be between 1 and 65535");
		}
		verifyDelimiter(SEMICOLON);
	}
	
	void ConfigFile::ParseDomain()
	{
		std::string data = get_data();
		if (!data.empty())
		{
			server.server_name = data;
		}
		verifyDelimiter(SEMICOLON);
	}

	void ConfigFile::ParseClientMaxBodySize()
	{
		std::string data = get_data();
		if (!data.empty())
		{
			server.client_max_body_size = convert_long(data);
		}
		verifyDelimiter(SEMICOLON);
	}

	// parsing inside server

		// Checks if the current token represent the start of a "server" block
	void ConfigFile::verifyServerKeyword()
	{
		size_t pos;

		pos = word.find("server", i);
		if (pos == std::string::npos)
			throw std::invalid_argument("syntax error : in server keyword");
		index_of_t++;
		i+=6;
	}

	void ConfigFile::DispatchParser()
	{
		const std::string &key = &word[i];
		typedef void (ConfigFile::*ParserFunc)();
		const std::string keys[] = {
			"location",
			"error_page",
			"root",
			"server_name",
			"client_max_body_size",
			"listen"
		};
		ParserFunc funcs[] = {
			&ConfigFile::Parselocationblock,
			&ConfigFile::ParseErrorPages,
			&ConfigFile::ParseGlobalRoot,
			&ConfigFile::ParseDomain,
			&ConfigFile::ParseClientMaxBodySize,
			&ConfigFile::ParsePort
		};

		for (int j = 0; j < 6; ++j)
		{
			if (keys[j] == key)
			{
				call = funcs[j];
				index_of_t++;
				i += key.length();
				check_semi = 0;
				return;
			}
		}
		check_final = true;
	}
	
	void ConfigFile::Parselocationblock()
	{
		if (index_of_lm == 0)
		{
			verifyLocationPath();
		}
		if (index_of_lm == 1)
		{
			verifyDelimiterLocation(OPEN_BRACKET);
		}
		if (index_of_lm == 2)
		{
			DispatchParserLocation();
		}
	}

	bool ConfigFile::HasSpecialDelimiter(const std::string &data) const
	{
		for (size_t i = 0; i < data.length(); ++i)
		{
			if (data[i] == '{' || data[i] == '}')
			{
				return (true);
			}
		}
		return (false);
	}

	std::string ConfigFile::get_data(int max_data)
	{
		size_t pos;
		std::string data;

		if (i >= word.length() || check_semi == max_data)
		{
			return ("");
		}
		pos = word.find(";", i);
		data = word.substr(i, pos);
		if (data.length() == 0 || HasSpecialDelimiter(data))
			throw std::invalid_argument("syntax error : fail to get data");
		std::cout << data << std::endl;
		i += pos;
		check_semi++;
		return (data);
	}


	void ConfigFile::verifyDelimiter(CharSymbol char_symbol)
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
		}
		if (char_symbol == CLOSE_BRACKET)
		{
			index_of_t = 0;
			check_final =  false;
		}
	}
		
	



	// Location Syntax

	void ConfigFile::verifyLocationPath()
	{
		GetNameOfLocation();
	}

	void ConfigFile::GetNameOfLocation()
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

	void ConfigFile::verifyDelimiterLocation(CharSymbol char_symbol)
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

	bool ConfigFile::L_HasSpecialDelimiter(const std::string &data) const
	{
		for (size_t i = 0; i < data.length(); ++i)
		{
			if (data[i] == '}' || data[i] == ';')
			{
				return (true);
			}
		}
		return (false);
	}

	void ConfigFile::DispatchParserLocation()
	{
		const std::string& key = &word[i];
		typedef void (ConfigFile::*ParserFunc)();

		std::string names[] = {
			"autoindex",
			"methods",
			"cgi_ext",
			"cgi_Path_Info",
			"upload",
			"redir",
			"root",
			"index"
		};

		ParserFunc funcs[] = {
			&ConfigFile::ParseAutoindex,
			&ConfigFile::ParseMethods,
			&ConfigFile::ParseCGI,
			&ConfigFile::ParseCGIPath,
			&ConfigFile::ParseUpload,
			&ConfigFile::ParseRedir,
			&ConfigFile::ParseLocationRoot,
			&ConfigFile::ParseIndex
		};

		for (int j = 0; j < 8; ++j) {
			if (names[j] == key) {
				call = funcs[j];
				index_of_lm++;
				i += key.length();
				check_semi = 0;
				return;
			}
		}
		check_final_location = true;
	}

// main function
//tokenize
// [server, {, f1, }]
// f1 = 1    [location, name, {, f2 ,}]
// f2 = 1  [check_metod1, ;]
// f1 = 2  [check_method2, ;]

void ConfigFile::parse(const std::string& file_path)
{
    std::fstream		file(file_path.c_str());
    std::string			line;
    std::stringstream	string;

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
					verifyServerKeyword();
				}
				if (index_of_t == 1)
				{
					verifyDelimiter(OPEN_BRACKET);
				}
				if (index_of_t == 2)
				{
					DispatchParser();
				}
				if (index_of_t == 3)
				{
					(this->*call)();
				}
				if (check_final_location)
				{
					verifyDelimiterLocation(CLOSE_BRACKET);
					if (index_of_lm == 3)
					{
						check_final_location =  false;
					}
				}
				if (check_final)
				{
					verifyDelimiter(CLOSE_BRACKET);
					if (index_of_t == 3)
					{
						check_final =  false;
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