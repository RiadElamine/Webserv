
#include "../Includes/ConfigFile.hpp"

ConfigFile::ConfigFile(){}

ConfigFile::ConfigFile(int argc, char **argv)
{
    if (argc > 2)
        throw std::runtime_error("Too many arguments provided.");

    std::string configFilePath = (argc == 2) ? argv[1] : "./Config/default.conf";

	ConfigFileStream.open(configFilePath);

    if (!ConfigFileStream.is_open())
        throw std::runtime_error("Cannot open config file");

	if (ConfigFileStream.peek() == EOF)
		throw std::runtime_error("Config file is empty");

	Initialize();
}

void ConfigFile::Initialize()
{
	index_of_t = 0;
	index_of_lm = 0;
	i = 0;
	check_semi = 0;
	indexOfErrorPages = 0;
	indexOfRedircat = 0;
	length = 0;

	servers = new std::vector<ServerConfig>();
	server.reset();
	location.reset();
	reset();
	resetLocation();

	funcs.insert(std::make_pair("location", &ConfigFile::Parselocationblock));
	funcs.insert(std::make_pair("error_page", &ConfigFile::ParseErrorPages));
	funcs.insert(std::make_pair("root", &ConfigFile::ParseGlobalRoot));
	funcs.insert(std::make_pair("server_name", &ConfigFile::ParseDomain));
	funcs.insert(std::make_pair("client_max_body_size", &ConfigFile::ParseClientMaxBodySize));
	funcs.insert(std::make_pair("listen", &ConfigFile::ParseListen));
	funcs.insert(std::make_pair("index", &ConfigFile::ParseIndexGlobal));

	funcs_location.insert(std::make_pair("autoindex", &ConfigFile::ParseAutoindex));
	funcs_location.insert(std::make_pair("methods", &ConfigFile::ParseMethods));
	funcs_location.insert(std::make_pair("cgi_ext", &ConfigFile::ParseCGI));
	funcs_location.insert(std::make_pair("cgi_Path_Info", &ConfigFile::ParseCGIPath));
	funcs_location.insert(std::make_pair("upload", &ConfigFile::ParseUpload));
	funcs_location.insert(std::make_pair("redir", &ConfigFile::ParseRedir));
	funcs_location.insert(std::make_pair("root", &ConfigFile::ParseLocationRoot));
	funcs_location.insert(std::make_pair("index", &ConfigFile::ParseIndex));
}

void Location::reset()
{
	URI.clear();
	root.clear();;
	index.clear();;
	methods.clear();;
	autoindex = false;
	redirect.clear();
	upload_store.clear();;
	cgi_ext.clear();
	cgi_Path_Info.clear();

}
void ServerConfig::reset() {
	listens.clear();
	server_name.clear();
	client_max_body_size = 1024 * 1024; // default 1MB
	global_root.clear();
	error_pages.clear();
	locations.clear();
}


void ConfigFile::reset() 
{
	directiveFlags["location"] = false;
	directiveFlags["error_page"] = false;
	directiveFlags["root"] = false;
	directiveFlags["server_name"] = false;
	directiveFlags["client_max_body_size"] = false;
	directiveFlags["listen"] = false;
	directiveFlags["index"] = false;
}

void ConfigFile::resetLocation()
{
	directiveFlagsLocation["autoindex"] = false;
	directiveFlagsLocation["methods"] = false;
	directiveFlagsLocation["cgi_ext"] = false;
	directiveFlagsLocation["cgi_Path_Info"] = false;
	directiveFlagsLocation["upload"] = false;
	directiveFlagsLocation["redir"] = false;
	directiveFlagsLocation["root"] = false;
	directiveFlagsLocation["index"] = false;
}

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
	std::string check = get_data_location();
	if (check.compare("on") == 0)
		location.autoindex = true;
	else
		location.autoindex = false;
}

void ConfigFile::addMethod(const std::string &method)
{
	if (std::find(location.methods.begin(), location.methods.end(), method) == location.methods.end())
		location.methods.push_back(method);
	else 
		throw std::invalid_argument("Duplicate mothod definition for " + method);
}
void ConfigFile::get_method()
{
	size_t pos;
	std::string data;

	pos = word.find(";", i);
	data = word.substr(i, pos);
	if (!data.compare("GET"))
		addMethod(data);
	else if ((!data.compare("POST")))
		addMethod(data);
	else if ((!data.compare("DELETE")))
		addMethod(data);
	else if ((i == pos && !check_semi))
		throw std::invalid_argument("syntax error : fail to get data");
	else 
		return ;

	if (pos == std::string::npos)
		i += data.length();
	else
		i += pos;

	check_semi++;
	if (check_semi == 3)
	{
		index_of_lm++;
		call = &ConfigFile::Parselocationblock;
	}
}

void ConfigFile::ParseMethods()
{
	get_method();
	if (i >= length)
		return;
	verifyDelimiterLocation(SEMICOLON);
}

void ConfigFile::ParseUpload()
{
	std::string data = get_data_location();
	location.upload_store = data;
}

void ConfigFile::ParseRedir()
{
	std::string data = get_data_location(2);
	if (check_semi == 1)
	{
		indexOfRedircat = convert_long(data);
		if (indexOfRedircat < 100 || indexOfRedircat > 599)
			throw std::out_of_range("value " + data + " must be between 300 and 599");
	}
	else
	{
		if (location.redirect.empty())
			location.redirect[indexOfRedircat] = data;
	}
}

void ConfigFile::ParseLocationRoot()
{
	std::string data = get_data_location();
	location.root = data;
}

void ConfigFile::ParseCGI()
{
	std::string data = get_data_location();
	location.cgi_ext = data;
}	

void ConfigFile::ParseCGIPath()
{
	std::string data = get_data_location();
	location.cgi_Path_Info = data;
}

void ConfigFile::ParseIndex()
{
	std::string data = get_data_location();
	location.index = data;
}


//////// inside server /////////
void ConfigFile::ParseErrorPages()
{
	std::string data = get_data(2);
	if (check_semi == 1)
	{
		indexOfErrorPages = convert_long(data);
		if (indexOfErrorPages < 300 || indexOfErrorPages > 599)
			throw std::out_of_range("value " + data + " must be between 300 and 599");
	}
	else
	{
		if (server.error_pages.find(indexOfErrorPages) == server.error_pages.end())
			server.error_pages[indexOfErrorPages] = data;
		else 
		{
			std::ostringstream errorPage ; errorPage << indexOfErrorPages;
			throw std::invalid_argument("Duplicate error_page definition for " + errorPage.str());
		}
	}
}

void ConfigFile::ParseGlobalRoot()
{
	std::string data = get_data();
	server.global_root = data;
}


void ConfigFile::ParseListen()
{
	std::string data = get_data();
	std::string host;

	struct addrinfo hints;
	struct addrinfo *res = NULL;

	memset(&hints, 0, sizeof(hints));

	hints.ai_family = AF_INET;
	hints.ai_flags = AI_NUMERICHOST;

	size_t pos = data.find(":");
	if (pos != std::string::npos)
	{
		host = data.substr(0, pos);
		data = data.substr(pos + 1);
		if (data.empty())
			throw std::invalid_argument("Port number is missing after host address");
		if (getaddrinfo(host.c_str(), NULL, &hints, &res) != 0)
			throw std::invalid_argument("Invalid host address: " + host);
	}
	else 
	{
		host = "0.0.0.0";
	}

	try
	{
		long port = convert_long(data);
		if ( port < 1 || port > 65535)
			throw std::runtime_error("Port must be between 1 and 65535");
		typedef std::set<std::pair<std::string, uint16_t> >::iterator it;
		std::pair<it, bool> existing = server.listens.insert(std::make_pair(host, static_cast<uint16_t>(port)));
		if (!existing.second)
			throw std::invalid_argument("Duplicate listen definition for " + host + ":" + data);
	}
	catch(const std::exception& e)
	{
		std::cout << "Error converting port: ";
		throw;
	}
	
	freeaddrinfo(res);
}

void ConfigFile::ParseDomain()
{
	std::string data = get_data();
	server.server_name = data;
}

void ConfigFile::ParseClientMaxBodySize()
{
	std::string data = get_data();
	server.client_max_body_size = convert_long(data);
}


void ConfigFile::ParseIndexGlobal()
{
	std::string data = get_data();
	server.global_index = data;
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

	if (directiveFlags.find(key) != directiveFlags.end())
	{
		if (directiveFlags[key] == true && key.compare("location") && key.compare("error_page") && key.compare("listen"))
			throw std::invalid_argument("Duplicate definition for " + key);
		call = funcs[key];
		i += key.length();
		index_of_t = 3;
		check_semi = 0;
		directiveFlags[key] = true;
		return;
	}
	index_of_t = 5;
}

void ConfigFile::Parselocationblock()
{
	if (index_of_lm == 0)
		verifyLocationPath();
	else if (index_of_lm == 1)
		verifyDelimiterLocation(OPEN_BRACKET);
	else if (index_of_lm == 2)
		DispatchParserLocation();
	else if (index_of_lm == 4)
		verifyDelimiterLocation(SEMICOLON);
	else if (index_of_lm == 5)
		verifyDelimiterLocation(CLOSE_BRACKET);
}


std::string ConfigFile::get_data(int max_data)
{
	size_t pos;
	std::string data;

	pos = word.find(";", i);
	data = word.substr(i, pos);
	if (data.empty() || data.find_first_of("{}") != std::string::npos)
		throw std::invalid_argument("syntax error : fail to get data");
	if (pos == std::string::npos)
		i+= data.length();
	else
		i += pos;

	check_semi++;
	if (check_semi == max_data)
		index_of_t++;

	return (data);
}


std::string ConfigFile::get_data_location(int max_data)
{
	size_t pos;
	std::string data;

	pos = word.find(";", i);
	data = word.substr(i, pos);
	if (i == pos || data.find_first_of("{}") != std::string::npos)
		throw std::invalid_argument("syntax error : fail to get data");
	if (pos == std::string::npos)
		i+= data.length();
	else
		i += pos;

	check_semi++;
	if (check_semi == max_data)
	{
		index_of_lm++;
		call = &ConfigFile::Parselocationblock;
	}
	return (data);
}


void ConfigFile::verifyDelimiter(CharSymbol char_symbol)
{
	if (word[i] != char_symbol)
		throw std::invalid_argument("syntax error : in delimiter");
	i++;
	if (char_symbol == CLOSE_BRACKET)
	{
		index_of_t = 0;
		servers->push_back(server);
		server.reset();
		reset();
	}
	else 
		index_of_t = 2;
}
	
// Location Syntax

void ConfigFile::checkDuplicateURIs(std::string &path_name)
{
	for (std::vector<Location>::iterator it = server.locations.begin(); it != server.locations.end(); ++it) {
		const std::string& uri = it->URI;
		if (!uri.compare(path_name))
			throw std::runtime_error("Duplicate URI found: " + uri);
	}
}

void ConfigFile::verifyLocationPath()
{
	size_t pos;
	std::string data;

	pos = word.find("{", i);
	data = word.substr(i, pos);
	if (data.empty() || data.find_first_of(";}") != std::string::npos)
		throw std::invalid_argument("syntax error : fail to get path of location");
	else if (pos != std::string::npos)
		i += data.length();
	else
		i += pos;
	checkDuplicateURIs(data);
	location.URI = data;
	index_of_lm++;
}

void ConfigFile::verifyDelimiterLocation(CharSymbol char_symbol)
{
	if (word[i] != char_symbol)
		throw std::invalid_argument("syntax error : delimiter in Location");
	i++;
	if (char_symbol == CLOSE_BRACKET)
	{
		index_of_t = 2;
		index_of_lm = 0;
		server.locations.push_back(location);
		location.reset();
		resetLocation();
	}
	else
	{
		index_of_lm = 2;
		call = &ConfigFile::Parselocationblock;
	}
}


	void ConfigFile::DispatchParserLocation()
	{
		const std::string& key = &word[i];

		if (directiveFlagsLocation.find(key) != directiveFlagsLocation.end())
		{
			if (directiveFlagsLocation[key] == true)
				throw std::invalid_argument("Duplicate definition for " + key);
			call = funcs_location[key];
			i += key.length();
			check_semi = 0;
			index_of_lm = 3;
			directiveFlagsLocation[key] = true;
			return;
		}

		index_of_lm = 5;
	}

	void ConfigFile::parse()
	{
	std::string			line;
	std::stringstream	sstring;

	while (std::getline(ConfigFileStream , line))
	{
		sstring << line;
		while (sstring >> word)
		{
			i = 0;
			length = word.length();
			while (i < length)
			{
				if (index_of_t == 0)
					verifyServerKeyword();
				else if (index_of_t == 1)
					verifyDelimiter(OPEN_BRACKET);
				else if (index_of_t == 2)
					DispatchParser();
				else if (index_of_t == 3)
					(this->*call)();
				else if (index_of_t == 4)
					verifyDelimiter(SEMICOLON);
				else if (index_of_t == 5)
					verifyDelimiter(CLOSE_BRACKET);
			}
		}
		sstring.str("");
		sstring.clear();
	}

	if (index_of_t != 0)
		throw std::invalid_argument("syntax error : in delimiter");

	fill_server_defaults();

}

void ConfigFile::fill_server_defaults()
{
	std::string errorDir = "./errors/";
	int codesArray[] = {
		400, 401, 403, 404, 405, 408,
		413, 414, 500, 501, 502, 503, 504
	};

	for (std::vector<ServerConfig>::iterator it = servers->begin(); it != servers->end(); ++it)
	{
		if (it->listens.empty())
			it->listens.insert(std::make_pair("0.0.0.0", 8080));
		if (it->global_root.empty())
			it->global_root = "/home/relamine/nginx-1.25.3/tt";
		if (it->global_index.empty())
			it->global_index = "index.html";
		for (std::vector<Location>::iterator loc = it->locations.begin(); loc != it->locations.end(); ++loc) {
			if (loc->root.empty())
				loc->root = it->global_root;
			if (loc->index.empty())
				loc->index = it->global_index;
			if (loc->methods.empty())
				loc->methods.push_back("GET");
			if (loc->upload_store.empty())
				loc->upload_store = "/tmp";
		}
		for (int i = 0 ; i != 13; ++i) 
		{
			std::ostringstream errorPage ; errorPage << codesArray[i];
			std::string filePath = errorDir + errorPage.str() + ".html";
			std::string &mappedPath = it->error_pages[codesArray[i]];
			if (mappedPath.empty() || access(mappedPath.c_str(), R_OK) != 0)
				mappedPath = filePath;
		}	
	}
}
