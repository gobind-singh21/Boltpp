#include "request.h"
#include "response.h"
#include "utils.h"

#include <iostream>

inline auto JsonBodyParser = [](Req &req, Res &res, long long &next) {
  if(req.headers["Content-Type"].find("application/json") != std::string::npos) {
    try {
      req.body = JSONParser(req.payload).parse();
    } catch (const std::exception &e) {
      res.status(400)->send("Bad Request");
      next = -1;
      return;
    }
  }
  next++;
};

inline auto UrlencodedBodyParser = [](Req &req, Res &res, long long &next) {
  if(req.headers["Content-Type"].find("application/x-www-form-urlencoded") != std::string::npos) {
    auto inline urlEncodingCharacter = [](const std::string sequence) {
      if(sequence[0] != '%' && sequence.length() != 3)
        return '\0';
      if(sequence.compare("%20") == 0)
        return ' ';
      else if(sequence.compare("%26") == 0)
        return '&';
      else if(sequence.compare("%3D") == 0)
        return '=';
      else if(sequence.compare("%3F") == 0)
        return '?';
      else if(sequence.compare("%23") == 0)
        return '#';
      else if(sequence.compare("%25") == 0)
        return '%';
      return '\0';
    };
    bool keyEnd = false;
    size_t length = req.payload.length(), thirdLast = length - 3;
    std::string key = "", value = "";
    key.reserve(length);
    value.reserve(length);
    JSONValue::Object json;
    for(size_t i = 0; i < length; i++) {
      char c = req.payload[i];
      if(c == '&') {
        json[trim(key)] = JSONValue(trim(value));
        keyEnd = false;
      } else if(c == '=')
        keyEnd = true;
      else if(c == '%' && i < thirdLast) {
        char ch = urlEncodingCharacter(req.payload.substr(i, 3));
        if(keyEnd)
          value.push_back(ch);
        else
          key.push_back(ch);
        i += 2;
      } else {
        if(keyEnd)
          value.push_back(c);
        else
          key.push_back(c);
      }
    }
    if(key != "")
      json[trim(key)] = JSONValue(trim(value));
    req.body = JSONValue(json);
  }
  next++;
};