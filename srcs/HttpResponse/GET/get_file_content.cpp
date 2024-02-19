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

    extension = StringHandler::get_extension(path);
    itr = mime_types.find(extension);
    return itr != mime_types.end();
}


Result<int, int> HttpResponse::get_file_content(const std::string &file_path,
                                                const std::map<std::string, std::string> &mime_types,
                                                std::vector<unsigned char> *buf,
                                                int *status_code) {
    if (!buf || !status_code) {
        return Result<int, int>::err(ERR);
    }
    DEBUG_PRINT(CYAN, "       get_file_content");
    DEBUG_PRINT(CYAN, "        1 path:[%s]", file_path.c_str());
    if (!is_support_content_type(file_path, mime_types)) {
        *status_code = STATUS_NOT_ACCEPTABLE;
		return Result<int, int>::err(ERR);
	}

    DEBUG_PRINT(CYAN, "        2");
    std::ifstream file(file_path, std::ios::binary);
    if (!file) {
        *status_code = STATUS_BAD_REQUEST;  // todo
        return Result<int, int>::err(ERR);
    }
    DEBUG_PRINT(CYAN, "        3");

    file.seekg(0, std::ios::end);
    std::streamsize file_size = file.tellg();
    file.seekg(0, std::ios::beg);
    DEBUG_PRINT(CYAN, "        4 file_size: %zu", file_size);

    buf->resize(file_size);
    if (!file.read(reinterpret_cast<char*>(&(*buf)[0]), file_size)) {
        const std::string error_msg = CREATE_ERROR_INFO_STR("fail to read file: " + file_path);
        std::cerr << error_msg << std::endl;  // todo log

        buf->clear();
        *status_code = STATUS_BAD_REQUEST;  // todo
        return Result<int, int>::err(ERR);
    }
    DEBUG_PRINT(CYAN, "        5");
    std::string body(this->body_buf_.begin(), this->body_buf_.end());
    DEBUG_PRINT(CYAN, "     recv_body:[%s]", body.c_str());

    *status_code = STATUS_OK;
    return Result<int, int>::ok(OK);
}
