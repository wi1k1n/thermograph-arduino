#include "_debug.h"
#include "filesystem.h"

std::map<String, cmdInfo> DEBUG::lfseCmdMap = {
	cmdMapEntry("help", cmdInfo(cmdHelp, "", "show help message")),
	cmdMapEntry("format", cmdInfo(cmdFormat, "", "format filesystem (delete all data)")),
	cmdMapEntry("ls", cmdInfo(cmdLs, "[dirpath]", "list children files/directories")),
	cmdMapEntry("cd", cmdInfo(cmdCd, "[dirpath]", "change current working directory")),
	cmdMapEntry("pwd", cmdInfo(cmdPwd, "", "show current working directory")),
	cmdMapEntry("mkdir", cmdInfo(cmdMkdir, "[dirname]", "create directory (not recursive)")),
	cmdMapEntry("mv", cmdInfo(cmdMv, "[path_from] [path_to]", "move and/or rename file/directory")),
	cmdMapEntry("rm", cmdInfo(cmdRm, "[path]", "remove file/directory")),
	cmdMapEntry("touch", cmdInfo(cmdTouch, "[filepath]", "create empty file")),
	cmdMapEntry("write", cmdInfo(cmdWrite, "[\"content_args\"] [filepath]", "overwrite text content to file")),
	// cmdMapEntry("append", cmdInfo(cmdAppend, "[\"content_args\"] [filepath]", "append text content to existing file")),
	cmdMapEntry("cat", cmdInfo(cmdCat, "[filepath]", "print content of the file")),
	cmdMapEntry("man", cmdInfo(cmdMan, "[command]", "show manual for command")),
};
LFSEPath DEBUG::lfsePath;
char DEBUG::lfseBuffer[LFSE_SERIAL_BUFFER_LENGTH];

void DEBUG::cmdHelp(LFSECommand& cmd) {
	LOGLN(F("The following commands are available for execution:"));
	for (const cmdMapEntry& cmdEntry : lfseCmdMap) {
		LOG(cmdEntry.first);
		LOG(F(" "));
		LOG(std::get<1>(cmdEntry.second));
		LOG(F("\t"));
		LOGLN(std::get<2>(cmdEntry.second));
	}
}
void DEBUG::cmdFormat(LFSECommand& cmd) {
	if (!LittleFS.format()) {
		LOGLN(F("Formatting filesystem failed!"));
	}
}
void DEBUG::cmdPwd(LFSECommand& cmd) {
	LOGLN(lfsePath.toString());
}
void DEBUG::cmdLs(LFSECommand& cmd) {
	cmd.parseArgs();
	String userPath;
	if (cmd._args.size()) {
		userPath = cmd.getArgFirstFilenameOrLastArg();
		if (checkInvalidDirPath(userPath))
			return;
	}
	String dirPath(lfsePath.createAdjustedFromUserPath(userPath).toString());
	if (checkDoesntExist(dirPath))
		return;
	Dir dir = LittleFS.openDir(dirPath);
	while (dir.next()) {
		LOG(dir.isFile() ? F("f ") : (dir.isDirectory() ? F("d ") : F("- ")));
		const size_t nCharFS = LOG(dir.fileSize());
		for (uint8_t i = 0; i < max(6 - nCharFS, (unsigned int)0); ++i)
			LOG(' ');
		const size_t nCharCT = LOG(dir.fileCreationTime());
		for (uint8_t i = 0; i < max(10 - nCharCT, (unsigned int)0); ++i)
			LOG(' ');
		LOGLN(dir.fileName());
	}
}
void DEBUG::cmdCd(LFSECommand& cmd) {
	cmd.parseArgs();
	if (checkMissingOperand(cmd))
		return;
	String userPath(cmd.getArgFirstFilenameOrLastArg());
	if (checkInvalidDirPath(userPath) || checkDoesntExist(userPath))
		return;
	lfsePath.adjust(userPath);
}
void DEBUG::cmdMkdir(LFSECommand& cmd) {
	cmd.parseArgs();
	if (checkMissingOperand(cmd))
		return;
	String userPath(cmd.getArgFirstFilenameOrLastArg());
	if (checkInvalidDirPath(userPath)) 
		return;
	String dirPath(lfsePath.createAdjustedFromUserPath(userPath).toString());
	if (checkAlreadyExists(dirPath))
		return;
	if (!LittleFS.mkdir(dirPath)) {
		LOG(F("Failed to create directory "));
		LOGLN(dirPath);
		return;
	}
}
void DEBUG::cmdMv(LFSECommand& cmd) {
	cmd.parseArgs();
	if (checkMissingOperand(cmd, 2))
		return;
	uint8_t filePath1ArgIdx = cmd.getArgFirstFilenameOrLastArgIdx();
	String userPath1(cmd._args[filePath1ArgIdx]);
	String userPath2(cmd.getArgFirstFilenameOrLastArg(filePath1ArgIdx + 1));
	if (checkInvalidFilePath(userPath1) || checkInvalidFilePath(userPath2))
		return;
	String path1 = lfsePath.createAdjustedFromUserPath(userPath1).toString();
	String path2 = lfsePath.createAdjustedFromUserPath(userPath2).toString();
	if (checkDoesntExist(path1) || checkAlreadyExists(path2))
		return;
	if (!LittleFS.rename(path1, path2)) {
		LOG(F("Failed to move from "));
		LOG(path1);
		LOG(F(" to "));
		LOGLN(path2);
		return;
	}
}
void DEBUG::cmdTouch(LFSECommand& cmd) {
	cmd.parseArgs();
	if (checkMissingOperand(cmd))
		return;
	String userPath(cmd.getArgFirstFilenameOrLastArg());
	if (checkInvalidFilePath(userPath)) 
		return;
	String filePath(lfsePath.createAdjustedFromUserPath(userPath).toString());
	if (checkAlreadyExists(filePath))
		return;
	File f = LittleFS.open(filePath, "w");
	if (!f) {
		LOG(F("Failed to create file "));
		LOGLN(filePath);
		return;
	}
	f.close();
}
void DEBUG::cmdWrite(LFSECommand& cmd) {
	_cmdWriteHelper(cmd);
}
// void DEBUG::cmdAppend(LFSECommand& cmd) {
// 	_cmdWriteHelper(cmd, true);
// }
void DEBUG::cmdRm(LFSECommand& cmd) {
	cmd.parseArgs();
	if (checkMissingOperand(cmd))
		return;
	
	std::unordered_set<char> flags = cmd.getArgFirstFlags().getSingleCharFlagsSet();
	bool removeDir = flags.find('r') != flags.end();

	String userPath(cmd.getArgFirstFilenameOrLastArg());
	String path = lfsePath.createAdjustedFromUserPath(userPath).toString();
	// r flag specified -> trying remove directory
	if (removeDir) {
		if (checkInvalidDirPath(userPath))
			return;
		if (checkDoesntExist(path))
			return;
		Dir dir = LittleFS.openDir(path);
		if (dir.isDirectory()) {
			if (!LittleFS.rmdir(path)) {
				LOG(F("Failed to remove directory "));
				LOGLN(path);
			}
		} else {
			LOG(path);
			LOGLN(F(" is not a directory"));
		}
		return;
	}
	// r flag is not present -> removing file
	if (checkInvalidFilePath(userPath))
		return;
	if (checkDoesntExist(path))
		return;
	Dir dir = LittleFS.openDir(path);
	if (dir.isFile()) {
		if (!LittleFS.remove(path)) {
			LOG(F("Failed to remove file "));
			LOGLN(path);
		}
	} else {
		LOG(path);
		LOGLN(F(" is not a file"));
	}
	return;
}
void DEBUG::cmdCat(LFSECommand& cmd) {
	cmd.parseArgs();
	if (checkMissingOperand(cmd))
		return;
	String userPath(cmd.getArgFirstFilenameOrLastArg());
	if (checkInvalidFilePath(userPath)) 
		return;
	String filePath(lfsePath.createAdjustedFromUserPath(userPath).toString());
	if (checkDoesntExist(filePath))
		return;

	File f = LittleFS.open(filePath, "r");
	if (!f) {
		LOG(F("Failed to open file "));
		LOGLN(filePath);
		return;
	}
	if (checkIsADir(f, filePath)) {
		f.close();
		return;
	}

	std::unordered_set<char> flags = cmd.getArgFirstFlags().getSingleCharFlagsSet();
	bool lineNumbers = flags.find('n') != flags.end();
	bool byteView = flags.find('b') != flags.end();

	uint16_t limitColumn = 128;
	uint16_t rowIdxFirst = 0;
	uint16_t rowIdxLast = 64;
	// TODO: change _args' type from vector to hashset and make this flag part easier
	uint8_t limitsFound = 0;
	auto handleCharLimit = [&](const LFSECommand::Arg& arg, char c, uint16_t& dst) {
		if (arg.value[0] == c) {
			int16_t t = arg.value.substring(1).toInt();
			if (t > 0)
				dst = t;
			++limitsFound;
		}
	};
	for (const LFSECommand::Arg& arg : cmd._args) {
		if (arg.type != LFSECommand::Arg::Type::FLAG || arg.value.isEmpty())
			continue;
		handleCharLimit(arg, 'c', limitColumn);
		handleCharLimit(arg, 'f', rowIdxFirst);
		handleCharLimit(arg, 'l', rowIdxLast);
		if (limitsFound == 3)
			break;
	}

	uint16_t lineIdx = 0;
	while (f.available() && lineIdx <= rowIdxLast) {
		String line = f.readStringUntil('\n');
		if (lineIdx < rowIdxFirst) {
			++lineIdx;
			continue;
		}
		if (lineNumbers) {
			LOG(lineIdx + 1);
			LOG(F("\t"));
		}
		if (byteView) {
			for (uint16_t i = 0; i < line.length() && i < limitColumn; ++i) {
				LOGF("%02x", line[i]);
				LOG(F(" "));
			}
		} else {
			LOG(line.substring(0, min((uint16_t)(line.length()-1), limitColumn)));
		}
		if (limitColumn < line.length()) {
			LOGLN(F("..."));
		} else {
			LOGLN();
		}
		++lineIdx;
	}
	f.close();
}
void DEBUG::cmdMan(LFSECommand& cmd) {
	cmd.parseArgs();
	if (checkMissingOperand(cmd))
		return;
}

void DEBUG::_cmdWriteHelper(LFSECommand& cmd, bool append) {
	cmd.parseArgs();
	if (checkMissingOperand(cmd))
		return;
	uint8_t filePathArgIdx = cmd.getArgFirstFilenameOrLastArgIdx();
	String userPath(cmd._args[filePathArgIdx]);
	if (checkInvalidFilePath(userPath)) 
		return;
	String filePath(lfsePath.createAdjustedFromUserPath(userPath).toString());
	File f = LittleFS.open(filePath, append ? "a" : "w+");
	if (!f) {
		LOG(F("Failed to open file "));
		LOGLN(filePath);
		return;
	}
	if (f.isDirectory()) {
		if (checkIsADir(f, filePath)) {
			f.close();
			return;
		}
	}

	std::unordered_set<char> flags = cmd.getArgFirstFlags().getSingleCharFlagsSet();
	bool newLines = flags.find('n') != flags.end();

	// iterate over all string args that go before the filename arg
	bool dirty = false;
	for (uint8_t i = filePathArgIdx; i < cmd._args.size(); ++i) {
		LFSECommand::Arg& arg = cmd._args[i];
		if (arg.type == LFSECommand::Arg::Type::STRING) {
			dirty = true;
			if (newLines)
				f.println(arg.value);
			else
				f.print(arg.value);
		}
	}
	f.close();
	if (!dirty) {
		LOGLN(F("Missing data to write"));
	}
}


inline static void _checkIsA(const String& path, const String& type) {
	LOG(path);
	LOG(F(" is a "));
	LOGLN(type);
}
bool DEBUG::checkIsAFile(const File& f, const String& path) {
	if (f.isDirectory())
		return false;
	_checkIsA(path, F("file"));
	return true;
}
bool DEBUG::checkIsADir(const File& f, const String& path) {
	if (f.isFile())
		return false;
	_checkIsA(path, F("directory"));
	return true;
}
bool DEBUG::checkMissingOperand(LFSECommand& cmd, uint8_t nRequiredArgs, LFSECommand::Arg::Type requiredType) {
	if (cmd._args.size() >= nRequiredArgs) {
		uint8_t correctArgs = 0;
		for (const LFSECommand::Arg& arg : cmd._args) {
			if (arg.type == requiredType)
				++correctArgs;
		}
		if (correctArgs >= nRequiredArgs)
			return false;
	}
	LOGLN(F("Missing operand"));
	return true;
}
inline static void _checkInvalidPathLog(const String& path, const String& type) {
	LOG(F("Invalid "));
	LOG(type);
	LOG(F(" path: "));
	LOGLN(path);
}
bool DEBUG::checkInvalidDirPath(const String& path) {
	if (isValidFSPath(path))
		return false;
	_checkInvalidPathLog(path, F("directory"));
	return true;
}
bool DEBUG::checkInvalidFilePath(const String& path) {
	if (isValidFSPath(path))
		return false;
	_checkInvalidPathLog(path, F("file"));
	return true;
}
bool DEBUG::checkAlreadyExists(const String& path) {
	if (!LittleFS.exists(path))
		return false;
	LOG(path);
	LOGLN(F(" already exists"));
	return true;
}
bool DEBUG::checkDoesntExist(const String& path) {
	if (LittleFS.exists(path))
		return false;
	LOG(path);
	LOGLN(F(" doesn't exist"));
	return true;
}


void DEBUG::logExecutedCommand(const LFSECommand& cmd) {
	LOG(lfsePath.toString());
	LOG(F("$ "));
	LOGLN(cmd.toString());
}
void DEBUG::handleCommand(uint16_t length) {
	if (!length || length >= LFSE_SERIAL_BUFFER_LENGTH)
		return;

	LFSECommand cmd(lfseBuffer, length);
	auto search = lfseCmdMap.find(cmd._cmd);
	logExecutedCommand(cmd);
	if (search == lfseCmdMap.end()) {
		LOG(F("Error: command "));
		LOG(cmd._cmd);
		LOGLN(F(" not found!"));
		return;
	}
	std::get<0>(search->second)(cmd);
}

void DEBUG::LittleFSExplorer() {
	while (Serial.available() > 0) {
		size_t nBytesGot = Serial.readBytesUntil('\n', lfseBuffer, LFSE_SERIAL_BUFFER_LENGTH);
		if (nBytesGot == 0) {
			LOGLN(F("Error: Could not read serial data: no valid data found!"));
			return;
		}
		if (nBytesGot == LFSE_SERIAL_BUFFER_LENGTH) {
			LOGLN(F("Error: Too big command!"));
			return;
		}
		handleCommand(nBytesGot);
	}
}

////////////////////////////////////////

LFSEPath::LFSEPath(const LFSEPath& obj) {
	for (const String& token : obj._tokens)
		_tokens.push_back(token);
}
LFSEPath::LFSEPath(const String& root) {
	tokenize(root);
}

// replaces if path is absolute, adds if relative
void LFSEPath::adjust(const String& path) {
	if (path.isEmpty() || !isValidFSPath(path)) {
		return;
	}
	tokenize(path, path[0] != '/');
}

String LFSEPath::toString(bool trailingSlash) const {
	String res;
	for (const String& token : _tokens) {
		res += "/";
		res += token;
	}
	if (trailingSlash || res.isEmpty())
		res += "/";
	return res;
}

void LFSEPath::tokenize(const String& s, bool appendToExisting) {
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
	
	// get rid of '.'s and empties
	_tokens.erase(std::remove_if(_tokens.begin(), _tokens.end(), [](const String& token) {
		return token.isEmpty() || token.equals(F("."));
	}), _tokens.end());

	// resolve '..'s
	{
		auto it = _tokens.rbegin();
		while(it != _tokens.rend()) {
			auto token = *it;
			if (token.equals(F(".."))) {
				it = decltype(it)(_tokens.erase(std::next(it).base()));
				if (it != _tokens.rend())
					it = decltype(it)(_tokens.erase(std::next(it).base()));
				continue;
			}
			++it;
		}
	}
}

LFSEPath LFSEPath::createAdjustedFromUserPath(String userPath) {
	LFSEPath path(*this);
	path.adjust(userPath);
	return path;
}

////////////

std::vector<char> LFSECommand::Arg::getSingleCharFlagsList() const {
	if (type != Type::FLAG)
		return std::vector<char>();
	return std::vector<char>(value.begin(), value.end());
}
std::unordered_set<char> LFSECommand::Arg::getSingleCharFlagsSet() const {
	if (type != Type::FLAG)
		return std::unordered_set<char>();
	return std::unordered_set<char>(value.begin(), value.end());
}

void LFSECommand::parseCmd() {
	String token;
	uint16_t i = 0;
	for (; i < _bufferLength; ++i) {
		char c = _buffer[i];
		if (!isValidFSNameChar(c))
			break;
		token += c;
	}
	_cmdBufferCursor = i;
	_cmd = std::move(token);
	_cmd.toLowerCase();
}

uint8_t LFSECommand::getArgFirstFilenameOrLastArgIdx(uint8_t startIdx) const {
	for (uint8_t i = startIdx; i < _args.size(); ++i) {
		if (_args[i].type == Arg::Type::FILENAME) {
			return i;
		}
	}
	return 0xFF;
}
LFSECommand::Arg LFSECommand::getArgFirstFilenameOrLastArg(uint8_t startIdx) const {
	return _args[getArgFirstFilenameOrLastArgIdx(startIdx)];
}
LFSECommand::Arg LFSECommand::getArgFirstFlags() const {
	Arg flags;
	for (const Arg& arg : _args)
		if (arg.type == Arg::Type::FLAG) {
			flags = arg;
			break;
		}
	return flags;
}

void LFSECommand::parseArgs() {
	// TODO: double check how the unpaired quotes are handled
	if (_argsParsed) {
		return;
	}
	_args.clear();
	if (!_cmdBufferCursor || _cmdBufferCursor >= _bufferLength) {
		return;
	}
	Arg token;
	bool prevCharEscape = false; // helps to detect escaped chars in string args
	auto addToken = [&](bool ignoreEmpty = true){
		if (!token.value.isEmpty() || !ignoreEmpty) {
			token.idx = _args.size();
			_args.push_back(std::move(token));
			token = Arg();
			return true;
		}
		return false;
	};
	auto strArgAddChar = [&](char c) {
		token.value += c;
		prevCharEscape = false;
	};
	for (uint16_t i = _cmdBufferCursor; i < _bufferLength; ++i) {
		char c = _buffer[i];
		if (token.type == Arg::Type::FILENAME) { // already filling filename arg
			if (isValidFSPathChar(c)) {
				token.value += c;
				continue;
			}
			addToken(); // if not valid filename char -> save token
			continue;
		}
		if (token.type == Arg::Type::FLAG) { // already filling flag arg
			if (isAlphaNumeric(c)) { // flag args are only alphanumeric
				token.value += c;
				continue;
			}
			addToken(); // if not valid flag char -> save token
			continue;
		}
		if (token.type == Arg::Type::STRING) { // already filling string filename arg
			if (c == '"') { // potentially end of string arg
				if (prevCharEscape) { // no, it was escaped
					strArgAddChar(c);
					continue;
				}
				addToken(false); // was not escaped -> string ended -> save token
				continue;
			}
			if (prevCharEscape) { // the char was escaped -> add correct symbol to token
				switch(c) {
					case '"':
					case '\\':
						strArgAddChar(c);
						break;
					case 'n':
						strArgAddChar('\n');
						break;
					case 'r':
						strArgAddChar('\r');
						break;
					case 't':
						strArgAddChar('\t');
						break;
					default:
						strArgAddChar('\\');
						strArgAddChar(c);
						break;
				}
				prevCharEscape = false;
			} else {
				if (c == '\\') { // escape symbol
					prevCharEscape = true;
					continue;
				}
				strArgAddChar(c);
			}
			continue;
		}
		// if none of above -> just started filling token
		if (c == '"') { // only string args start with '"'
			token.type = Arg::Type::STRING;
			continue;
		}
		if (c == '-') { // flags start with '-'
			token.type = Arg::Type::FLAG;
			continue;
		}
		if (isValidFSPathChar(c)) {
			token.type = Arg::Type::FILENAME;
			token.value += c;
		}
		continue; // if no conditions met -> bullshit symbol
	}
	if (prevCharEscape) { // if '\' was the last symbol, add it
		strArgAddChar('\\');
	}
	addToken(token.type != Arg::Type::STRING);
	_argsParsed = true;
}

String LFSECommand::toString() const {
	String res(_cmd);
	if (_argsParsed) {
		for (const Arg& arg : _args) {
			res += " ";
			res += arg.toString(false);
		}
	} else if (_cmdBufferCursor && _cmdBufferCursor < _bufferLength) {
		for (uint16_t i = _cmdBufferCursor; i < _bufferLength; ++i)
			res += _buffer[i];
	}
	return res;
}