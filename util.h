void formatBackTime(uint32_t ms, char* tempStrBuf, byte startInd) {
  uint32_t t;
  if (ms < 10000) {  // < 10s
      tempStrBuf[startInd+0] = ms / 1000 + 48;
      tempStrBuf[startInd+1] = '.';
      tempStrBuf[startInd+2] = ms % 1000 / 100 + 48;
      tempStrBuf[startInd+3] = 's';
  } else if (ms < 60000) {  // < 60s == 1m
      t = ms / 1000;  // seconds
      tempStrBuf[startInd+0] = t / 10 + 48;
      tempStrBuf[startInd+1] = t % 10 + 48;
      tempStrBuf[startInd+2] = ' ';
      tempStrBuf[startInd+3] = 's';
  } else if (ms < 600000) {  // < 600s == 10m
      t = ms / 1000;  // seconds
      tempStrBuf[startInd+0] = t / 60 + 48;
      tempStrBuf[startInd+1] = '.';
      tempStrBuf[startInd+2] = t % 60 / 6 + 48;
      tempStrBuf[startInd+3] = 'm';
  } else if (ms < 3600000) {  // < 60m == 1h
      t = ms / 60000;  // minutes
      tempStrBuf[startInd+0] = ' ';
      tempStrBuf[startInd+1] = t / 10 + 48;
      tempStrBuf[startInd+2] = t % 10 + 48;
      tempStrBuf[startInd+3] = 'm';
  } else if (ms < 36000000) {  // < 600m == 10h
      t = ms / 60000;  // minutes
      tempStrBuf[startInd+0] = t / 60 + 48;
      tempStrBuf[startInd+1] = '.';
      tempStrBuf[startInd+2] = t % 60 / 6 + 48;
      tempStrBuf[startInd+3] = 'h';
  } else if (ms < 86400000) {  // < 24h == 1d
      t = ms / 3600000;  // hours
      tempStrBuf[startInd+0] = ' ';
      tempStrBuf[startInd+1] = t / 10 + 48;
      tempStrBuf[startInd+2] = t % 10 + 48;
      tempStrBuf[startInd+3] = 'h';
  } else if (ms < 864000000) {  // < 240h == 10d
      t = ms / 3600000;  // hours
      tempStrBuf[startInd+0] = t / 24 + 48;
      tempStrBuf[startInd+1] = '.';
      tempStrBuf[startInd+2] = t % 24 * 10 / 24 + 48;
      tempStrBuf[startInd+3] = 'd';
  } else if (ms < 2592000000) {  // < 30d == 1M
      t = ms / 86400000;  // days
      tempStrBuf[startInd+0] = ' ';
      tempStrBuf[startInd+1] = t / 10 + 48;
      tempStrBuf[startInd+2] = t % 10 + 48;
      tempStrBuf[startInd+3] = 'd';
  } else {  // > 1M
      t = ms / 86400000;  // days
      tempStrBuf[startInd+0] = t / 30 + 48;
      tempStrBuf[startInd+1] = '.';
      tempStrBuf[startInd+2] = t % 30 / 3 + 48;
      tempStrBuf[startInd+3] = 'M';
  }
}