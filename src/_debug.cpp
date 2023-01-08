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
	cmdMapEntry("cat", cmdInfo(cmdCat, "[filepath]", "print content of the file")),
	cmdMapEntry("man", cmdInfo(cmdMan, "[command]", "show manual for command")),
};
LFSEPath DEBUG::lfsePath;
char DEBUG::lfseBuffer[LFSE_SERIAL_BUFFER_LENGTH];

void DEBUG::cmdHelp(const LSFECommand& cmd) {
	LOGLN(F("The following commands are available for execution:"));
	for (const cmdMapEntry& cmdEntry : lfseCmdMap) {
		LOG(cmdEntry.first);
		LOG(F(" "));
		LOG(std::get<1>(cmdEntry.second));
		LOG(F("\t"));
		LOGLN(std::get<2>(cmdEntry.second));
	}
}
void DEBUG::cmdFormat(const LSFECommand& cmd) {
	if (!LittleFS.format()) {
		LOGLN(F("Formatting filesystem failed!"));
	}
}
void DEBUG::cmdPwd(const LSFECommand& cmd) {
	LOGLN(lfsePath.toString());
}
void DEBUG::cmdLs(const LSFECommand& cmd) {
	LFSEPath path(lfsePath);
	if (cmd._args.size()) {
		if (!isValidFSPath(cmd._args[0])) {
			LOG(F("Invalid directory path: "));
			LOGLN(cmd._args[0]);
			return;
		}
		path.adjust(cmd._args[0]);
	}
	String dirPath = path.toString();
	if (!LittleFS.exists(dirPath)) {
		LOG(dirPath);
		LOGLN(F(" doesn't exist"));
		return;
	}
	Dir dir = LittleFS.openDir(dirPath);
	while (dir.next()) {
		LOG(dir.isFile() ? F("f ") : (dir.isDirectory() ? F("d ") : F("- ")));
		LOG(dir.fileSize());
		LOG(F("\t"));
		LOG(dir.fileCreationTime());
		LOG(F("\t"));
		LOGLN(dir.fileName());
	}
}
void DEBUG::cmdCd(const LSFECommand& cmd) {
	if (!cmd._args.size()) {
		LOGLN(F("Missing operand"));
		return;
	}
	if (!isValidFSPath(cmd._args[0])) {
		LOG(F("Invalid directory path: "));
		LOGLN(cmd._args[0]);
		return;
	}
	lfsePath.adjust(cmd._args[0]);
}
void DEBUG::cmdMkdir(const LSFECommand& cmd) {
	if (!cmd._args.size()) {
		LOGLN(F("Missing operand"));
		return;
	}
	LFSEPath path(lfsePath);
	// DLOG("Current path ");
	// LOGLN(path.toString());
	if (!isValidFSPath(cmd._args[0])) {
		LOG(F("Invalid directory path: "));
		LOGLN(cmd._args[0]);
		return;
	}
	path.adjust(cmd._args[0]);
	String dirPath = path.toString();
	if (LittleFS.exists(dirPath)) {
		LOG(dirPath);
		LOGLN(F(" already exists"));
		return;
	}
	if (!LittleFS.mkdir(dirPath)) {
		LOG(F("Failed to create directory "));
		LOGLN(dirPath);
		return;
	}
}
void DEBUG::cmdMv(const LSFECommand& cmd) {

}
void DEBUG::cmdRm(const LSFECommand& cmd) {
	if (!cmd._args.size()) {
		LOGLN(F("Missing operand"));
		return;
	}
	LFSEPath path(lfsePath);
	if (!isValidFSPath(cmd._args[0])) {
		LOG(F("Invalid directory path: "));
		LOGLN(cmd._args[0]);
		return;
	}
	path.adjust(cmd._args[0]);
	String dirPath = path.toString();
	if (!LittleFS.exists(dirPath)) {
		LOG(dirPath);
		LOGLN(F(" doesn't exist"));
		return;
	}
	Dir dir = LittleFS.openDir(dirPath);
	if (dir.isDirectory()) {
		LOG(F("Removing directory "));
		LOGLN(dirPath);
		if (!LittleFS.rmdir(dirPath)) {
			LOG(F("Failed to remove directory "));
			LOGLN(dirPath);
		}
		return;
	}
	if (dir.isFile()) {
		LOG(F("Removing file "));
		LOGLN(dirPath);
		if (!LittleFS.remove(dirPath)) {
			LOG(F("Failed to remove file "));
			LOGLN(dirPath);
		}
		return;
	}
}
void DEBUG::cmdCat(const LSFECommand& cmd) {

}
void DEBUG::cmdMan(const LSFECommand& cmd) {

}


void DEBUG::logExecutedCommand(const LSFECommand& cmd) {
	LOG(lfsePath.toString());
	LOG(F("$ "));
	LOGLN(cmd.toString());
}
void DEBUG::handleCommand(uint16_t length) {
	if (!length || length >= LFSE_SERIAL_BUFFER_LENGTH)
		return;

	LSFECommand cmd(lfseBuffer, length);
	auto search = lfseCmdMap.find(cmd._cmd);
	logExecutedCommand(cmd);
	if (search == lfseCmdMap.end()) {
		DLOG(F("Error: command "));
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
			DLOGLN(F("Error: Could not read serial data: no valid data found!"));
			return;
		}
		if (nBytesGot == LFSE_SERIAL_BUFFER_LENGTH) {
			DLOGLN(F("Error: Too big command!"));
			return;
		}
		handleCommand(nBytesGot);
	}
}