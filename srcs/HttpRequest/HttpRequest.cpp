#include "HttpRequest.hpp"

HttpRequest::HttpRequest(const std::string &all_request_text):_status_code(200)
{
	std::string	line;
	std::string	remove_sohword_line;
	std::string	key;
	std::string	value;

	ready_functionmap();
	std::stringstream ss(all_request_text);
	std::getline(ss, line, '\n');
	if (this->is_requestlineformat(line) == false)
	{
		this->_status_code = 400;
		return;
	}
	this->_request_line.set_value(line);
	// std::cout << "request_line end" << std::endl;
	while (std::getline(ss, line, '\n'))
	{
		// 文末にCREFがあるかどうかを確認する
		remove_sohword_line = line.substr(0, line.length() - 1);
		if (is_requestformat(line) == false)
		{
			this->_status_code = 400;
			return;
		}
		if (HandlingString::is_printable_content(remove_sohword_line) == false)
		{
			this->_status_code = 400;
			return;
		}
		key = this->obtain_request_key(line);
		value = this->obtain_request_value(line);
		value = value.substr(0, value.length() - 1);
		if (this->is_keyword_exist(key) == true)
			(this->*_field_name_parser[key])(key, value);
	}
}

HttpRequest::~HttpRequest()
{
	std::map<std::string, BaseKeyValueMap*>::iterator inputed_class_itr = this->_request_keyvalue_map.begin();
	while (inputed_class_itr != this->_request_keyvalue_map.end())
	{
		delete (inputed_class_itr->second);
		inputed_class_itr++;
	}
}

bool HttpRequest::is_requestlineformat(std::string input_requestline)
{
	int			i = 0;
	size_t		pos = 0;

	while (i != 3)
	{
		if (input_requestline[pos] == ' ')
		{
			std::cout << "1" << std::endl;
			return (false);
		}
		if (i == 2)
		{
			if (HandlingString::is_end_with_cr(input_requestline.substr(pos)) == false)
				return (false);
			if (input_requestline.substr(pos, input_requestline.length() - pos - 1).find(' ') != std::string::npos)
				return (false);
		}
		while (input_requestline[pos] != ' ' && pos != input_requestline.length() - 2)
		{
			if (isprint(input_requestline[pos]) == false)
				return false;
			pos++;
		}
		if (i != 2)
			pos++;
		i++;
	}
	return (true);
}

LinkClass* HttpRequest::ready_LinkClass(std::map<std::string, std::map<std::string, std::string> > link_valuemap)
{
	return (new LinkClass(link_valuemap));
}

SecurityPolicy* HttpRequest::ready_SecurityPolicy(const std::string &report_url, std::map<std::string, std::vector<std::string> >	_policy_directive)
{
	return (new SecurityPolicy(report_url, _policy_directive));
}

SecurityPolicy* HttpRequest::ready_SecurityPolicy(std::map<std::string, std::vector<std::string> >	_policy_directive)
{
	return (new SecurityPolicy(_policy_directive));
}

TwoValueSet* HttpRequest::ready_TwoValueSet(const std::string &all_value)
{
	std::stringstream	ss(HandlingString::obtain_withoutows_value(all_value));
	std::string			first_value;
	std::string			second_value;

	std::getline(ss, first_value, '/');
	std::getline(ss, second_value, '/');

	return (new TwoValueSet(first_value, second_value));
}

TwoValueSet* HttpRequest::ready_TwoValueSet(const std::string &value, char delimiter)
{
	std::stringstream	ss(HandlingString::obtain_withoutows_value(value));
	std::string			first_value;
	std::string			second_value;

	std::getline(ss, first_value, delimiter);
	std::getline(ss, second_value, delimiter);
	return (new TwoValueSet(HandlingString::obtain_withoutows_value(first_value), HandlingString::obtain_withoutows_value(second_value)));
}

ValueArraySet* HttpRequest::ready_ValueArraySet(const std::string &all_value)
{
	std::vector<std::string>	value_array;

	std::stringstream	ss(all_value);
	std::string			line;
	while(std::getline(ss, line, ','))
		value_array.push_back(HandlingString::obtain_withoutows_value(line));
	return (new ValueArraySet(value_array));
}

ValueDateSet* HttpRequest::ready_ValueDateSet(const std::string &value)
{
	return (new ValueDateSet(HandlingString::obtain_withoutows_value(value)));
}

ValueMap* HttpRequest::ready_ValueMap(const std::string &value, char delimiter)
{
	std::map<std::string, std::string> value_map;
	std::stringstream	ss(value);
	std::string			line;

	while(std::getline(ss, line, delimiter))
		value_map[HandlingString::obtain_word_before_delimiter(HandlingString::obtain_withoutows_value(line), '=')] \
		= HandlingString::obtain_word_after_delimiter(HandlingString::obtain_withoutows_value(line), '=');
	return (new ValueMap(value_map));
}

ValueMap* HttpRequest::ready_ValueMap(const std::string &value)
{
	std::map<std::string, std::string> value_map;
	std::stringstream	ss(value);
	std::string			line;

	while(std::getline(ss, line, ';'))
		value_map[HandlingString::obtain_word_before_delimiter(HandlingString::obtain_withoutows_value(line), '=')] \
		= HandlingString::obtain_word_after_delimiter(HandlingString::obtain_withoutows_value(line), '=');
	return (new ValueMap(value_map));
}

ValueMap* HttpRequest::ready_ValueMap(const std::string &only_value, const std::string &value)
{
	std::map<std::string, std::string> value_map;
	std::stringstream	ss(value);
	std::string			line;
	std::string			skipping_word;

	while(std::getline(ss, line, ';'))
	{
		skipping_word = HandlingString::obtain_withoutows_value(line);
		value_map[HandlingString::obtain_word_before_delimiter(skipping_word, '=')] \
		= HandlingString::obtain_withoutows_value(HandlingString::obtain_word_after_delimiter(skipping_word, '='));
	}
	return (new ValueMap(only_value, value_map));
}

ValueSet* HttpRequest::ready_ValueSet(const std::string &value)
{
	return (new ValueSet(HandlingString::obtain_withoutows_value(value)));
}

ValueWeightArraySet*	HttpRequest::ready_ValueWeightArraySet(const std::string &value)
{
	std::map<std::string, double>	value_map;
	std::stringstream				splited_by_commma(value);
	std::string						line;
	std::string						changed_line;
	std::string						target_value;

	while(std::getline(splited_by_commma, line, ','))
	{
		changed_line = HandlingString::obtain_withoutows_value(line);
		if (changed_line.find(';') != std::string::npos)
		{
			target_value = HandlingString::obtain_weight(HandlingString::obtain_word_after_delimiter(changed_line, ';'));
			value_map[HandlingString::obtain_word_before_delimiter(changed_line, ';')] = \
			HandlingString::str_to_double(HandlingString::obtain_weight(HandlingString::obtain_word_after_delimiter(changed_line, ';')));
		}
		else
			value_map[changed_line] = 1.0;
	}
	return (new ValueWeightArraySet(value_map));
}

bool	HttpRequest::is_keyword_exist(const std::string &key)
{
	// const std::string httprequest_keyset_arr[] = {
	// 	"Host",
	// 	"Connection",
	// 	"Referer", "Content-Type", "Range", "Upgrade", "Accept-Encoding", "Via", "Keep-Alive", "Accept-Language", "Accept", "Date",
	// 	"Cookie", "If-Modified-Since", "If-Unmodified-Since", "If-Match", "If-None-Match", "Content-Length", "Content-Range", "If-Range",
	// 	"Transfer-Encoding", "Expect", "Authorization", "User-Agent", "Accept-CH", "Accept-Charset", "Accept-Patch", "Accept-Ranges",
	// 	"Access-Control-Allow-Credentials", "Access-Control-Allow-Headers", "Access-Control-Allow-Methods", "Access-Control-Allow-Origin",
	// 	"Access-Control-Expose-Headers", "Access-Control-Max-Age", "Access-Control-Request-Headers", "Access-Control-Request-Method",
	// 	"Age", "Allow", "Alt-Svc", "Cache-Control", "Clear-Site-Data", "Content-Disposition", "Content-Encoding", "Content-Language",
	// 	"Content-Location", "Content-Security-Policy", "Content-Security-Policy-Report-Only", "Cross-Origin-Embedder-Policy",
	// 	"Cross-Origin-Opener-Policy", "Cross-Origin-Resource-Policy", "ETag", "Expect-CT", "Expires", "Forwarded", "From",
	// 	"Last-Modified", "Location", "Origin", "Permissions-Policy", "Proxy-Authenticate", "Proxy-Authorization", "Referrer-Policy",
	// 	"Retry-After", "Server", "Server-Timing", "Set-Cookie", "SourceMap", "Timing-Allow-Origin", "Authorization",
	// 	"Upgrade-Insecure-Requests", "Vary", "WWW-Authenticate", "Max-Forwards", "TE", "Accept-Post", "X-Custom-Header", "Sec-Fetch-Dest",
	// 	"Sec-Fetch-Mode", "Sec-Fetch-Site", "Sec-Fetch-User", "Sec-Purpose", "Sec-WebSocket-Accept", "Service-Worker-Navigation-Preload",
	// 	"Trailer", "Link"
	// };
	// const std::set<std::string> httprequest_keyset
	// (
	// 	httprequest_keyset_arr,
	// 	httprequest_keyset_arr + sizeof(httprequest_keyset_arr) / sizeof(httprequest_keyset_arr[0])
	// );
	if (key == "Access-Control-Expose-Headers")
		std::cout << this->_field_name_parser.count(key) << std::endl;
	if (this->_field_name_parser.count(key) > 0)
		return true;
	return false;
}

std::string	HttpRequest::obtain_request_key(const std::string value)
{
	std::stringstream	ss(value);
	std::string			line;

	std::getline(ss, line, ':');
	return (line);
}

bool	HttpRequest::is_requestformat(const std::string &val)
{
	std::string::size_type pos = val.find_first_of(":");
	if (pos == 0 || pos == std::string::npos)
		return (false);
	if (HandlingString::is_ows(val[pos - 1]))
		return (false);
	return (true);
}

std::string	HttpRequest::obtain_request_value(const std::string value)
{
	std::string::size_type pos = value.find_first_of(":");
	std::string part2 = value.substr(pos + 2);
	return (part2);
}

void HttpRequest::set_accept(const std::string &key, const std::string &value)
{
	size_t	value_length = value.size();
	size_t	now_location = 0;

	char accept_encoding_keyset[] = {
		'\x00', '\x01', '\x02', '\x03', '\x04', '\x05', '\x06', '\x07', '\x08', '\x09', '\x10', \
		'\x0A', '\x0B', '\x0C', '\x0D', '\x0E', '\x0F', '\x10', '\x11', '\x12', '\x13', '\x14', \
		'\x15', '\x16', '\x17', '\x18', '\x19', '\x1A', '\x1B', '\x1C', '\x1D', '\x1E', '\x1F', \
		'(', ')', ':', '<', '>', '?', '@', '[', '\\', ']', '{', '}'
	};
	const std::set<char> accept_keyset
	(
		accept_encoding_keyset,
		accept_encoding_keyset + sizeof(accept_encoding_keyset) / sizeof(accept_encoding_keyset[0])
	);
	while (now_location != value_length - 1)
	{
		if (accept_keyset.count(value[now_location]) > 0)
			return;
		now_location++;
	}
	std::stringstream				splited_by_commma(value);
	std::string						line;
	std::string						changed_line;
	while(std::getline(splited_by_commma, line, ','))
	{
		if (line[0] == ' ')
			changed_line = line.substr(1);
		else
			changed_line = line;
		if (changed_line.find(';') != std::string::npos)
		{
			if (HandlingString::is_double_or_not(HandlingString::obtain_weight(HandlingString::obtain_word_after_delimiter(changed_line, ';'))) == false)
				return;
		}
	}
	this->_request_keyvalue_map[key] = ready_ValueWeightArraySet(value);
}

void	HttpRequest::set_accept_ch(const std::string &key, const std::string &value)
{
	this->_request_keyvalue_map[key] = ready_ValueArraySet(value);
}

void	HttpRequest::set_accept_charset(const std::string &key, const std::string &value)
{
	std::stringstream				splited_by_commma(value);
	std::string						line;
	std::string						changed_line;
	while(std::getline(splited_by_commma, line, ','))
	{
		if (line[0] == ' ')
			changed_line = line.substr(1);
		else
			changed_line = line;
		if (changed_line.find(';') != std::string::npos)
		{
			if (HandlingString::is_double_or_not(HandlingString::obtain_weight(HandlingString::obtain_word_after_delimiter(changed_line, ';'))) == false)
				return;
		}
	}
	this->_request_keyvalue_map[key] = ready_ValueWeightArraySet(value);
}

void	HttpRequest::set_accept_encoding(const std::string &key, const std::string &value)
{
	std::stringstream splited_by_commma(value);
	std::string	skipping_nokeyword;
	std::string	keyword;
	std::string	peir_value;
	std::string	line;
	std::string	last_line;
	// Accept-Encoding: gzip, deflate

	const std::string accept_encoding_keyset[] = {
		"gzip", "compress", "deflate", "br", "*", "identity"
	};
	const std::set<std::string> httprequest_keyset
	(
		accept_encoding_keyset,
		accept_encoding_keyset + sizeof(accept_encoding_keyset) / sizeof(accept_encoding_keyset[0])
	);
	while(std::getline(splited_by_commma, line, ','))
	{
		if (line.find(';') != std::string::npos)
		{
			if (HandlingString::is_double_or_not(HandlingString::obtain_weight(HandlingString::obtain_word_after_delimiter(line, ';'))) == false)
				return;
			keyword = HandlingString::obtain_word_before_delimiter(line, ';');
		}
		else
			keyword = line;
		if (keyword[0] == ' ')
		{
			keyword = keyword.substr(1);
			if (httprequest_keyset.count(keyword) > 0)
				skipping_nokeyword = skipping_nokeyword + line + ',';
		}
		else
		{
			if (httprequest_keyset.count(keyword) > 0)
				skipping_nokeyword = skipping_nokeyword + line + ',';
		}
	}
	last_line = skipping_nokeyword.substr(0, skipping_nokeyword.length() - 1);

	this->_request_keyvalue_map[key] = ready_ValueWeightArraySet(last_line);
}

void	HttpRequest::set_accept_language(const std::string &key, const std::string &value)
{
	std::stringstream splited_by_commma(value);
	std::string	skipping_nokeyword;
	std::string	accept_language_value;
	std::string	line;
	std::string	target_value;

	while(std::getline(splited_by_commma, line, ','))
	{
		if (line.find(';') != std::string::npos)
		{
			accept_language_value = HandlingString::obtain_word_before_delimiter(line, ';');
			target_value = HandlingString::obtain_weight(HandlingString::obtain_word_after_delimiter(line, ';'));
			if (is_accept_langage_valueword(accept_language_value) == true)
				skipping_nokeyword = skipping_nokeyword + line;
			if (target_value == "0" || target_value == "0.0")
				return;
			if (HandlingString::is_double_or_not(target_value) == false)
				return;
		}
		else
		{
			if (is_accept_langage_valueword(line) == true)
				skipping_nokeyword = skipping_nokeyword + line;
		}
	}
	this->_request_keyvalue_map[key] = ready_ValueWeightArraySet(value);
}

bool	HttpRequest::is_accept_langage_valueword(const std::string &value)
{
	size_t	string_length = value.length();
	size_t	now_location = 0;

	while (string_length != now_location)
	{
		if (!('0' <= value[now_location] && value[now_location] <= '9'))
		{
			if (!('A' <= value[now_location] && value[now_location] <= 'Z'))
			{
				if (!('a' <= value[now_location] && value[now_location] <= 'z'))
				{
					if (!(' ' == value[now_location] || '*' == value[now_location] || '-' == value[now_location] \
					|| ',' == value[now_location] || '.' == value[now_location] || ';' == value[now_location] \
					|| '=' == value[now_location]))
					{
						this->_status_code = 406;
						return (false);
					}
				}
			}
		}
		now_location++;
	}
	return (true);
}

// Accept-Patchどういう持ち方かわからん

void	HttpRequest::set_accept_post(const std::string &key, const std::string &value)
{
	this->_request_keyvalue_map[key] = ready_TwoValueSet(value, ',');
}

void	HttpRequest::set_accept_ranges(const std::string &key, const std::string &value)
{
	if (value == "bytes")
		this->_request_keyvalue_map[key] = ready_ValueSet(value);
	else
		return;
}

void	HttpRequest::set_access_control_allow_credentials(const std::string &key, const std::string &value)
{
	if (value != "true")
		return;
	this->_request_keyvalue_map[key] = ready_ValueSet(value);
}

void	HttpRequest::set_access_control_allow_headers(const std::string &key, const std::string &value)
{
	std::vector<std::string>	value_array;
	std::stringstream	ss(value);
	std::string			line;
	while(std::getline(ss, line, ','))
	{
		if (this->is_keyword_exist(HandlingString::obtain_withoutows_value(line)) == false)
			return;
	}
	this->_request_keyvalue_map[key] = ready_ValueArraySet(value);
}

void	HttpRequest::set_access_control_allow_methods(const std::string &key, const std::string &value)
{
	std::stringstream	ss(value);
	std::string			line;
	std::string			word;
	while(std::getline(ss, line, ','))
	{
		if (line[0] == ' ')
			word = line.substr(1);
		else
			word = line;
		if (word != "GET" && word != "HEAD" && word != "POST" && word != "PUT" && word != "PUT" && word != "DELETE" \
		&& word != "CONNECT" && word != "OPTIONS" && word != "TRACE" && word != "PATCH")
			return;
	}
	this->_request_keyvalue_map[key] = ready_ValueArraySet(value);
}

void	HttpRequest::set_access_control_allow_origin(const std::string &key, const std::string &value)
{
	this->_request_keyvalue_map[key] = ready_ValueSet(value);
}

void	HttpRequest::set_access_control_expose_headers(const std::string &key, const std::string &value)
{
	std::vector<std::string>	value_array;
	std::stringstream	ss(value);
	std::string			line;
	while(std::getline(ss, line, ','))
	{
		if (this->is_keyword_exist(HandlingString::obtain_withoutows_value(line)) == false)
			return;
	}
	this->_request_keyvalue_map[key] = ready_ValueArraySet(value);
}

void	HttpRequest::set_access_control_max_age(const std::string &key, const std::string &value)
{
	if (HandlingString::is_positive_and_under_intmax(value) == false)
		return;
	this->_request_keyvalue_map[key] = ready_ValueSet(value);
}

void	HttpRequest::set_access_control_request_headers(const std::string &key, const std::string &value)
{
	std::vector<std::string>	value_array;
	std::stringstream	ss(value);
	std::string			line;
	std::string			word;

	while(std::getline(ss, line, ','))
	{
		if (line[0] == ' ')
			word = line.substr(1);
		else
			word = line;
		if (this->is_keyword_exist(HandlingString::obtain_withoutows_value(word)) == false)
			return;
	}
	this->_request_keyvalue_map[key] = ready_ValueArraySet(value);
}

void	HttpRequest::set_access_control_request_method(const std::string &key, const std::string &value)
{
	if (value != "GET" && value != "HEAD" && value != "POST" && value != "PUT" && value != "PUT" && value != "DELETE" \
		&& value != "CONNECT" && value != "OPTIONS" && value!= "TRACE" && value != "PATCH")
		return;
	this->_request_keyvalue_map[key] = ready_ValueSet(value);
}

void	HttpRequest::set_age(const std::string &key, const std::string &value)
{
	if (HandlingString::is_positive_and_under_intmax(value) == false)
		return;
	if (HandlingString::is_positive_and_under_intmax(value) == false)
		return;
	this->_request_keyvalue_map[key] = ready_ValueSet(value);
}

void	HttpRequest::set_allow(const std::string &key, const std::string &value)
{
	std::stringstream	ss(value);
	std::string			line;
	std::string			word;
	while(std::getline(ss, line, ','))
	{
		if (line[0] == ' ')
			word = line.substr(1);
		else
			word = line;
		if (word != "GET" && word != "HEAD" && word != "POST" && word != "PUT" && word != "PUT" && word != "DELETE" \
		&& word != "CONNECT" && word != "OPTIONS" && word != "TRACE" && word != "PATCH")
			return;
	}
	this->_request_keyvalue_map[key] = ready_ValueArraySet(value);
}

void	HttpRequest::set_alt_svc(const std::string &key, const std::string &value)
{
	this->_request_keyvalue_map[key] = ready_ValueMap(value);
}

void	HttpRequest::set_alt_used(const std::string &key, const std::string &value)
{
	this->_request_keyvalue_map[key] = ready_ValueSet(value);
}

void	HttpRequest::set_authorization(const std::string &key, const std::string &value)
{
	// Digest username=<username>,realm="<realm>",uri="<url>",algorithm=<algorithm>,nonce="<nonce>",
	// ValueMapに変更
	this->_request_keyvalue_map[key] = ready_TwoValueSet(value, ' ');
}

// Cache-Controlどう使うのか全くわからない
void	HttpRequest::set_cache_control(const std::string &key, const std::string &value)
{
	(void)key;
	(void)value;
	// Digest username=<username>,realm="<realm>",uri="<url>",algorithm=<algorithm>,nonce="<nonce>",
	// ValueMapに変更
	// this->_request_keyvalue_map[key] = ready_ValueWeightArraySet(value);
}

void	HttpRequest::set_clear_site_data(const std::string &key, const std::string &value)
{
	// ダブルクオーテーションで囲う必要性があるようだが、"aaaa"", "bbb"みたいなことをされたとする
	// この場合にフォーマットが正しくないみたいなステータスコード を投げる？
	this->_request_keyvalue_map[key] = ready_ValueArraySet(value);
}

void	HttpRequest::set_connection(const std::string &key, const std::string &value)
{
	if (value == "close" || value == "keep-alive")
		this->_request_keyvalue_map[key] = ready_ValueSet(value);
	else
		return;
}

void	HttpRequest::set_content_disponesition(const std::string &key, const std::string &value)
{
	std::stringstream	ss(value);
	std::string			only_value;
	std::string			except_onlyvalue_line;
	std::string 		line;
	std::getline(ss, only_value, ';');
	while (std::getline(ss, line, ';'))
		except_onlyvalue_line = except_onlyvalue_line + line;
	this->_request_keyvalue_map[key] = ready_ValueMap(only_value, except_onlyvalue_line);
}

void	HttpRequest::set_content_encoding(const std::string &key, const std::string &value)
{
	std::stringstream	ss(value);
	std::string			line;
	while(std::getline(ss, line, ','))
	{
		if (line != "gzip" && line != "compress" && line != "deflate" && line != "br")
			return;
	}
	this->_request_keyvalue_map[key] = ready_ValueArraySet(value);
}

void	HttpRequest::set_content_language(const std::string &key, const std::string &value)
{
	// valueの値がある程度限定されていそうだが、まとまっていそうではなく特定が難しい
	this->_request_keyvalue_map[key] = ready_ValueArraySet(value);
}

void	HttpRequest::set_content_length(const std::string &key, const std::string &value)
{
	if (HandlingString::is_positive_and_under_intmax(value) == false)
		return;
	if (HandlingString::is_positive_and_under_intmax(value) == false)
		return;
	this->_request_keyvalue_map[key] = ready_ValueSet(value);
}

void	HttpRequest::set_content_location(const std::string &key, const std::string &value)
{
	this->_request_keyvalue_map[key] = ready_ValueSet(value);
}

void	HttpRequest::set_content_range(const std::string &key, const std::string &value)
{
	// 確かにvaluesetでも良さそうだが、もう少しあっているものもありそう
	this->_request_keyvalue_map[key] = ready_ValueSet(value);
}

std::vector<std::string>	HttpRequest::securitypolicy_readyvector(const std::string &words)
{
	std::vector<std::string>	tmp_vector;
	std::string	skippingfirstemptyword;
	if (words[0] == ' ')
		skippingfirstemptyword = words.substr(1);
	else
		skippingfirstemptyword = words;
	std::stringstream	ss(skippingfirstemptyword);
	std::string			line;
	while(std::getline(ss, line, ' '))
		tmp_vector.push_back(line);
	return (tmp_vector);
}

void	HttpRequest::set_content_security_policy(const std::string &key, const std::string &value)
{
	std::map<std::string, std::vector<std::string> > content_security_map;
	std::stringstream	ss(value);
	std::string			line;
	std::string			skip_emptyword;
	std::string			policy_directive;
	std::string			words;
	size_t				empty_position;
	while(std::getline(ss, line, ';'))
	{
		if (line[0] == ' ')
			skip_emptyword = line.substr(1);
		else
			skip_emptyword = line;
		empty_position = skip_emptyword.find(' ');
		policy_directive = skip_emptyword.substr(0, empty_position);
		words = skip_emptyword.substr(empty_position + 1);
		content_security_map[policy_directive] = securitypolicy_readyvector(words);
	}
	this->_request_keyvalue_map[key] = ready_SecurityPolicy(content_security_map);
}

void	HttpRequest::set_content_security_policy_report_only(const std::string &key, const std::string &value)
{
	(void)key;
	(void)value;
	// std::cout << key << " and " << "value is " << value << std::endl;
	// this->_request_keyvalue_map[key] = ready_SecurityPolicy();
}

void	HttpRequest::set_content_type(const std::string &key, const std::string &value)
{
	std::stringstream	ss(value);
	std::string			only_value;
	std::string			except_onlyvalue_line;
	std::string 		line;
	std::getline(ss, only_value, ',');
	while (std::getline(ss, line, ','))
		except_onlyvalue_line = except_onlyvalue_line + line;
	this->_request_keyvalue_map[key] = ready_ValueMap(only_value, except_onlyvalue_line);
}

void	HttpRequest::set_cookie(const std::string &key, const std::string &value)
{
	this->_request_keyvalue_map[key] = ready_ValueMap(value);
}

void	HttpRequest::set_cross_origin_embedder_policy(const std::string &key, const std::string &value)
{
	this->_request_keyvalue_map[key] = ready_ValueSet(value);
}

void	HttpRequest::set_cross_origin_opener_policy(const std::string &key, const std::string &value)
{
	this->_request_keyvalue_map[key] = ready_ValueSet(value);
}

void	HttpRequest::set_cross_origin_resource_policy(const std::string &key, const std::string &value)
{
	if (value == "same-site" || value == "same-origin" || value == "cross-origin")
		this->_request_keyvalue_map[key] = ready_ValueSet(value);
	else
		return;
}

void	HttpRequest::set_date(const std::string &key, const std::string &value)
{
	std::stringstream	ss(value);
	std::string			line;
	std::string			day_name;
	std::string			day;
	std::string			month;
	std::string			year;
	std::string			hour;
	std::string			minute;
	std::string			second;

	std::getline(ss, day_name, ',');
	std::getline(ss, line, ',');
	if (day_name != "Mon" && day_name != "Tue" && day_name != "Wed" && day_name != "Thu" && day_name != "Fri" && day_name != "Sat" && day_name != "Sun")
		return;
	std::string after_line = line.substr(1);
	std::stringstream	sss(after_line);
	std::getline(sss, day, ' ');
	if (day.length() != 2)
		return;
	if (!(1 <= HandlingString::str_to_int(day) || HandlingString::str_to_int(day) <= 31))
		return;
	std::getline(sss, month, ' ');
	std::getline(sss, year, ' ');
	std::string			hour_minute_second;
	std::getline(sss, hour_minute_second, ' ');
	std::stringstream	ssss(hour_minute_second);
	std::getline(ssss, hour, ':');
	if (!(0 <= HandlingString::str_to_int(hour) || HandlingString::str_to_int(hour) <= 60))
		return;
	std::getline(ssss, minute, ':');
	if (!(0 <= HandlingString::str_to_int(minute) || HandlingString::str_to_int(minute) <= 60))
		return;
	std::getline(ssss, second, ':');
	if (!(0 <= HandlingString::str_to_int(second) || HandlingString::str_to_int(second) <= 60))
		return;
	this->_request_keyvalue_map[key] = ready_ValueDateSet(value);
}

void	HttpRequest::set_etag(const std::string &key, const std::string &value)
{
	this->_request_keyvalue_map[key] = ready_ValueSet(value);
}

void	HttpRequest::set_expect(const std::string &key, const std::string &value)
{
	if (value == "100-continue")
		this->_request_keyvalue_map[key] = ready_ValueSet(value);
	else
		return;
}

// expect-ctは現状廃止されているっぽくて対応したくない

void	HttpRequest::set_expires(const std::string &key, const std::string &value)
{
	this->_request_keyvalue_map[key] = ready_ValueSet(value);
}

void	HttpRequest::set_forwarded(const std::string &key, const std::string &value)
{
	this->_request_keyvalue_map[key] = ready_ValueMap(value);
}

void	HttpRequest::set_from(const std::string &key, const std::string &value)
{
	this->_request_keyvalue_map[key] = ready_ValueSet(value);
}

void	HttpRequest::set_host(const std::string &key, const std::string &value)
{
	this->_request_keyvalue_map[key] = ready_TwoValueSet(value);
	// ちょっと変更する必要性あり
}

void	HttpRequest::set_if_match(const std::string &key, const std::string &value)
{
	this->_request_keyvalue_map[key] = ready_ValueArraySet(value);
}

void	HttpRequest::set_if_modified_since(const std::string &key, const std::string &value)
{
	std::stringstream	ss(value);
	std::string			line;
	std::string			day_name;
	std::string			day;
	std::string			month;
	std::string			year;
	std::string			hour;
	std::string			minute;
	std::string			second;

	std::getline(ss, day_name, ',');
	std::getline(ss, line, ',');
	if (day_name != "Mon" && day_name != "Tue" && day_name != "Wed" && day_name != "Thu" && day_name != "Fri" && day_name != "Sat" && day_name != "Sun")
		return;
	std::string after_line = line.substr(1);
	std::stringstream	sss(after_line);
	std::getline(sss, day, ' ');
	if (day.length() != 2)
		return;
	if (!(1 <= HandlingString::str_to_int(day) || HandlingString::str_to_int(day) <= 31))
		return;
	std::getline(sss, month, ' ');
	std::getline(sss, year, ' ');
	std::string			hour_minute_second;
	std::getline(sss, hour_minute_second, ' ');
	std::stringstream	ssss(hour_minute_second);
	std::getline(ssss, hour, ':');
	if (!(0 <= HandlingString::str_to_int(hour) || HandlingString::str_to_int(hour) <= 60))
		return;
	std::getline(ssss, minute, ':');
	if (!(0 <= HandlingString::str_to_int(minute) || HandlingString::str_to_int(minute) <= 60))
		return;
	std::getline(ssss, second, ':');
	if (!(0 <= HandlingString::str_to_int(second) || HandlingString::str_to_int(second) <= 60))
		return;
	this->_request_keyvalue_map[key] = ready_ValueDateSet(value);
}

void	HttpRequest::set_if_none_match(const std::string &key, const std::string &value)
{
	this->_request_keyvalue_map[key] = ready_ValueArraySet(value);
}

void	HttpRequest::set_if_range(const std::string &key, const std::string &value)
{
	this->_request_keyvalue_map[key] = ready_ValueSet(value);
}

void	HttpRequest::set_if_unmodified_since(const std::string &key, const std::string &value)
{
	std::stringstream	ss(value);
	std::string			line;
	std::string			day_name;
	std::string			day;
	std::string			month;
	std::string			year;
	std::string			hour;
	std::string			minute;
	std::string			second;

	std::getline(ss, day_name, ',');
	std::getline(ss, line, ',');
	if (day_name != "Mon" && day_name != "Tue" && day_name != "Wed" && day_name != "Thu" && day_name != "Fri" && day_name != "Sat" && day_name != "Sun")
		return;
	std::string after_line = line.substr(1);
	std::stringstream	sss(after_line);
	std::getline(sss, day, ' ');
	if (day.length() != 2)
		return;
	if (!(1 <= HandlingString::str_to_int(day) || HandlingString::str_to_int(day) <= 31))
		return;
	std::getline(sss, month, ' ');
	std::getline(sss, year, ' ');
	std::string			hour_minute_second;
	std::getline(sss, hour_minute_second, ' ');
	std::stringstream	ssss(hour_minute_second);
	std::getline(ssss, hour, ':');
	if (!(0 <= HandlingString::str_to_int(hour) || HandlingString::str_to_int(hour) <= 60))
		return;
	std::getline(ssss, minute, ':');
	if (!(0 <= HandlingString::str_to_int(minute) || HandlingString::str_to_int(minute) <= 60))
		return;
	std::getline(ssss, second, ':');
	if (!(0 <= HandlingString::str_to_int(second) || HandlingString::str_to_int(second) <= 60))
		return;
	this->_request_keyvalue_map[key] = ready_ValueDateSet(value);
}

void	HttpRequest::set_keep_alive(const std::string &key, const std::string &value)
{
	// ,が区切り文字なのでちょっとValueMapではうまくいかん
	this->_request_keyvalue_map[key] = ready_ValueMap(value, ',');
}

void	HttpRequest::set_last_modified(const std::string &key, const std::string &value)
{
	std::stringstream	ss(value);
	std::string			line;
	std::string			day_name;
	std::string			day;
	std::string			month;
	std::string			year;
	std::string			hour;
	std::string			minute;
	std::string			second;

	std::getline(ss, day_name, ',');
	std::getline(ss, line, ',');
	if (day_name != "Mon" && day_name != "Tue" && day_name != "Wed" && day_name != "Thu" && day_name != "Fri" && day_name != "Sat" && day_name != "Sun")
		return;
	std::string after_line = line.substr(1);
	std::stringstream	sss(after_line);
	std::getline(sss, day, ' ');
	if (day.length() != 2)
		return;
	if (!(1 <= HandlingString::str_to_int(day) || HandlingString::str_to_int(day) <= 31))
		return;
	std::getline(sss, month, ' ');
	std::getline(sss, year, ' ');
	std::string			hour_minute_second;
	std::getline(sss, hour_minute_second, ' ');
	std::stringstream	ssss(hour_minute_second);
	std::getline(ssss, hour, ':');
	if (!(0 <= HandlingString::str_to_int(hour) || HandlingString::str_to_int(hour) <= 60))
		return;
	std::getline(ssss, minute, ':');
	if (!(0 <= HandlingString::str_to_int(minute) || HandlingString::str_to_int(minute) <= 60))
		return;
	std::getline(ssss, second, ':');
	if (!(0 <= HandlingString::str_to_int(second) || HandlingString::str_to_int(second) <= 60))
		return;
	this->_request_keyvalue_map[key] = ready_ValueDateSet(value);
}

std::map<std::string, std::string>	HttpRequest::ready_mappingvalue(const std::string &value_map_words)
{
	std::stringstream				ss(value_map_words);
	std::string						line;
	std::map<std::string, std::string>	words_mapping;
	std::string	key;
	std::string	val;
	std::string	skipping_first_empty;
	while(std::getline(ss, line, ';'))
	{
		skipping_first_empty = line.substr(1);
		key = HandlingString::obtain_word_before_delimiter(skipping_first_empty, '=');
		val = HandlingString::obtain_word_after_delimiter(skipping_first_empty, '=');
		words_mapping[key] = val;
	}
	return (words_mapping);
}

// Link: <https://example.com/style.css>; rel=preload; as=style\r\n
void	HttpRequest::set_link(const std::string &key, const std::string &value)
{
	// a a=a, b b=b, c c=c
	std::map<std::string, std::map<std::string, std::string> > value_map;
	std::stringstream	ss(value);
	std::string			line;
	std::string			uri;
	std::string			mapping_value;

	while(std::getline(ss, line, ','))
	{
		uri = HandlingString::obtain_word_before_delimiter(line, ';');
		mapping_value = HandlingString::obtain_word_after_delimiter(line, ';');
		value_map[uri] = this->ready_mappingvalue(mapping_value);
	}
	this->_request_keyvalue_map[key] = ready_LinkClass(value_map);
}

void	HttpRequest::set_location(const std::string &key, const std::string &value)
{
	this->_request_keyvalue_map[key] = ready_ValueSet(value);
}

void	HttpRequest::set_max_forwards(const std::string &key, const std::string &value)
{
	if (HandlingString::is_positive_and_under_intmax(value))
		this->_request_keyvalue_map[key] = ready_ValueSet(value);
	else
		return;
}

void	HttpRequest::set_origin(const std::string &key, const std::string &value)
{
	this->_request_keyvalue_map[key] = ready_ValueSet(value);
}

void	HttpRequest::set_permission_policy(const std::string &key, const std::string &value)
{
	this->_request_keyvalue_map[key] = ready_TwoValueSet(value, ',');
}

void	HttpRequest::set_proxy_authenticate(const std::string &key, const std::string &value)
{
	// Basic realm="Proxy Server"
	std::string re_line = value.substr(1);
	size_t	empty_position = re_line.find(' ');
	std::string	before_space_word = re_line.substr(0, empty_position);
	std::string	after_space_word = re_line.substr(empty_position + 1, re_line.length());

	this->_request_keyvalue_map[key] = ready_ValueMap(before_space_word, after_space_word);
}

void	HttpRequest::set_proxy_authorization(const std::string &key, const std::string &value)
{
	// 空白が分割文字だからそのまま使うとまずい
	this->_request_keyvalue_map[key] = ready_TwoValueSet(value, ' ');
}

// range何かよくわからん

void	HttpRequest::set_referer(const std::string &key, const std::string &value)
{
	this->_request_keyvalue_map[key] = ready_ValueSet(value);
}

void	HttpRequest::set_referrer_policy(const std::string &key, const std::string &value)
{
	if (value == "no-referrer" || value == "no-referrer-when-downgrade" || value == "origin" || value == "origin-when-cross-origin" || \
	value == "same-origin" || value == "strict-origin" || value == "strict-origin-when-cross-origin" || value == "unsafe-url")
		this->_request_keyvalue_map[key] = ready_ValueSet(value);
	else
		return;
}

void	HttpRequest::set_retry_after(const std::string &key, const std::string &value)
{
	this->_request_keyvalue_map[key] = ready_ValueSet(value);
}

void	HttpRequest::set_sec_fetch_dest(const std::string &key, const std::string &value)
{
	if (value == "audio" || value == "audioworklet" || value == "document" || value == "embed" || \
	value == "empty" || value == "font" || value == "frame" || value == "iframe" || value == "image" || value == "manifest" || \
	value == "object" || value == "paintworklet" || value == "report" || value == "script" || value == "serviceworker" || \
	value == "sharedworker" || value == "style" || value == "track" || value == "video" || value == "worker" || value == "xslt")
		this->_request_keyvalue_map[key] = ready_ValueSet(value);
	else
		return;
}

void	HttpRequest::set_sec_fetch_mode(const std::string &key, const std::string &value)
{
	if (value == "cors" || value == "navigate" || value == "no-cors" || value == "same-origin" || value == "websocket")
		this->_request_keyvalue_map[key] = ready_ValueSet(value);
	else
		return;
}

void	HttpRequest::set_sec_fetch_site(const std::string &key, const std::string &value)
{
	if (value == "cross-site" || value == "same-origin" || value == "same-site" || value == "none")
		this->_request_keyvalue_map[key] = ready_ValueSet(value);
	else
		return;
}

void	HttpRequest::set_sec_fetch_user(const std::string &key, const std::string &value)
{
	this->_request_keyvalue_map[key] = ready_ValueSet(value);
}

void	HttpRequest::set_sec_purpose(const std::string &key, const std::string &value)
{
	if (value == "prefetch")
		this->_request_keyvalue_map[key] = ready_ValueSet(value);
	else
		return;
}

void	HttpRequest::set_sec_websocket_accept(const std::string &key, const std::string &value)
{
	this->_request_keyvalue_map[key] = ready_ValueSet(value);
}

void	HttpRequest::set_server(const std::string &key, const std::string &value)
{
	this->_request_keyvalue_map[key] = ready_ValueSet(value);
}

void	HttpRequest::set_servertiming(const std::string &key, const std::string &value)
{
	// cpu;dur=2.4;a=b, cpu; ,,,みたいな感じなのでmapで保持しないほうがいいかもしれない
	// this->_request_keyvalue_map[key] = ready_ValueMap(value);
	(void)key;
	(void)value;
}

void	HttpRequest::set_service_worker_navigation_preload(const std::string &key, const std::string &value)
{
	this->_request_keyvalue_map[key] = ready_ValueSet(value);
}

void	HttpRequest::set_set_cookie(const std::string &key, const std::string &value)
{
	this->_request_keyvalue_map[key] = ready_ValueMap(value);
}

void	HttpRequest::set_sourcemap(const std::string &key, const std::string &value)
{
	this->_request_keyvalue_map[key] = ready_ValueSet(value);
}

void	HttpRequest::set_strict_transport_security(const std::string &key, const std::string &value)
{
	this->_request_keyvalue_map[key] = ready_ValueMap(value);
}

void	HttpRequest::set_te(const std::string &key, const std::string &value)
{
	std::stringstream				splited_by_commma(value);
	std::string						line;
	std::string						target_key;
	std::string						target_value;
	while(std::getline(splited_by_commma, line, ','))
	{
		if (line.find(';') != std::string::npos)
		{
			if (line[0] == ' ')
				line = line.substr(1);
			target_key = HandlingString::obtain_word_before_delimiter(line, ';');
			target_value = HandlingString::obtain_weight(HandlingString::obtain_word_after_delimiter(line, ';'));
			if (!(target_key == "compress" || target_key == "deflate" || target_key == "gzip" || target_key == "trailers"))
				return;
			if (HandlingString::is_double_or_not(target_value) == false)
				return;
		}
		else
		{
			if (line[0] == ' ')
				line = line.substr(1);
			if (!(line == "compress" || line == "deflate" || line == "gzip" || line == "trailers"))
				return;
		}
	}
	this->_request_keyvalue_map[key] = ready_ValueWeightArraySet(value);
}

void	HttpRequest::set_timing_allow_origin(const std::string &key, const std::string &value)
{
	this->_request_keyvalue_map[key] = ready_ValueSet(value);
}

void	HttpRequest::set_trailer(const std::string &key, const std::string &value)
{
	this->_request_keyvalue_map[key] = ready_ValueSet(value);
}

void	HttpRequest::set_transfer_encoding(const std::string &key, const std::string &value)
{
	std::stringstream	ss(value);
	std::string			line;
	std::string			skipping_firstemptyword;
	while(std::getline(ss, line, ','))
	{
		if (line[0] == ' ')
			skipping_firstemptyword = line.substr(1);
		else
			skipping_firstemptyword = line;
		if (skipping_firstemptyword != "gzip" && skipping_firstemptyword != "compress" && skipping_firstemptyword \
		!= "deflate" && skipping_firstemptyword != "gzip" && skipping_firstemptyword != "chunked")
			return;
	}
	this->_request_keyvalue_map[key] = ready_ValueArraySet(value);
}

void	HttpRequest::set_upgrade(const std::string &key, const std::string &value)
{
	this->_request_keyvalue_map[key] = ready_ValueArraySet(value);
}

void	HttpRequest::set_upgrade_insecure_requests(const std::string &key, const std::string &value)
{
	if (HandlingString::is_positive_and_under_intmax(value) == false)
		return;
	if (HandlingString::is_positive_and_under_intmax(value) == false)
		return;
	this->_request_keyvalue_map[key] = ready_ValueSet(value);
}

void	HttpRequest::set_user_agent(const std::string &key, const std::string &value)
{
	this->_request_keyvalue_map[key] = ready_ValueSet(value);
}

void	HttpRequest::set_vary(const std::string &key, const std::string &value)
{
	// headerのみしか許可しないのでは
	this->_request_keyvalue_map[key] = ready_ValueArraySet(value);
}

void	HttpRequest::set_via(const std::string &key, const std::string &value)
{
	this->_request_keyvalue_map[key] = ready_ValueSet(value);
}

void	HttpRequest::set_www_authenticate(const std::string &key, const std::string &value)
{
	this->_request_keyvalue_map[key] = ready_ValueArraySet(value);
}

void	HttpRequest::set_x_xss_protection(const std::string &key, const std::string &value)
{
	(void)key;
	(void)value;
}

void HttpRequest::ready_functionmap()
{
	this->_field_name_parser["Accept"] = &HttpRequest::set_accept;
	this->_field_name_parser["Accept-CH"] = &HttpRequest::set_accept_ch;
	this->_field_name_parser["Accept-Charset"] = &HttpRequest::set_accept_charset;
	this->_field_name_parser["Accept-Encoding"] = &HttpRequest::set_accept_encoding;
	this->_field_name_parser["Accept-Language"] = &HttpRequest::set_accept_language;
	// this->_field_name_parser["Accept-Patch"] = this->set_accept_patch;
	this->_field_name_parser["Accept-Post"] = &HttpRequest::set_accept_post;
	this->_field_name_parser["Accept-Ranges"] = &HttpRequest::set_accept_ranges;
	this->_field_name_parser["Access-Control-Allow-Credentials"] = &HttpRequest::set_access_control_allow_credentials;
	this->_field_name_parser["Access-Control-Allow-Headers"] = &HttpRequest::set_access_control_allow_headers;
	this->_field_name_parser["Access-Control-Allow-Methods"] = &HttpRequest::set_access_control_allow_methods;
	this->_field_name_parser["Access-Control-Allow-Origin"] = &HttpRequest::set_access_control_allow_origin;
	this->_field_name_parser["Access-Control-Expose-Headers"] = &HttpRequest::set_access_control_expose_headers;
	this->_field_name_parser["Access-Control-Max-Age"] = &HttpRequest::set_access_control_max_age;
	this->_field_name_parser["Access-Control-Request-Headers"] = &HttpRequest::set_access_control_request_headers;
	this->_field_name_parser["Access-Control-Request-Method"] = &HttpRequest::set_access_control_request_method;
	this->_field_name_parser["Age"] = &HttpRequest::set_age;
	this->_field_name_parser["Allow"] = &HttpRequest::set_allow;
	this->_field_name_parser["Alt-Svc"] = &HttpRequest::set_alt_svc;
	this->_field_name_parser["Alt-Used"] = &HttpRequest::set_alt_used;
	this->_field_name_parser["Authorization"] = &HttpRequest::set_authorization;
	this->_field_name_parser["Cache-Control"] =  &HttpRequest::set_cache_control;
	this->_field_name_parser["Clear-Site-Data"] = &HttpRequest::set_clear_site_data;
	this->_field_name_parser["Connection"] = &HttpRequest::set_connection;
	this->_field_name_parser["Content-Disposition"] = &HttpRequest::set_content_disponesition;
	this->_field_name_parser["Content-Encoding"] = &HttpRequest::set_content_encoding;
	this->_field_name_parser["Content-Language"] = &HttpRequest::set_content_language;
	this->_field_name_parser["Content-Length"] = &HttpRequest::set_content_length;
	this->_field_name_parser["Content-Location"] = &HttpRequest::set_content_location;
	this->_field_name_parser["Content-Range"] = &HttpRequest::set_content_range;
	this->_field_name_parser["Content-Security-Policy"] = &HttpRequest::set_content_security_policy;
	this->_field_name_parser["Content-Security-Policy-Report-Only"] = &HttpRequest::set_content_security_policy_report_only;
	this->_field_name_parser["Content-Type"] = &HttpRequest::set_content_type;
	this->_field_name_parser["Cookie"] = &HttpRequest::set_cookie;
	this->_field_name_parser["Cross-Origin-Embedder-Policy"] = &HttpRequest::set_cross_origin_embedder_policy;
	this->_field_name_parser["Cross-Origin-Opener-Policy"] = &HttpRequest::set_cross_origin_opener_policy;
	this->_field_name_parser["Cross-Origin-Resource-Policy"] = &HttpRequest::set_cross_origin_resource_policy;
	this->_field_name_parser["Date"] = &HttpRequest::set_date;
	this->_field_name_parser["ETag"] = &HttpRequest::set_etag;
	this->_field_name_parser["Expect"] = &HttpRequest::set_expect;
	// this->_field_name_parser["Expect-CT"] = this->set_expect_ct;
	this->_field_name_parser["Expires"] = &HttpRequest::set_expires;
	this->_field_name_parser["Forwarded"] = &HttpRequest::set_forwarded;
	this->_field_name_parser["From"] = &HttpRequest::set_from;
	this->_field_name_parser["Host"] = &HttpRequest::set_host;
	this->_field_name_parser["If-Match"] = &HttpRequest::set_if_match;
	this->_field_name_parser["If-Modified-Since"] = &HttpRequest::set_if_modified_since;
	this->_field_name_parser["If-None-Match"] = &HttpRequest::set_if_none_match;
	this->_field_name_parser["If-Range"] = &HttpRequest::set_if_range;
	this->_field_name_parser["If-Unmodified-Since"] = &HttpRequest::set_if_unmodified_since;
	this->_field_name_parser["Keep-Alive"] = &HttpRequest::set_keep_alive;
	this->_field_name_parser["Last-Modified"] = &HttpRequest::set_last_modified;
	this->_field_name_parser["Link"] = &HttpRequest::set_link;
	this->_field_name_parser["Location"] = &HttpRequest::set_location;
	this->_field_name_parser["Max-Forwards"] = &HttpRequest::set_max_forwards;
	this->_field_name_parser["Origin"] = &HttpRequest::set_origin;
	this->_field_name_parser["Permissions-Policy"] = &HttpRequest::set_permission_policy;
	this->_field_name_parser["Proxy-Authenticate"] = &HttpRequest::set_proxy_authenticate;
	this->_field_name_parser["Proxy-Authorization"] = &HttpRequest::set_proxy_authorization;
	// this->_field_name_parser["Range"] = this->set_range;
	this->_field_name_parser["Referer"] = &HttpRequest::set_referer;
	this->_field_name_parser["Retry-After"] = &HttpRequest::set_retry_after;
	this->_field_name_parser["Sec-Fetch-Dest"] = &HttpRequest::set_sec_fetch_dest;
	this->_field_name_parser["Sec-Fetch-Mode"] = &HttpRequest::set_sec_fetch_mode;
	this->_field_name_parser["Sec-Fetch-Site"] = &HttpRequest::set_sec_fetch_site;
	this->_field_name_parser["Sec-Fetch-User"] = &HttpRequest::set_sec_fetch_user;
	this->_field_name_parser["Sec-Purpose"] = &HttpRequest::set_sec_purpose;
	this->_field_name_parser["Sec-WebSocket-Accept"] = &HttpRequest::set_sec_websocket_accept;
	this->_field_name_parser["Server"] = &HttpRequest::set_server;
	// this->_field_name_parser["Server-Timing"] = this->set_server_timing;
	this->_field_name_parser["Service-Worker-Navigation-Preload"] = &HttpRequest::set_service_worker_navigation_preload;
	this->_field_name_parser["Set-Cookie"] = &HttpRequest::set_cookie;
	this->_field_name_parser["SourceMap"] = &HttpRequest::set_sourcemap;
	this->_field_name_parser["Strict-Transport-Security"] = &HttpRequest::set_strict_transport_security;
	this->_field_name_parser["TE"] = &HttpRequest::set_te;
	this->_field_name_parser["Timing-Allow-Origin"] = &HttpRequest::set_timing_allow_origin;
	this->_field_name_parser["Trailer"] = &HttpRequest::set_trailer;
	this->_field_name_parser["Transfer-Encoding"] = &HttpRequest::set_transfer_encoding;
	this->_field_name_parser["Upgrade"] = &HttpRequest::set_upgrade;
	this->_field_name_parser["Upgrade-Insecure-Requests"] = &HttpRequest::set_upgrade_insecure_requests;
	this->_field_name_parser["User-Agent"] = &HttpRequest::set_user_agent;
	this->_field_name_parser["Vary"] = &HttpRequest::set_vary;
	this->_field_name_parser["Via"] = &HttpRequest::set_via;
	this->_field_name_parser["WWW-Authenticate"] = &HttpRequest::set_www_authenticate;
	// this->_field_name_parser["X-Custom-Header"] = &HttpRequest::set_x_custom_header;
}

RequestLine& HttpRequest::get_request_line()
{
	return (this->_request_line);
}

BaseKeyValueMap* HttpRequest::return_value(const std::string &key)
{
	return (this->_request_keyvalue_map[key]);
}

std::map<std::string, BaseKeyValueMap*> HttpRequest::get_request_keyvalue_map(void)
{
	return (this->_request_keyvalue_map);
}

int	HttpRequest::get_statuscode() const
{
	return (this->_status_code);
}