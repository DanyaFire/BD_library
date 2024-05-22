#include "common.h"
#include "config.h"
#include "database.h"
#include "query.h"
#include "record.h"
#include "result.h"

#include <iostream>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

std::string urlDecode(const std::string& str);
std::string generateHomePage();
std::string generateQueryResponse(const Result& record);

struct Server {
    virtual bool process(int fd, DataBase &db) = 0;
};

struct Server_Client : public Server {
    virtual bool process(int fd, DataBase &db) {
    // 1. Read string from client
    int n;

    if (myRead(fd, &n, sizeof(n)) != IOStatus::OK) {
        return false;
    }

    std::vector<char> cmd(n);

    if (myRead(fd, &cmd[0], n) != IOStatus::OK) {
        return false;
    }
  
    printf("Body: %s\n", &cmd[0]);
  
    // 2. Process input data
    Query query;
    query.parse(&cmd[0]);
    Result record = db.process(query);
    // 3. Send result
    if (record.writeBin(fd) != IOStatus::OK) {
        return false;
    }

    return true;
}
};

std::string urlDecode(const std::string& str) {
    std::string result;
    std::string::size_type i;
    for (i = 0; i < str.length(); ++i) {
        if (str[i] == '+') {
            result += ' ';
        } else if (str[i] == '%') {
            std::string hex = str.substr(i + 1, 2);
            result += char(strtol(hex.c_str(), nullptr, 16));
            i += 2;
        } else {
            result += str[i];
        }
    }
    
    // Удаляем приписку "query="
    std::string prefix = "query=";
    if (result.substr(0, prefix.length()) == prefix) {
        result.erase(0, prefix.length());
    }
    
    return result;
}


std::string generateQueryResponse(const Result& result) {
    std::stringstream ss;
    ss << "<html><body><h1>Query Result</h1><table>";

    for (const Record& record : result.records()) {
        ss << "<tr>";
        for (const auto& value : record.values()) {
            ss << "<td>" << value << "</td>";
        }
        ss << "</tr>";
    }

    ss << "</table></body></html>";
    return ss.str();
}

std::string generateHomePage() {
    return "<html><body><h1>Welcome to my server!</h1><form action='/query' method='get'><input type='text' name='query' placeholder='Enter your query'><input type='submit' value='Submit'></form></body></html>";
}

struct ServerHTTP : public Server {
    virtual bool process(int fd, DataBase &db) {
    // 1. Read HTTP request from client
    std::string request;
    char buf[1024];
    ssize_t bytesRead;

    while ((bytesRead = read(fd, buf, sizeof(buf))) > 0) {
        request.append(buf, bytesRead);
        if (request.find("\r\n\r\n") != std::string::npos) {
            break;
        }
    }

    if (bytesRead == -1) {
        fprintf(stderr, "Error reading from socket\n");
        return false;
    }

    // 2. Parse HTTP request
    std::istringstream iss(request);
    std::string method, path, protocol;
    iss >> method >> path >> protocol;

    // 3. Process request based on path
    std::string response;
    std::cout<<path<<std::endl;
    if (path == "/") {
        response = generateHomePage();
    } else if (path.find("/query") == 0) {
        std::string queryString = path.substr(path.find('?') + 1);
        Query query;
        //std::cout<<queryString<<std::endl;
        query.parse(urlDecode(queryString).c_str());
        //std::cout<<urlDecode(queryString).c_str()<<std::endl;
        Result record = db.process(query);
        response = generateQueryResponse(record);
    } else {
        response = "404 Not Found";
    }

    // 4. Send HTTP response
    std::string httpResponse = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + std::to_string(response.length()) + "\r\n\r\n" + response;
    if (write(fd, httpResponse.c_str(), httpResponse.length()) == -1) {
        fprintf(stderr, "Error writing to socket\n");
        return false;
    }

    return true;
}
};