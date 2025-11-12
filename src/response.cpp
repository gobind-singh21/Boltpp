#include "response.h"

#include <filesystem>
#include <functional>
#include <algorithm>

Response& Response::setProtocol(const std::string protocol) {
  this->protocol = protocol;
  return *(this);
}

Response& Response::status(int statusCode) {
  this->statusCode = statusCode;
  return *this;
}

Response& Response::json(const JSONValue &j) {
  std::string jsonString = j.stringify();
  this->payload = jsonString;
  headers["Content-Type"] = "application/json";
  headers["Content-Length"] = std::to_string(jsonString.length());
  return *this;
}

Response& Response::send(const std::string_view dataView) {
  this->payload = dataView;
  headers["Content-Length"] = std::to_string(dataView.length());
  return *this;
}

Response& Response::setHeader(const std::string_view key, const std::string_view value) {
  headers[std::string(key)] = value;
  return *this;
}

const std::string Response::getMimeType(const std::string& extension) {
  static const std::unordered_map<std::string, std::string> mime_types = {
    {".html", "text/html"},
    {".css", "text/css"},
    {".js", "application/javascript"},
    {".json", "application/json"},
    {".txt", "text/plain"},
    {".jpg", "image/jpeg"},
    {".jpeg", "image/jpeg"},
    {".png", "image/png"},
    {".gif", "image/gif"},
    {".svg", "image/svg+xml"},
    {".pdf", "application/pdf"},
    {".zip", "application/zip"},
    {".mp4", "video/mp4"}
  };
  auto it = mime_types.find(extension);
  if (it != mime_types.end()) {
    return it->second;
  }
  return "application/octet-stream";
}

Response& Response::sendFile(const std::string_view file_path) {
  this->file_path = file_path;
  this->isFileResponse = true;

  std::filesystem::path fsPath(file_path);
  std::string extension = fsPath.has_extension() ? fsPath.extension().string() : "";
  std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

  headers["Content-Type"] = getMimeType(extension);

  headers["Content-Disposition"] = "inline; filename=\"" + fsPath.filename().string() + "\"";

  return *this;
}

Response& Response::download(const std::string_view file_path) {
  this->file_path = file_path;
  this->isFileResponse = true;

  std::filesystem::path fsPath(file_path);
  std::string extension = fsPath.has_extension() ? fsPath.extension().string() : "";
  std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

  headers["Content-Type"] = getMimeType(extension);

  headers["Content-Disposition"] = "attachment; filename=\"" + fsPath.filename().string() + "\"";

  return *this;
}