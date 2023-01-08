#ifndef _DEBUG_H__
#define _DEBUG_H__

#include "sdk.h"
#ifdef TDEBUG

#include <map>
#include <vector>
#include <functional>

// #define LFSE_COMMAND_MAX_LENGTH 10
#define LFSE_SERIAL_BUFFER_LENGTH 64

inline static bool isValidFSNameChar(char c) { return isAlphaNumeric(c) || c == '-'; }
static bool isValidFSName(const String& name) { for (const char& c : name) if (!isValidFSNameChar(c)) return false; return !name.isEmpty(); }
static bool isValidFSPath(const String& name) { for (const char& c : name) if (!isValidFSNameChar(c) && c != '/') return false; return !name.isEmpty(); }

struct LFSEPath {
	std::vector<String> _tokens;

	LFSEPath() = default;
	LFSEPath(const LFSEPath& obj) {
		for (const String& token : obj._tokens)
			_tokens.push_back(token);
	}
	LFSEPath(const String& root) {
		tokenize(root);
	}

	// replaces if path is absolute, adds if relative
	void adjust(const String& path) {
		if (path.isEmpty() || !isValidFSPath(path)) {
			return;
		}
		// DLOG("adjust current path ");
		// LOG(toString());
		// LOG(" with path ");
		// LOGLN(path);
		tokenize(path, path[0] != '/');
		// DLOG("after adjustment ");
		// LOGLN(toString());
	}

	bool isEmpty() const { return _tokens.size(); }
	String toString(bool trailingSlash = false) const {
		String res;
		for (const String& token : _tokens) {
			res += "/";
			res += token;
		}
		if (trailingSlash || res.isEmpty())
			res += "/";
		return res;
	}
private:
	void tokenize(const String& s, bool appendToExisting = false) {
		if (!appendToExisting)
			_tokens.clear();
		String token;
		for (uint16_t i = 0; i < s.length(); ++i) {
			char c = s[i];
			if (isValidFSNameChar(c)) {
				token += c;
				continue;
			}
			if (c == '/') {
				if (token.length()) {
					_tokens.push_back(token);
					token.clear();
				}
			}
		}
		if (token.length()) {
			_tokens.push_back(std::move(token));
		}
	}
};

struct LSFECommand {
	String _cmd;
	std::vector<String> _args;
	char* _buffer = nullptr;
	uint16_t _bufferLength = 0;

	LSFECommand(char* buffer, uint16_t len) : _buffer(buffer), _bufferLength(len) {
		parse(buffer, len);
	}

	String toString() const {
		String res(_cmd);
		for (const String& arg : _args) {
			res += " ";
			res += arg;
		}
		return res;
	}
private:
	void parse(char* buffer, uint16_t len) {
		_cmd.clear();
		_args.clear();
		String token;
		for (uint16_t i = 0; i < len; ++i) {
			char c = buffer[i];
			if (isValidFSNameChar(c) || c == '/') {
				// DLOGLN(c);
				token += c;
			} else {
				// LOGLN("!isAlphaNumeric()");
				if (!_cmd.length()) { // skip leading spaces (and bulshit)
					// LOGLN("!_cmd.length()");
					if (token.length()) {
						// DLOGLN(token);
						_cmd = std::move(token);
						token.clear();
					}
					continue;
				}
				if (token.length()) {
					// DLOGLN(token);
					_args.push_back(std::move(token));
					token.clear();
				}
				continue;
			}
		}
		if (token.length()) {
			if (!_cmd.length()) {
				_cmd = std::move(token);
			} else {
				_args.push_back(std::move(token));
			}
		}
		_cmd.toLowerCase();
	}
};


typedef std::function<void(const LSFECommand&)> cmdFunc;
typedef std::tuple<cmdFunc, String, String> cmdInfo; // function, arguments description, command description
typedef std::pair<String, cmdInfo> cmdMapEntry;

class DEBUG {
public:
	static void LittleFSExplorer();
private:
	static std::map<String, cmdInfo> lfseCmdMap;
	static char lfseBuffer[];
	static LFSEPath lfsePath;

	static void logExecutedCommand(const LSFECommand& cmd);

	static void cmdHelp(const LSFECommand& cmd);
	static void cmdFormat(const LSFECommand& cmd);
	static void cmdLs(const LSFECommand& cmd);
	static void cmdCd(const LSFECommand& cmd);
	static void cmdPwd(const LSFECommand& cmd);
	static void cmdMkdir(const LSFECommand& cmd);
	static void cmdMv(const LSFECommand& cmd);
	static void cmdRm(const LSFECommand& cmd);
	static void cmdCat(const LSFECommand& cmd);
	static void cmdMan(const LSFECommand& cmd);

	static void handleCommand(uint16_t length);
};

#endif // #ifdef TDEBUG
#endif // _DEBUG_H__