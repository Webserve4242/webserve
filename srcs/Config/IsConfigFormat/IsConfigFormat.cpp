#include <string>
#include "IsConfigFormat.hpp"

bool	IsConfigFormat::is_start_locationblock(const std::string &line)
{
	std::string		line_without_ows = HandlingString::obtain_without_ows_value(line);
	size_t			start_pos = 0;
	size_t			end_pos = 0;

	while (!(HandlingString::is_ows(line_without_ows[end_pos])) && line_without_ows[end_pos] != '\0')
		end_pos++;
	if (line_without_ows.substr(start_pos, end_pos - start_pos) != "location" || end_pos == line_without_ows.length())
		return (false);
	while (HandlingString::is_ows(line_without_ows[end_pos]))
		end_pos++;
	start_pos = end_pos;
	while (!(HandlingString::is_ows(line_without_ows[end_pos])) && line_without_ows[end_pos] != '\0')
		end_pos++;
	while (HandlingString::is_ows(line_without_ows[end_pos]))
		end_pos++;
	start_pos = end_pos;
	while (!(HandlingString::is_ows(line_without_ows[end_pos])) && end_pos != line_without_ows.length())
		end_pos++;
	if (line_without_ows.substr(start_pos, end_pos - start_pos) != "{" || end_pos != line_without_ows.length())
		return (false);
	return (true);
}

// locationのスタートなら　location *.cgi {のように<OWS> location <OWS> <文字列> <OWS> { <OWS>のみ許容
bool	IsConfigFormat::is_start_locationblock(const std::string &line, std::string *config_location_path)
{
	std::string		line_without_ows = HandlingString::obtain_without_ows_value(line);
	std::string		location_path;
	size_t			start_pos = 0;
	size_t			end_pos = 0;

	while (!(HandlingString::is_ows(line_without_ows[end_pos])) && line_without_ows[end_pos] != '\0')
		end_pos++;
	if (line_without_ows.substr(start_pos, end_pos - start_pos) != "location" || end_pos == line_without_ows.length())
		return (false);
	while (HandlingString::is_ows(line_without_ows[end_pos]))
		end_pos++;
	start_pos = end_pos;
	while (!(HandlingString::is_ows(line_without_ows[end_pos])) && line_without_ows[end_pos] != '\0')
		end_pos++;
	location_path = line_without_ows.substr(start_pos, end_pos - start_pos);
	if (!HandlingString::is_printable_content(location_path) || end_pos == \
	line_without_ows.length())  // location pathに関して本当にprintaleかどうかのチェックだけでいいのか
		return (false);
	while (HandlingString::is_ows(line_without_ows[end_pos]))
		end_pos++;
	start_pos = end_pos;
	while (!(HandlingString::is_ows(line_without_ows[end_pos])) && end_pos != line_without_ows.length())
		end_pos++;
	if (line_without_ows.substr(start_pos, end_pos - start_pos) != "{" || end_pos != line_without_ows.length())
		return (false);
	*config_location_path = location_path;
	return (true);
}

// start_server -> <OWS> server <OWS> { <OWS>
bool	IsConfigFormat::is_start_serverblock(const std::string &line)
{
	std::string		line_without_ows = HandlingString::obtain_without_ows_value(line);
	size_t			start_pos = 0;
	size_t			end_pos = 0;

	if (line_without_ows == "server{")
		return (true);
	while (!(HandlingString::is_ows(line_without_ows[end_pos])) && line_without_ows[end_pos] != '\0')
		end_pos++;
	if (line_without_ows.substr(start_pos, end_pos - start_pos) != "server" || end_pos == line_without_ows.length())
		return (false);
	while (HandlingString::is_ows(line_without_ows[end_pos]))
		end_pos++;
	start_pos = end_pos;
	while (!(HandlingString::is_ows(line_without_ows[end_pos])) && end_pos != line_without_ows.length())
		end_pos++;
	if (line_without_ows.substr(start_pos, end_pos - start_pos) != "{" || end_pos != line_without_ows.length())
		return (false);
	return (true);
}

// location -> <OWS> (文字列->header) <OWS> {文字列->value}; <OWS>
// もしくは終了を表す　}　元のOWSはなくても許容
// このis_型ではチェックのみ行う
bool	IsConfigFormat::is_locationblock_config(const std::string &line, bool *in_location_block)
{
	std::string		line_without_ows = HandlingString::obtain_without_ows_value(line);
	// (文字列->header) <OWS> {文字列->value};
	size_t			start_pos = 0;
	size_t			end_pos = 0;

	if (line_without_ows == "}")
	{
		*in_location_block = false;
		return true;
	}
	while (!(HandlingString::is_ows(line_without_ows[end_pos])) && end_pos != line_without_ows.length())
		end_pos++;
	if (end_pos == line_without_ows.length())  // headerの存在確認を行う必要があればここで行う headerしか存在しない場合のチェックは必要
		return (false);
	while (HandlingString::is_ows(line_without_ows[end_pos]))
		end_pos++;
	if (line_without_ows.substr(end_pos, line_without_ows.length() - end_pos) == ";")  // "  key ; "のようにvalueがない場合はじく
		return (false);
	start_pos = end_pos;
	while (line_without_ows[end_pos] != ';' && line_without_ows[end_pos] != '\0')  // valueの終了条件は必ずセミコロンが存在しているかどうかになる
		end_pos++;
	// if (HandlingString::obtain_without_ows_value(field_value).empty()) //valueを比較する
	// 	return (false);
	if (line_without_ows.length() == end_pos || line_without_ows.length() != end_pos + 1)
		return (false);
	return (true);
}

// location -> <OWS> (文字列->header) <OWS> {文字列->value}; <OWS>
// もしくは終了を表す　}　元のOWSはなくても許容
bool	IsConfigFormat::ready_locationblock_config(const std::string &line, bool *in_location_block, LocationConfig *locationconfig, std::vector<std::string> *fieldkey_map)
{
	std::string		line_without_ows = HandlingString::obtain_without_ows_value(line);
	// (文字列->header) <OWS> {文字列->value};
	size_t			start_pos = 0;
	size_t			end_pos = 0;
	std::string		field_header;
	std::string		field_value;

	if (line_without_ows == "}")
	{
		*in_location_block = false;
		return true;
	}
	while (!(HandlingString::is_ows(line_without_ows[end_pos])) && end_pos != line_without_ows.length())
		end_pos++;
	if (end_pos == line_without_ows.length())  // headerの存在確認を行う必要があればここで行う headerしか存在しない場合のチェックは必要
	{
		std::cout << "first" << std::endl;
		return (false);
	}  // check_header_exist();
	field_header = line_without_ows.substr(start_pos, end_pos - start_pos);
	while (HandlingString::is_ows(line_without_ows[end_pos]))
		end_pos++;
	if (line_without_ows.substr(end_pos, line_without_ows.length() - end_pos) == ";")  // "  key ; "のようにvalueがない場合はじく
		return (false);
	start_pos = end_pos;
	while (line_without_ows[end_pos] != ';' && line_without_ows[end_pos] != '\0')  // valueの終了条件は必ずセミコロンが存在しているかどうかになる
		end_pos++;
	// if (HandlingString::obtain_without_ows_value(field_value).empty()) //valueを比較する valueが空白なら許容しない
	// 	return (false);
	if (line_without_ows.length() == end_pos || line_without_ows.length() != end_pos + 1)
		return (false);
	field_value = line_without_ows.substr(start_pos, end_pos - start_pos);
	if (std::find(fieldkey_map->begin(), fieldkey_map->end(), field_header) != fieldkey_map->end())
		return (false);
	fieldkey_map->push_back(field_header);
	if (locationconfig->ready_locationblock_keyword(field_header, field_value) == false)
		return (false);
	return (true);
}

// server <OWS> (文字列->header) <OWS> {文字列->value} ; <OWS>
// もしくはserver が終了する }
// もしくはlocationブロックのスタート
bool	IsConfigFormat::ready_serverblock_format(const std::string &line, bool *in_server_block,
ServerConfig *serverconfig, std::vector<std::string> *field_key_map)
{
	std::string		line_without_ows = HandlingString::obtain_without_ows_value(line);
	// (文字列->header) <OWS> {文字列->value};
	size_t			start_pos = 0;
	size_t			end_pos = 0;
	std::string		field_header;
	std::string		field_value;

	if (is_start_locationblock(line))
		return (true);
	if (line_without_ows == "}")
	{
		*in_server_block = false;
		return true;
	}
	while (!(HandlingString::is_ows(line_without_ows[end_pos])) && end_pos != line_without_ows.length())
		end_pos++;
	if (end_pos == line_without_ows.length())  // headerの存在確認を行う必要があればここで行う headerしか存在しない場合のチェックは必要
		return (false);
	field_header = line_without_ows.substr(start_pos, end_pos);
	if (std::find(field_key_map->begin(), field_key_map->end(), field_header) != field_key_map->end())
		return false;
	while (HandlingString::is_ows(line_without_ows[end_pos]))
		end_pos++;
	if (line_without_ows.substr(end_pos, line_without_ows.length() - end_pos) == ";")  // "  key ; "のようにvalueがない場合はじく
		return (false);
	start_pos = end_pos;
	while (line_without_ows[end_pos] != ';' && line_without_ows[end_pos] != '\0')  // valueの終了条件は必ずセミコロンが存在しているかどうかになる
		end_pos++;
	if (line_without_ows[end_pos] == '\0')
		return (false);
	field_value = line_without_ows.substr(start_pos, end_pos - start_pos);
	if (HandlingString::obtain_without_ows_value(field_value).empty())  // valueを比較する valueが空白なら許容しない
		return (false);
	if (line_without_ows.length() == end_pos || line_without_ows.length() != end_pos + 1)
		return (false);
	if (serverconfig->ready_serverblock_keyword(field_header, field_value) == false)
	{
		std::cout << "serverconfig -> |" << field_header << "|" << field_value << "|" << std::endl;
		return (false);
	}
	field_key_map->push_back(field_header);
	return (true);
}
