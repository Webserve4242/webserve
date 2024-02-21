#include <fcntl.h>
#include <sys/stat.h>
#include <cerrno>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include "Color.hpp"
#include "Error.hpp"
#include "Debug.hpp"
#include "HttpResponse.hpp"
#include "Result.hpp"
#include "StringHandler.hpp"

namespace {

}  // namespace


////////////////////////////////////////////////////////////////////////////////

bool is_support_content_type(const std::string &path,
                             const std::map<std::string, std::string> &mime_types) {
    std::string extension;
    std::map<std::string, std::string>::const_iterator itr;
    DEBUG_PRINT(CYAN, "    is_support_content_type");

    extension = StringHandler::get_extension(path);

    DEBUG_PRINT(CYAN, "     path      : %s", path.c_str());
    DEBUG_PRINT(CYAN, "     extensnion: %s", extension.c_str());
    itr = mime_types.find(extension);
    return itr != mime_types.end();
}


// todo: get from conf
std::map<std::string, std::string> mime_types() {
    std::map<std::string, std::string> mime_types;

    mime_types["html"] = "text/html";
    mime_types["htm"] = "text/htm";
    mime_types["jpg"] = "text/htm";
    return mime_types;
}


Result<ProcResult, StatusCode> HttpResponse::get_file_content(const std::string &file_path,
                                                              std::vector<unsigned char> *buf) {
    if (!buf) {
        return Result<ProcResult, StatusCode>::err(InternalServerError);
    }
    DEBUG_PRINT(CYAN, "    get_file_content 1 path:[%s]", file_path.c_str());

    DEBUG_PRINT(CYAN, "    get_file_content 2");
    std::ifstream file(file_path.c_str(), std::ios::binary);
    if (!file) {
        DEBUG_PRINT(CYAN, "    get_file_content 3 -> file not found 404");
        return Result<ProcResult, StatusCode>::err(NotFound);
    }
    DEBUG_PRINT(CYAN, "    get_file_content 5");

    if (!is_support_content_type(file_path, mime_types())) {
        DEBUG_PRINT(RED, "   not support content: %s", file_path.c_str());
		return Result<ProcResult, StatusCode>::err(NotAcceptable);
	}

    file.seekg(0, std::ios::end);
    std::streamsize file_size = file.tellg();
    file.seekg(0, std::ios::beg);
    DEBUG_PRINT(CYAN, "    get_file_content 5 file_size: %zu", file_size);

    buf->resize(file_size);
    if (!file.read(reinterpret_cast<char*>(&(*buf)[0]), file_size)) {
        const std::string error_msg = CREATE_ERROR_INFO_STR("fail to read file: " + file_path);
        std::cerr << error_msg << std::endl;  // todo log

        buf->clear();
        return Result<ProcResult, StatusCode>::err(BadRequest);
    }
    DEBUG_PRINT(CYAN, "    get_file_content 6");
    std::string body(this->body_buf_.begin(), this->body_buf_.end());
    DEBUG_PRINT(CYAN, "    get_file_content recv_body:[%s]", body.c_str());

    return Result<ProcResult, StatusCode>::ok(Success);
}
