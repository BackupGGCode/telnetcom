#include "telnetcomPCH.hpp"
#include "config.hpp"


void exitWithError(const std::string &error)
{
	std::cout << error;
	std::cin.ignore();
	std::cin.get();

	exit(EXIT_FAILURE);
}

void ConfigFile::CheckConfigFile()
{
	std::ifstream file;
	file.open(CONFIG_FILE_NAME);

	if (!file)
	{
		std::ofstream myfilee (CONFIG_FILE_NAME);

		myfilee << ";## Asignacion de puertos de escucha.\n";
		myfilee << "Port1 = 23\n";
		myfilee << ";## Si los demas puertos son distintos a none, se iniciara un escuchador para cada uno.\n";
		myfilee << "Port2 = none\n";
		myfilee << "Port3 = none\n";
		myfilee << "\n";
		myfilee << ";## Si SerialPort = none, se deshabilita esta conexion.\n";
		myfilee << "SerialPort = none\n";
		myfilee << "SerialBauds = 9600\n";
		myfilee << "\n";
		myfilee << ";## Password necesaria para algunos comandos.\n";
		myfilee << "AdminPass = admin\n";
		myfilee << "ModPass = moderator\n";
		myfilee.close();

		exitWithError("CFG: Se ha creado el archivo de configuracion, editelo y reinicie el programa.\n");

	}
	else
		file.close();
}

void ConfigFile::removeComment(std::string &line) const
{
	if (line.find(';') != line.npos)
		line.erase(line.find(';'));
}

bool ConfigFile::onlyWhitespace(const std::string &line) const
{
	return (line.find_first_not_of(' ') == line.npos);
}

bool ConfigFile::validLine(const std::string &line) const
{
	std::string temp = line;
	temp.erase(0, temp.find_first_not_of("\t "));
	if (temp[0] == '=')
		return false;

	for (size_t i = temp.find('=') + 1; i < temp.length(); i++)
		if (temp[i] != ' ')
			return true;

	return false;
}

void ConfigFile::extractKey(std::string &key, size_t const &sepPos, const std::string &line) const
{
	key = line.substr(0, sepPos);
	if (key.find('\t') != line.npos || key.find(' ') != line.npos)
		key.erase(key.find_first_of("\t "));
}
void ConfigFile::extractValue(std::string &value, size_t const &sepPos, const std::string &line) const
{
	value = line.substr(sepPos + 1);
	value.erase(0, value.find_first_not_of("\t "));
	value.erase(value.find_last_not_of("\t ") + 1);
}

void ConfigFile::extractContents(const std::string &line)
{
	std::string temp = line;
	temp.erase(0, temp.find_first_not_of("\t "));
	size_t sepPos = temp.find('=');

	std::string key, value;
	extractKey(key, sepPos, temp);
	extractValue(value, sepPos, temp);

	if (!keyExists(key))
		contents.insert(std::pair<std::string, std::string>(key, value));
	else
		exitWithError("CFG: Can only have unique key names!\n");
}

void ConfigFile::parseLine(const std::string &line, size_t const lineNo)
{
	if (line.find('=') == line.npos)
		exitWithError("CFG: Couldn't find separator on line: " + Convert::T_to_string(lineNo) + "\n");

	if (!validLine(line))
		exitWithError("CFG: Bad format for line: " + Convert::T_to_string(lineNo) + "\n");

	extractContents(line);
}

void ConfigFile::ExtractKeys()
{
	std::ifstream file;
	file.open(fName.c_str());
	if (!file)
		exitWithError("CFG: El archivo de configuracion " + fName + " no existe!\n");

	std::string line;
	size_t lineNo = 0;
	while (std::getline(file, line))
	{
		lineNo++;
		std::string temp = line;

		if (temp.empty())
			continue;

		removeComment(temp);
		if (onlyWhitespace(temp))
			continue;

		parseLine(temp, lineNo);
	}

	file.close();
}

bool ConfigFile::GetBoolDefault(const char* name)
{
	ConfigFile cfg(CONFIG_FILE_NAME);

	std::string val = cfg.getValueOfKey<std::string>(name);

	return (val == "true" || val == "TRUE" || val == "yes" || val == "YES" ||
		val == "1");
}

int32 ConfigFile::GetIntDefault(const char* name, int32 def)

{
	ConfigFile cfg(CONFIG_FILE_NAME);
	return cfg.getValueOfKey<int32>(name, def);
}

float ConfigFile::GetFloatDefault(const char* name, float def)
{
	ConfigFile cfg(CONFIG_FILE_NAME);
	return cfg.getValueOfKey<float>(name, def);
}

std::string ConfigFile::GetStringDefault(const char* name, std::string def)
{
	ConfigFile cfg(CONFIG_FILE_NAME);
	return cfg.getValueOfKey<std::string>(name, def);
}